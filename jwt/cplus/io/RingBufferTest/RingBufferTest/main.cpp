#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <atomic>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <boost/lockfree/spsc_queue.hpp>
#include <string>
#include <numeric>

/**
 
 === Testing SyncQueue SPMC ===
 Run 1: 0.0276213 seconds, 3.6204e+07 ops/sec
 Run 2: 0.025548 seconds, 3.91419e+07 ops/sec
 Run 3: 0.0241003 seconds, 4.14933e+07 ops/sec
 Run 4: 0.0260058 seconds, 3.84529e+07 ops/sec
 Run 5: 0.0276954 seconds, 3.61071e+07 ops/sec

 Summary:
 Average time: 0.0261942 ± 0.00135112 seconds
 Average throughput: 3.82799e+07 ± 2.00647e+06 ops/sec

 === Testing RingBuffer SPMC ===
 Run 1: 0.0382203 seconds, 2.61641e+07 ops/sec
 Run 2: 0.0392209 seconds, 2.54966e+07 ops/sec
 Run 3: 0.0380384 seconds, 2.62892e+07 ops/sec
 Run 4: 0.0385007 seconds, 2.59735e+07 ops/sec
 Run 5: 0.0382513 seconds, 2.61429e+07 ops/sec

 Summary:
 Average time: 0.0384463 ± 0.000414364 seconds
 Average throughput: 2.60133e+07 ± 277214 ops/sec
 
 */

// SyncQueue实现
template<typename T>
class SyncQueue {
private:
    std::queue<T> queue_;
    mutable std::mutex mutex_;
    std::condition_variable cv_;
    std::atomic<bool> running_{true};

public:
    void push(const T& value) {
        std::unique_lock<std::mutex> lock(mutex_);
        queue_.push(value);
        cv_.notify_one();
    }

    bool pop(T& value) {
        std::unique_lock<std::mutex> lock(mutex_);
        // 等待队列非空或停止信号
        cv_.wait(lock, [this]{ return !queue_.empty() || !running_; });
        if (queue_.empty()) return false;
        value = queue_.front();
        queue_.pop();
        return true;
    }
    
    bool empty() {
        std::unique_lock<std::mutex> lock(mutex_);
        return queue_.empty();
    }

    void stop() {
        running_ = false;
        cv_.notify_all();
    }
};

// 测试参数
constexpr size_t kQueueSize = 8192;
constexpr size_t kNumOperations = 1000000;
constexpr int kNumRuns = 5;  // 每个测试运行的次数

// 计时辅助类
class Timer {
public:
    void start() {
        start_time_ = std::chrono::high_resolution_clock::now();
    }

    double stop() {
        auto end_time = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time_).count() * 1e-9;
    }

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start_time_;
};

// 测试 SyncQueue SPMC
double testSyncQueueSPMC() {
    Timer timer;
    SyncQueue<int> queue;
    std::atomic<bool> done(false);
    std::atomic<size_t> count(0);

    // 消费者线程
    std::thread consumer([&] {
        int value;
        // 先检查 done 标志，避免生产者提前完成时的无限等待
        while (!done || !queue.empty()) {
            if (queue.pop(value)) {
                count.fetch_add(1);
            } else {
                // 队列为空时短暂休眠，避免忙等待
                std::this_thread::yield();
            }
        }
    });

    // 生产者线程
    std::thread producer([&] {
        for (size_t i = 0; i < kNumOperations; ++i) {
            queue.push(i);
        }
        done = true;
        // 确保消费者被唤醒
        queue.stop();
    });

    // 开始计时
    timer.start();

    // 等待线程完成
    producer.join();
    consumer.join();

    // 停止计时
    double duration = timer.stop();

    // 验证操作次数
    if (count.load() != kNumOperations) {
        std::cerr << "Error: SyncQueue processed " << count.load()
                  << " items, expected " << kNumOperations << std::endl;
    }

    return duration;
}

// 测试 RingBuffer SPMC
double testRingBufferSPMC() {
    Timer timer;
    boost::lockfree::spsc_queue<int, boost::lockfree::capacity<kQueueSize>> queue;
    std::atomic<bool> done(false);
    std::atomic<size_t> count(0);

    // 消费者线程
    std::thread consumer([&] {
        int value;
        // 先检查 done 标志，避免生产者提前完成时的无限等待
        while (!done || !queue.empty()) {
            if (queue.pop(value)) {
                count.fetch_add(1);
            } else {
                // 队列为空时短暂休眠，避免忙等待
                std::this_thread::sleep_for(std::chrono::microseconds(1));
            }
        }
    });

    // 生产者线程
    std::thread producer([&] {
        for (size_t i = 0; i < kNumOperations; ++i) {
            // 等待队列有空间
            while (!queue.push(i)) {
                std::this_thread::yield();
            }
        }
        done = true;
    });

    // 开始计时
    timer.start();

    // 等待线程完成
    producer.join();
    consumer.join();

    // 停止计时
    double duration = timer.stop();

    // 验证操作次数
    if (count.load() != kNumOperations) {
        std::cerr << "Error: RingBuffer processed " << count.load()
                  << " items, expected " << kNumOperations << std::endl;
    }

    return duration;
}

// 运行测试并输出结果
void runTest(const std::string& name, double (*testFunc)()) {
    std::vector<double> times;
    std::vector<double> throughputs;

    std::cout << "\n=== Testing " << name << " ===" << std::endl;

    for (int i = 0; i < kNumRuns; ++i) {
        std::cout << "Run " << (i+1) << ": ";
        double duration = testFunc();
        double throughput = kNumOperations / duration;

        times.push_back(duration);
        throughputs.push_back(throughput);

        std::cout << duration << " seconds, "
                  << throughput << " ops/sec" << std::endl;
    }

    // 计算平均值和标准差
    double avg_time = std::accumulate(times.begin(), times.end(), 0.0) / kNumRuns;
    double avg_throughput = std::accumulate(throughputs.begin(), throughputs.end(), 0.0) / kNumRuns;

    double variance_time = 0.0;
    double variance_throughput = 0.0;
    for (int i = 0; i < kNumRuns; ++i) {
        variance_time += (times[i] - avg_time) * (times[i] - avg_time);
        variance_throughput += (throughputs[i] - avg_throughput) * (throughputs[i] - avg_throughput);
    }
    variance_time /= kNumRuns;
    variance_throughput /= kNumRuns;

    double stddev_time = std::sqrt(variance_time);
    double stddev_throughput = std::sqrt(variance_throughput);

    std::cout << "\nSummary:" << std::endl;
    std::cout << "Average time: " << avg_time << " ± " << stddev_time << " seconds" << std::endl;
    std::cout << "Average throughput: " << avg_throughput << " ± " << stddev_throughput << " ops/sec" << std::endl;
}

int main() {
    std::cout << "Performance Comparison: SyncQueue vs RingBuffer" << std::endl;
    std::cout << "Operations per test: " << kNumOperations << std::endl;
    std::cout << "Number of runs per test: " << kNumRuns << std::endl;

    runTest("SyncQueue SPMC", testSyncQueueSPMC);
    runTest("RingBuffer SPMC", testRingBufferSPMC);

    return 0;
}
