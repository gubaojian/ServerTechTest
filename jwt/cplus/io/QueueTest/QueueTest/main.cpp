//
//  main.cpp
//  QueueTest
//
//  Created by baojian on 2025/6/19.
//

#include <iostream>
#include <iomanip>
#include <queue>
#include <mutex>
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>
#include <functional>
#include <string>
#include <numeric>
#include <boost/lockfree/queue.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/post.hpp>

/**

 === 队列性能对比测试 ===
 测试配置: 4096 次操作, 5 轮测试
 测试项目             最小耗时平均耗时最大耗时   吞吐量
 -------------------------------------------------------------------
 std::queue (SPMC)              0.42 ms      0.43 ms      0.48 ms     9490269 ops/sec
 std::queue (MPSC)              0.63 ms      0.68 ms      0.72 ms     6016451 ops/sec
 std::queue (MPMC)              0.99 ms      1.14 ms      1.25 ms     3599297 ops/sec
 boost::lockfree                0.73 ms      0.74 ms      0.75 ms     5535135 ops/sec
 boost::lockfree (MPSC)         0.81 ms      0.82 ms      0.84 ms     4964848 ops/sec
 boost::lockfree (MPMC)         1.64 ms      1.92 ms      2.07 ms     2135781 ops/sec
 boost::asio::post (1 thread)      1.28 ms      1.32 ms      1.34 ms     3110099 ops/sec
 boost::asio::post (4 threads)      2.86 ms      3.10 ms      3.46 ms     1320524 ops/sec
 boost::asio::post (8 threads)      6.66 ms      6.93 ms      7.35 ms      591139 ops/sec
 
 release 模式：
 === 队列性能对比测试 ===
 测试配置: 4096 次操作, 5 轮测试
 测试项目             最小耗时平均耗时最大耗时   吞吐量
 -------------------------------------------------------------------
 std::queue (SPMC)              0.10 ms      0.11 ms      0.12 ms    38208955 ops/sec
 std::queue (MPSC)              0.16 ms      0.19 ms      0.22 ms    22068966 ops/sec
 std::queue (MPMC)              0.29 ms      0.35 ms      0.47 ms    11538028 ops/sec
 boost::lockfree                0.45 ms      0.47 ms      0.51 ms     8740930 ops/sec
 boost::lockfree (MPSC)         0.75 ms      0.81 ms      0.89 ms     5064293 ops/sec
 boost::lockfree (MPMC)         1.92 ms      2.04 ms      2.09 ms     2004502 ops/sec
 boost::asio::post (1 thread)      0.25 ms      0.33 ms      0.36 ms    12503053 ops/sec
 boost::asio::post (4 threads)      0.73 ms      0.88 ms      0.99 ms     4661962 ops/sec
 boost::asio::post (8 threads)      1.53 ms      1.80 ms      2.06 ms     2273030 ops/sec
 *
 */
using namespace std::chrono;

// 测试配置
constexpr size_t QUEUE_SIZE = 4096;  // 测试数据量
constexpr size_t BATCH_SIZE = 1000;     // 批处理大小
constexpr int WARMUP_ROUNDS = 0;        // 预热轮数
constexpr int BENCHMARK_ROUNDS = 5;     // 正式测试轮数

// 同步队列实现 (std::queue + std::mutex)
template<typename T>
class SyncQueue {
private:
    std::queue<T> queue_;
    std::mutex mutex_;
    std::condition_variable cv_;
    std::atomic<bool> running_{true};

public:
    void push(const T& value) {
        std::unique_lock<std::mutex> lock(mutex_);
        queue_.push(value);
        cv_.notify_all();
    }
    
    void push(T&& value) {
        std::unique_lock<std::mutex> lock(mutex_);
        queue_.push(std::move(value));
        cv_.notify_all();
    }

    bool pop(T& value) {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this]{ return !queue_.empty() || !running_; });
        if (queue_.empty()) return false;
        value = queue_.front();
        queue_.pop();
        return true;
    }
    
    bool try_pop(T& value) {
        std::unique_lock<std::mutex> lock(mutex_);
        if (queue_.empty() || !running_) return false;
        value = queue_.front();
        queue_.pop();
        return true;
    }

    bool pop(T& value, std::chrono::milliseconds timeout) {
        std::unique_lock<std::mutex> lock(mutex_);
        if (!cv_.wait_for(lock, timeout, [this]{ return !queue_.empty() || !running_; })) {
            return false;  // 超时
        }
        if (queue_.empty()) return false;
        value = queue_.front();
        queue_.pop();
        return true;
    }

    void stop() {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            running_ = false;  // 在锁保护下设置标志
        }
        cv_.notify_all();
    }
};

