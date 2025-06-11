//
//  batch_queue_test.cc
//  websocketpp
//
//  Created by baojian on 2025/6/11.
//
#include "BatchPost.hpp"
#include <iostream>
#include <boost/asio.hpp>


int batch_queue_test(int argc, const char * argv[]){
    
    
    boost::asio::io_context encode_send_frame_io_context;
    boost::asio::io_context& io_context = encode_send_frame_io_context;
    auto worker_guard = boost::asio::make_work_guard(io_context);
    
    std::thread encodeSendFrameThread([&io_context] {
        io_context.run();
    });
    
    
    
    BatchPostMessage<std::shared_ptr<std::string>> batchPost(128);
    batchPost.setOnBatchMessaage([&io_context](auto batchMsg) {
        boost::asio::post(io_context, [batchMsg] {
            std::cout << "batch message size" << std::endl;
        });
    });
    
    for(int i=0; i<10000*100; i++) {
        batchPost.add(std::make_shared<std::string>("hello world"));
    }
    batchPost.flush();
    
    encodeSendFrameThread.join();
    
    return 0;
}
