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
#include "FastSwapQueue.h"
#include "BatchPostMessage.h"

/**

 === 队列性能对比测试 ===
 测试配置: 4096 次操作, 5 轮测试
 测试项目             最小耗时平均耗时最大耗时   吞吐量
 -------------------------------------------------------------------
 std::queue (SPMC)              0.72 ms      0.74 ms      0.80 ms     5533640 ops/sec
 std::queue (MPSC)              0.78 ms      0.82 ms      0.89 ms     4978123 ops/sec
 std::queue (MPMC)              0.84 ms      0.89 ms      0.93 ms     4605352 ops/sec
 boost::lockfree                0.67 ms      0.73 ms      0.77 ms     5598688 ops/sec
 boost::lockfree (MPSC)         1.03 ms      1.15 ms      1.36 ms     3554321 ops/sec
 boost::lockfree (MPMC)         0.96 ms      1.32 ms      1.59 ms     3104441 ops/sec
 boost::asio::post (1 thread)      0.86 ms      0.90 ms      0.93 ms     4553135 ops/sec
 boost::asio::post (4 threads)      1.35 ms      1.40 ms      1.55 ms     2918626 ops/sec
 boost::asio::post (8 threads)      1.57 ms      1.83 ms      2.03 ms     2239230 ops/sec
 
 release 模式：
 === 队列性能对比测试 ===
 测试配置: 32768 次操作, 5 轮测试
 测试项目             最小耗时平均耗时最大耗时   吞吐量
 -------------------------------------------------------------------
 -------------------------------------------------------------------
 std::queue (SPMC)              2.63 ms      2.98 ms      3.54 ms    11011493 ops/sec
 std::queue (MPSC)              3.42 ms      3.79 ms      3.95 ms     8643174 ops/sec
 std::queue (MPMC)              5.27 ms      5.40 ms      5.52 ms     6073322 ops/sec
 boost::lockfree                3.24 ms      3.33 ms      3.46 ms     9852667 ops/sec
 boost::lockfree (MPSC)         6.75 ms      7.15 ms      7.72 ms     4583578 ops/sec
 boost::lockfree (MPMC)        12.24 ms     13.05 ms     13.57 ms     2511882 ops/sec
 boost::asio::post (1 thread)      4.77 ms      6.65 ms      7.74 ms     4929001 ops/sec
 boost::asio::post (4 threads)      8.80 ms      9.26 ms      9.69 ms     3538355 ops/sec
 boost::asio::post (8 threads)     16.67 ms     18.23 ms     19.13 ms     1797477 ops/sec
 boost::asio::post fastswap queu (1 thread)      2.14 ms      2.18 ms      2.23 ms    15060208 ops/sec
 boost::asio::post fastswap queu (4 thread)      2.52 ms      2.86 ms      3.20 ms    11446936 ops/sec
 boost::asio::post fastswap queu (8 thread)      3.32 ms      4.03 ms      4.51 ms     8138692 ops/sec
 boost::asio::post batch post (1 thread)      2.05 ms      2.11 ms      2.15 ms    15547542 ops/sec
 boost::asio::post batch post (4 thread)      3.88 ms      3.99 ms      4.09 ms     8217062 ops/sec
 boost::asio::post batch post (8 thread)      5.64 ms      5.85 ms      6.11 ms     5604241 ops/sec
 *
 */
using namespace std::chrono;

// 测试配置
constexpr size_t QUEUE_SIZE = 4096*8;  // 测试数据量
constexpr size_t BATCH_SIZE = 1000;     // 批处理大小
constexpr int WARMUP_ROUNDS = 0;        // 预热轮数
constexpr int BENCHMARK_ROUNDS = 5;     // 正式测试轮数

boost::asio::io_context main_io_context;
std::atomic<int64_t> io_count;
std::mutex connMutex;

boost::asio::io_context main_io_context2;
std::mutex connMutex2;
std::atomic<int64_t> io_count2;

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

/// Four byte conversion union
union uint32_converter {
    uint32_t i;
    uint8_t c[4];
};

/// Eight byte conversion union
union uint64_converter {
    uint64_t i;
    uint8_t  c[8];
};


