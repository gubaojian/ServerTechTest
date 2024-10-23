//
//  tbb_test.cc
//  follyd
//
//  Created by efurture on 2024/10/8.
//
#include <iostream>
#include <string>
#include <iostream>
#include <chrono>
#include <map>
#include <set>
#include <cstdlib>
#include <unistd.h>
#include <thread>
#include <memory>
#include <folly/concurrency/container/LockFreeRingBuffer.h>


/**
 
 https://github.com/facebook/folly/blob/main/folly/concurrency/container/test/LockFreeRingBufferTest.cpp
 
 10000*300  element
 
 client send event used 319ms
 
 client send event used 295ms
 client send event used 320ms
 
 */

int folly_circular_buffer_test_main(int argc, const char * argv[]) {
    auto start = std::chrono::high_resolution_clock::now();
    auto end = std::chrono::high_resolution_clock::now();
    auto used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    
    folly::detail::hextab
    std::shared_ptr<folly::LockFreeRingBuffer<int>> buffer = std::make_shared<folly::LockFreeRingBuffer<int>>(1024);
    
    std::thread consumer([buffer] {
        std::cout << "comsumer start " << std::endl;
        auto  cur = buffer->currentHead();
        int count = 0;
        while (true) {
            int item;
            buffer->waitAndTryRead(item, cur);
            count++;
            //std::cout << "comsumer " << item << std::endl;
            cur.moveForward();
            if (count == 10000*300*2) {
                std::cout << "all message received "  << std::endl;
            }
        }
    });
    
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(500ms);
    
    auto produceFunc = [buffer] {
        std::cout << "producer start " << std::endl;
        auto start = std::chrono::high_resolution_clock::now();
       
        for(int i=0; i<10000*300; i++) {
            buffer->write(i);
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "client send event used " << used.count() << "ms" << std::endl;
        
    };
    
    std::thread producter(produceFunc);
    producter.join();
    
    std::thread producter2(produceFunc);
    producter2.join();
    
    consumer.join();
    return 0;
}
