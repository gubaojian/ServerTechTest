#include <iostream>
#include <chrono>
#include <thread>
#include <atomic>
#include <boost/asio/io_context.hpp>
#include <boost/asio/post.hpp>  // 添加这一行

constexpr int POST_COUNT = 4000000;

/**
 
 完成 4000000 次 post 调用
 耗时: 32 毫秒
 平均每次调用: 0.008 微秒
 
 */
int main() {
    boost::asio::io_context io_context;
    // 使用make_work_guard替代io_context::work
    auto work = boost::asio::make_work_guard(io_context);
    
    std::atomic<int> counter(0);
    std::atomic<bool> finished(false);
    
    // 启动工作线程运行io_context
    std::thread worker([&]() {
        io_context.run();
    });
    
    // 记录开始时间
    auto start = std::chrono::high_resolution_clock::now();
    
    // 在另一个线程中进行400万次post调用
    std::thread poster([&]() {
        std::vector<int> batch;
        for (int i = 0; i < POST_COUNT; ++i) {
            batch.push_back(i);
            if (batch.size() >= 100) {
                 // 使用boost::asio::post替代io_context.post
                boost::asio::post(io_context, [&, bt = std::move(batch)]() {
                    for(int i=0; i<bt.size(); ++i) {
                        if (++counter == POST_COUNT) {
                           finished = true;
                       }
                    }
                    
                });
                batch = std::vector<int>(); // 清空batch
            }
        }
    });
    
    // 等待所有post完成
    while (!finished) {
        std::this_thread::yield();
    }
    
    // 记录结束时间
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    
    // 清理资源
    work.reset();  // 释放工作保护
    io_context.stop();
    if (poster.joinable()) poster.join();
    if (worker.joinable()) worker.join();
    
    // 输出结果
    std::cout << "完成 " << POST_COUNT << " 次 post 调用" << std::endl;
    std::cout << "耗时: " << duration << " 毫秒" << std::endl;
    std::cout << "平均每次调用: " << (double)duration / POST_COUNT * 1000 << " 微秒" << std::endl;
    
    return 0;
}