typedef uint32_converter masking_key_type;


/// Byte by byte mask/unmask
/**
 * Iterator based byte by byte masking and unmasking for WebSocket payloads.
 * Performs masking in place using the supplied key offset by the supplied
 * offset number of bytes.
 *
 * This function is simple and can be done in place on input with arbitrary
 * lengths and does not vary based on machine word size. It is slow.
 *
 * @param b Beginning iterator to start masking
 *
 * @param e Ending iterator to end masking
 *
 * @param o Beginning iterator to store masked results
 *
 * @param key 32 bit key to mask with.
 *
 * @param key_offset offset value to start masking at.
 */
template <typename input_iter, typename output_iter>
void byte_mask(input_iter first, input_iter last, output_iter result,
    masking_key_type const & key, size_t key_offset = 0)
{
    size_t key_index = key_offset%4;
    while (first != last) {
        *result = *first ^ key.c[key_index++];
        key_index %= 4;
        ++result;
        ++first;
    }
}

/// Copy and mask/unmask in one operation
/**
 * Reads input from one string and writes unmasked output to another.
 *
 * @param [in] i The input string.
 * @param [out] o The output string.
 * @param [in] key The masking key to use for masking/unmasking
 */
void masked_copy (std::string const & i, std::string & o,
    masking_key_type key)
{
    byte_mask(i.begin(),i.end(),o.begin(),key);
}


void masked_copy_simd64 (std::string const & i, std::string & o,
    masking_key_type key)
{
    uint64_converter u64Key;
    std::memcpy(u64Key.c, key.c, 4);
    std::memcpy(u64Key.c + 4, key.c, 4);
    size_t length = i.size()/8;
    uint64_t* u64I = (uint64_t*)i.data();
    uint64_t* u64O = (uint64_t*)o.data();
    for(size_t i=0; i<length; i++){
        u64O[i] = u64I[i] ^ u64Key.i;
    }
    size_t remain = i.length()%8;
    if (remain > 0) {
        size_t offset = i.size() - remain;
        uint8_t* rI = (uint8_t*)(i.data() + offset);
        uint8_t* rO = (uint8_t*)(o.data() + offset);
        for (int i=0; i<remain; i++) {
            rO[i] = rI[i] ^ key.c[i & 3];
        }
    }
}

void masked_copy_simd128(std::string const & i, std::string & o,
    masking_key_type key)
{
    uint64_converter u64Key;
    std::memcpy(u64Key.c, key.c, 4);
    std::memcpy(u64Key.c + 4, key.c, 4);
    size_t length = i.size()/8;
    uint64_t* u64I = (uint64_t*)i.data();
    uint64_t* u64O = (uint64_t*)o.data();
    size_t loop2Length = (length/2)*2;
    uint64_t mask = u64Key.i;
    for(size_t i=0; i<loop2Length; i+=2){
        size_t two = i + 1;
        u64O[i] = u64I[i] ^ mask;
        u64O[two] = u64I[two] ^ mask;
    }
    for(size_t i=loop2Length; i<length; i++){
        u64O[i] = u64I[i] ^ mask;
    }
    size_t remain = i.length()%8;
    if (remain > 0) {
        size_t offset = i.size() - remain;
        uint8_t* rI = (uint8_t*)(i.data() + offset);
        uint8_t* rO = (uint8_t*)(o.data() + offset);
        for (int i=0; i<remain; i++) {
            rO[i] = rI[i] ^ key.c[i & 3];
        }
    }
}

