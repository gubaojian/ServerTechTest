//
//  uuid_compare_test.cc
//  cplus
//
//  Created by efurture on 2024/9/26.
//



#include <string>
#include <iostream>
#include <uuid/uuid.h>
#include <chrono>
#include <map>
#include <set>
#include <cstdlib>
#include <unistd.h>

#include <folly/futures/Future.h>
#include <folly/executors/GlobalExecutor.h>
#include <folly/executors/CPUThreadPoolExecutor.h>


std::atomic<int> taskNum;
std::atomic<int> futureNum;

int folly_executor_test_main(int argc, const char * argv[]) {
    auto start = std::chrono::high_resolution_clock::now();
    auto end = std::chrono::high_resolution_clock::now();
    auto used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    
    // 创建一个拥有 4 个线程的 CPUThreadPoolExecutor
    
    
    folly::via(folly::getGlobalCPUExecutor(), []() {
           std::cout << "Async task is running." << std::endl;
           return 42;
    });
    folly::getGlobalCPUExecutor()->add([]{
        std::cout << "Add Fun success " << std::endl;
    });
    
    start = std::chrono::high_resolution_clock::now();
    // 100万 send fun done task message done 348ms
    // 200万 send fun done task message done 348*2ms
    size_t length = 10000*100;
    for(int i=0; i<length; i++) {
        folly::getGlobalCPUExecutor()->add([length, start]{
            taskNum++;
            if(taskNum >= length) {
                auto end = std::chrono::high_resolution_clock::now();
                auto used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
                std::cout << "task message done " << used.count() << "ms "  << std::endl;
              
            }
        });
    }
    end = std::chrono::high_resolution_clock::now();
    used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "send fun done " << used.count() << "ms "  << std::endl;
    usleep(1000);
    
    
    
    
    return 0;
}
