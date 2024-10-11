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

#include <zmq.h>

#include <thread>






/**
 
 size_t length = 10000*300;
 tbb send fun done 1529ms
 */

int zero_mq_inner_ipc_test_main(int argc, const char * argv[]) {
    auto start = std::chrono::high_resolution_clock::now();
    auto end = std::chrono::high_resolution_clock::now();
    auto used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    
    std::thread server([] {
        std::cout << "server bind success " << std::endl;
        int io_threads = 4;
        void *context = zmq_ctx_new ();
        zmq_ctx_set (context, ZMQ_IO_THREADS, io_threads);
        void *responder = zmq_socket (context, ZMQ_REP);
        int rc = zmq_bind (responder, "inproc://workers");
        int receiveNum = 0;
        std::cout << "server bind success " << rc << std::endl;
        auto start = std::chrono::high_resolution_clock::now();
       
        while (1) {
               char buffer [32];
               zmq_recv (responder, buffer, 18, 0);
               if (receiveNum == 0) {
                 start = std::chrono::high_resolution_clock::now();
               }
               receiveNum ++;
               zmq_send (responder, "World From Server", 10, 0);
               if (receiveNum >= 20000) {
                   auto end = std::chrono::high_resolution_clock::now();
                   auto used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
                   std::cout << buffer << " done" << receiveNum << "used" << used.count() << std::endl;
                   break;
                }
        }
        zmq_close (responder);
        zmq_ctx_destroy (context);
    });
    
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(2000ms);
    
    auto clientFunc = [] {
        void *context = zmq_ctx_new ();
        void *requester = zmq_socket (context, ZMQ_REQ);
        int connect = zmq_connect (requester, "inproc://workers");
        
        std::cout << "client connect success " << connect  << std::endl;
        auto start = std::chrono::high_resolution_clock::now();
       
        int request_num;
        for (request_num = 0; request_num < 10000; request_num++) {
            char buffer [32];
            int status = zmq_send (requester, "Hello World Client", 18, 0);
            zmq_recv (requester, buffer, 10, 0);
        }
        auto end = std::chrono::high_resolution_clock::now();
        auto used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "client send event used " << used.count() << "ms" << std::endl;
        zmq_close (requester);
        zmq_ctx_destroy (context);

    };
    
    std::thread clientOne(clientFunc);
    
    clientOne.join();
    std::thread clientTwo(clientFunc);
    server.join();
    clientTwo.join();
    return 0;
}