void masked_copy_simd256(std::string const & i, std::string & o,
    masking_key_type key)
{
    uint64_converter u64Key;
    std::memcpy(u64Key.c, key.c, 4);
    std::memcpy(u64Key.c + 4, key.c, 4);
    size_t length = i.size()/8;
    uint64_t* u64I = (uint64_t*)i.data();
    uint64_t* u64O = (uint64_t*)o.data();
    size_t loop4Length = (length/4)*4;
    uint64_t mask = u64Key.i;
    for(size_t i=0; i<loop4Length; i+=4){
        size_t two = i + 1;
        size_t three = i + 1;
        size_t four = i + 1;
        u64O[i] = u64I[i] ^ mask;
        u64O[two] = u64I[two] ^ mask;
        u64O[three] = u64I[three] ^ mask;
        u64O[four] = u64I[four] ^ mask;
    }
    for(size_t i=loop4Length; i<length; i++){
        u64O[i] = u64I[i] ^ mask;
    }
    size_t remain = i.length()%8;
    if (remain > 0) {
        size_t offset = i.size() - remain;
        uint8_t* rI = (uint8_t*)(i.data() + offset);
        uint8_t* rO = (uint8_t*)(o.data() + offset);
        for (int i=0; i<remain; i++) {
            rO[i] = rI[i] ^ key.c[i%4];
        }
    }
}

void masked_copy_simd32(std::string const & i, std::string & o,
    masking_key_type key)
{
    uint32_converter u32Key;
    std::memcpy(u32Key.c, key.c, 4);
    size_t length = i.size()/4;
    uint32_t* u32I = (uint32_t*)i.data();
    uint32_t* u32O = (uint32_t*)o.data();
    for(int i=0; i<length; i++){
        u32O[i] = u32I[i] ^ u32Key.i;
    }
    size_t remain = i.length()%4;
    if (remain > 0) {
        size_t offset = i.size() - remain;
        uint8_t* rI = (uint8_t*)(i.data() + offset);
        uint8_t* rO = (uint8_t*)(o.data() + offset);
        for (int i=0; i<remain; i++) {
            rO[i] = rI[i] ^ key.c[i%4];
        }
    }
}

void byte_mask2(char* first, char* last, char* result,
               uint8_t* mask, size_t key_offset)
{
    size_t key_index = key_offset%4;
    while (first != last) {
        *result = *first ^ mask[key_index++];
        key_index %= 4;
        ++result;
        ++first;
    }
}


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
                //模拟耗时操作
                /**
                //std::this_thread::sleep_for(std::chrono::microseconds(2));
                auto start = std::chrono::high_resolution_clock::now();
                auto end = std::chrono::high_resolution_clock::now();
                auto used = std::chrono::duration_cast<std::chrono::microseconds>(end-start).count();
                do {
                    end = std::chrono::high_resolution_clock::now();
                    used = std::chrono::duration_cast<std::chrono::microseconds>(end-start).count();
                } while (used <= 2);
                */
                std::shared_ptr<std::string> in = std::make_shared<std::string>(1024, 'a');
                masking_key_type key;
                {
                    std::lock_guard<std::mutex> lock(connMutex2);
                    key.i = rand();
                }
                masked_copy_simd128(*in, *in, key);
               /**
                boost::asio::post(main_io_context2, [in] {
                    {
                        std::lock_guard<std::mutex> lock(connMutex2);
                        io_count2 += in->size();
                        io_count2 += in->at(rand()%1024);
                    }
                    auto start = std::chrono::high_resolution_clock::now();
                    auto end = std::chrono::high_resolution_clock::now();
                    auto used = std::chrono::duration_cast<std::chrono::microseconds>(end-start).count();
                    do {
                        end = std::chrono::high_resolution_clock::now();
                        used = std::chrono::duration_cast<std::chrono::microseconds>(end-start).count();
                    } while (used <= 2);
                   
                });*/
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
            /**
            auto start = std::chrono::high_resolution_clock::now();
            auto end = std::chrono::high_resolution_clock::now();
            auto used = std::chrono::duration_cast<std::chrono::microseconds>(end-start).count();
            do {
                end = std::chrono::high_resolution_clock::now();
                used = std::chrono::duration_cast<std::chrono::microseconds>(end-start).count();
            } while (used <= 6);*/
            std::shared_ptr<std::string> in = std::make_shared<std::string>(1025, 'a');
            masking_key_type key;
            {
                std::lock_guard<std::mutex> lock(connMutex);
                key.i = rand();
            }
            masked_copy_simd128(*in, *in, key);
            
            /**
            boost::asio::post(main_io_context, [in] {
                {
                    std::lock_guard<std::mutex> lock(connMutex);
                    io_count += in->size();
                    io_count += in->at(rand()%1024);
                }
                auto start = std::chrono::high_resolution_clock::now();
                auto end = std::chrono::high_resolution_clock::now();
                auto used = std::chrono::duration_cast<std::chrono::microseconds>(end-start).count();
                do {
                    end = std::chrono::high_resolution_clock::now();
                    used = std::chrono::duration_cast<std::chrono::microseconds>(end-start).count();
                } while (used <= 2);
            });*/
            
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

