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
#include <folly/IndexedMemPool.h>


/**
 
 https://github.com/facebook/folly/blob/main/folly/concurrency/container/test/LockFreeRingBufferTest.cpp
 
 10000*300  element
 

 client send event used 343ms
 
 */

//2kb以下，静态数组效率高
//再往上就比较慢啦
struct StringMessage {
    char msg[1024];
    int type;
};

struct PoolMsg {
    char msg[2048];
    
    PoolMsg() {
        std::cout << "con" << std::endl;
    }
    
    ~PoolMsg() {
        std::cout << "release" << std::endl;
    }
};

/**
 *
 folly::IndexedMemPool<PoolMsg> pool(2048)
 for(int i=0; i<2048; i++) {
     auto ptr2 = pool.allocElem();
     uint32_t idx = pool.allocIndex();
    
     //pool[idx];
     //PoolMsg* ptr = pool[idx];
     std::cout << 0  << "id " << idx << std::endl;
     pool.recycleIndex(idx);
 }

 
 std::cout << "capcity " << pool.capacity() << std::endl;
 
 */

int folly_circular_buffer_string_test_main(int argc, const char * argv[]) {
    auto start = std::chrono::high_resolution_clock::now();
    auto end = std::chrono::high_resolution_clock::now();
    auto used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    /**
    folly::IndexedMemPool<PoolMsg> pool(2048);
    
    for(int i=0; i<2048; i++) {
        auto ptr2 = pool.allocElem();
        uint32_t idx = pool.allocIndex();
       
        //pool[idx];
        //PoolMsg* ptr = pool[idx];
        std::cout << 0  << "id " << idx << std::endl;
        pool.recycleIndex(idx);
    }
     
     std::cout << "capcity " << pool.capacity() << std::endl;
     
     */

    
   
    std::shared_ptr<folly::LockFreeRingBuffer<StringMessage>> buffer = std::make_shared<folly::LockFreeRingBuffer<StringMessage>>(1024);
    
    std::thread consumer([buffer] {
        std::cout << "comsumer start " << std::endl;
        auto  cur = buffer->currentHead();
        int count = 0;
        while (true) {
            StringMessage item;
            buffer->waitAndTryRead(item, cur);
            count++;
            //if (item.msg != nullptr) {
                //free(item.msg);
            //}
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
            StringMessage msg;
            buffer->currentHead();
            buffer->write(msg);
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "client send event used " << used.count() << "ms" << std::endl;
        
    };
    
    std::thread producter(produceFunc);
    producter.join();
    
    //std::thread producter2(produceFunc);
    //producter2.join();
    
    consumer.join();
    return 0;
}