// 测试函数
template<typename Queue>
double test_spsc(Queue& queue, int producer_count, int consumer_count) {
    std::atomic<bool> start(false);
    std::atomic<size_t> produced(0);
    std::atomic<size_t> consumed(0);
    
    std::vector<std::thread> producers;
    std::vector<std::thread> consumers;
    
    // 创建生产者线程
    for (int i = 0; i < producer_count; ++i) {
        producers.emplace_back([&] {
            while (!start) std::this_thread::yield();
            
            size_t count = 0;
            while (produced.fetch_add(1, std::memory_order_relaxed) < QUEUE_SIZE) {
                queue.push(count++);
            }
        });
    }
    
    // 创建消费者线程
    for (int i = 0; i < consumer_count; ++i) {
        consumers.emplace_back([&] {
            int value;
            while (!start) std::this_thread::yield();
            
            while (consumed.fetch_add(1, std::memory_order_relaxed) < QUEUE_SIZE) {
                while (!queue.pop(value)) {}
            }
        });
    }
    
    // 同步开始时间
    auto start_time = high_resolution_clock::now();
    start = true;
    
    // 等待所有线程完成
    for (auto& t : producers) t.join();
    for (auto& t : consumers) t.join();
    
    auto end_time = high_resolution_clock::now();
    return duration_cast<microseconds>(end_time - start_time).count() / 1000.0;
}

// Boost::asio::io_context 测试
double test_boost_post(int thread_count) {
    boost::asio::io_context io_context;
    auto work = boost::asio::make_work_guard(io_context);
    
    std::vector<std::thread> threads;
    for (int i = 0; i < thread_count; ++i) {
        threads.emplace_back([&] { io_context.run(); });
    }
    
    std::atomic<size_t> counter(0);
    std::atomic<bool> finished(false);
    
    auto start_time = high_resolution_clock::now();
    
    // 提交任务
    for (size_t i = 0; i < QUEUE_SIZE; ++i) {
        boost::asio::post(io_context, [&] {
            if (counter.fetch_add(1, std::memory_order_relaxed) >= QUEUE_SIZE - 1) {
                finished = true;
            }
        });
    }
    
    // 等待所有任务完成
    while (!finished) std::this_thread::yield();
    
    auto end_time = high_resolution_clock::now();
    
    // 停止并清理
    io_context.stop();
    for (auto& t : threads) t.join();
    
    return duration_cast<microseconds>(end_time - start_time).count() / 1000.0;
}

// 运行基准测试
void run_benchmark(const std::string& name, std::function<double()> test_func) {
    // 预热
    for (int i = 0; i < WARMUP_ROUNDS; ++i) {
        test_func();
    }
    
    // 正式测试
    std::vector<double> results;
    for (int i = 0; i < BENCHMARK_ROUNDS; ++i) {
        results.push_back(test_func());
    }
    
    // 计算统计数据
    double avg = std::accumulate(results.begin(), results.end(), 0.0) / results.size();
    double min = *std::min_element(results.begin(), results.end());
    double max = *std::max_element(results.begin(), results.end());
    
    // 输出结果
    std::cout << std::left << std::setw(25) << name
              << std::right
              << std::setw(10) << std::fixed << std::setprecision(2) << min << " ms"
              << std::setw(10) << std::fixed << std::setprecision(2) << avg << " ms"
              << std::setw(10) << std::fixed << std::setprecision(2) << max << " ms"
              << std::setw(12) << std::fixed << std::setprecision(0)
              << QUEUE_SIZE / (avg / 1000) << " ops/sec"
              << std::endl;
}

int main() {
    std::cout << "=== 队列性能对比测试 ===" << std::endl;
    std::cout << "测试配置: " << QUEUE_SIZE << " 次操作, "
              << BENCHMARK_ROUNDS << " 轮测试" << std::endl;
    std::cout << std::left << std::setw(25) << "测试项目"
              << std::right << std::setw(10) << "最小耗时"
              << std::setw(10) << "平均耗时"
              << std::setw(10) << "最大耗时"
              << std::setw(12) << "吞吐量" << std::endl;
    std::cout << std::string(67, '-') << std::endl;
    
    // 测试不同场景
    {
        SyncQueue<int> sync_queue;
        run_benchmark("std::queue (SPMC)", [&] { return test_spsc(sync_queue, 1, 1); });
    }
    
    {
        SyncQueue<int> sync_queue;
        run_benchmark("std::queue (MPSC)", [&] { return test_spsc(sync_queue, 4, 1); });
    }
    
    {
        SyncQueue<int> sync_queue;
        run_benchmark("std::queue (MPMC)", [&] { return test_spsc(sync_queue, 4, 4); });
    }
    
    {
        boost::lockfree::queue<int, boost::lockfree::capacity<QUEUE_SIZE>> lockfree_queue(QUEUE_SIZE);
        run_benchmark("boost::lockfree", [&] { return test_spsc(lockfree_queue, 1, 1); });
    }
    
    {
        boost::lockfree::queue<int> lockfree_queue(QUEUE_SIZE);
        run_benchmark("boost::lockfree (MPSC)", [&] { return test_spsc(lockfree_queue, 4, 1); });
    }
    
    {
        boost::lockfree::queue<int> lockfree_queue(QUEUE_SIZE);
        run_benchmark("boost::lockfree (MPMC)", [&] { return test_spsc(lockfree_queue, 4, 4); });
    }
    
    run_benchmark("boost::asio::post (1 thread)", [&] { return test_boost_post(1); });
    run_benchmark("boost::asio::post (4 threads)", [&] { return test_boost_post(4); });
    run_benchmark("boost::asio::post (8 threads)", [&] { return test_boost_post(8); });
    
    return 0;
}