double test_boost_post_with_swap_queue(int thread_count) {
    boost::asio::io_context io_context;
    auto work = boost::asio::make_work_guard(io_context);
    
    std::vector<std::thread> threads;
    for (int i = 0; i < thread_count; ++i) {
        threads.emplace_back([&] { io_context.run(); });
    }
    
    std::atomic<size_t> counter(0);
    std::atomic<bool> finished(false);
    
    FastSwapQueue fastSwapQueue(true);
    fastSwapQueue.setConsumeTasksFunc([&] {
        boost::asio::post(io_context, [&] {
            fastSwapQueue.consumeTasks();
        });
    });
    auto start_time = high_resolution_clock::now();
    
    // 提交任务
    for (size_t i = 0; i < QUEUE_SIZE; ++i) {
        fastSwapQueue.post([&] {
            std::shared_ptr<std::string> in = std::make_shared<std::string>(1025, 'a');
            masking_key_type key;
            {
                std::lock_guard<std::mutex> lock(connMutex);
                key.i = rand();
            }
            masked_copy_simd128(*in, *in, key);
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

double test_boost_post_with_batch_queue(int thread_count) {
    boost::asio::io_context io_context;
    auto work = boost::asio::make_work_guard(io_context);
    
    std::vector<std::thread> threads;
    for (int i = 0; i < thread_count; ++i) {
        threads.emplace_back([&] { io_context.run(); });
    }
    
    std::atomic<size_t> counter(0);
    std::atomic<bool> finished(false);
    
    BatchPostMessage<int> batchPostMessage(256, true);
    batchPostMessage.setOnBatchMessageAction([&](auto batchMsg) {
        boost::asio::post(io_context, [&,batchMsg] {
            for(int i=0; i<batchMsg->size(); i++) {
                std::shared_ptr<std::string> in = std::make_shared<std::string>(1025, 'a');
                masking_key_type key;
                {
                    std::lock_guard<std::mutex> lock(connMutex);
                    key.i = rand();
                }
                masked_copy_simd128(*in, *in, key);
                if (counter.fetch_add(1, std::memory_order_relaxed) >= QUEUE_SIZE - 1) {
                    finished = true;
                }
            }
            batchPostMessage.recycle(batchMsg);
        });
    });
  
    auto start_time = high_resolution_clock::now();
    
    // 提交任务
    for (size_t i = 0; i < QUEUE_SIZE; ++i) {
        batchPostMessage.add(i);
    }
    batchPostMessage.flush();
    
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
    
    std::thread mainThread([]{
        main_io_context.run_one();
        main_io_context.run();
    });
    
    std::thread mainThread2([]{
        main_io_context.run_one();
        main_io_context2.run();
    });
    
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
    
    
    run_benchmark("boost::asio::post fastswap queu (1 thread)", [&] { return test_boost_post_with_swap_queue(1); });
    run_benchmark("boost::asio::post fastswap queu (4 thread)", [&] { return test_boost_post_with_swap_queue(4); });
    run_benchmark("boost::asio::post fastswap queu (8 thread)", [&] { return test_boost_post_with_swap_queue(8); });
    
    run_benchmark("boost::asio::post batch post (1 thread)", [&] { return test_boost_post_with_batch_queue(1); });
    run_benchmark("boost::asio::post batch post (4 thread)", [&] { return test_boost_post_with_batch_queue(4); });
    run_benchmark("boost::asio::post batch post (8 thread)", [&] { return test_boost_post_with_batch_queue(8); });
    
    
    mainThread.join();
    
    mainThread2.join();
    
    return 0;
}
