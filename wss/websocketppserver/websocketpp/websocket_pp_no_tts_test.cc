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

// See https://github.com/bitcoin/bitcoin/issues/30751.
// https://github.com/bitcoin/bitcoin/pull/30821/files
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wenum-constexpr-conversion"
#endif

#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <websocketpp/config/asio_client.hpp>
#include "websocketpp/client.hpp"


#if defined(__clang__)
#pragma clang diagnostic pop
#endif

typedef websocketpp::client<websocketpp::config::asio_client> client;

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

// pull out the type of messages sent by our config
typedef websocketpp::config::asio_client::message_type::ptr message_ptr;


static auto receive_start = std::chrono::high_resolution_clock::now();


static int64_t receive_count = 0;


// This message handler will be invoked once for each incoming message. It
// prints the message and then sends a copy of the message back to the server.
static void on_message(client* c, websocketpp::connection_hdl hdl, message_ptr msg) {
    /*
    if (msg->get_payload().starts_with("start_")) {
        receive_start = std::chrono::high_resolution_clock::now();
        std::cout << "receive message start " << std::endl;
    }
    receive_count ++;

    if (msg->get_payload().starts_with("end_") || receive_count % 10000 == 0) {
        auto end = std::chrono::high_resolution_clock::now();
        auto used = std::chrono::duration_cast<std::chrono::milliseconds>(end - receive_start);
        std::cout << "receive message " << used.count() << "ms " << receive_count  << std::endl;
    } */
}


/**
 128
 start new turn total message 2000002
 receive used 8686.95ms
 
 1024
 
 start new turn total message 4000002
 receive used 11824.5ms
 send message speed 3906.25 mb
 400-500mb/s
 
 
 4096
 start new turn total message 2000002
 receive used 6941.19ms
 send message speed 7812 mb
 
 1.13gb / s
 
 start new turn total message 4000002
 receive used 19493.1ms
 send message speed  15625 mb
 0.78 gb/s
 
 
 4096*2
 start new turn total message 2000002
 receive used 9383.77ms
 send message speed 15625 mb
 1.61gb/s
 
 
 4096*4
 start new turn total message 2000002
 receive used 32324.3ms
 send message speed 31250 mb
 
 1gb/s
 
  */

static client* mainClient;
static client::connection_ptr mainConn;

static void send_message_bench(const boost::system::error_code&) {
    
    auto start = std::chrono::high_resolution_clock::now();
    auto end = std::chrono::high_resolution_clock::now();
    auto used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
   
    std::cout << "send message bench start " << std::endl;
   
    websocketpp::lib::error_code ec;
    mainClient->send(mainConn, "start_", websocketpp::frame::opcode::value::TEXT, ec);
    start = std::chrono::high_resolution_clock::now();
   
    std::string msg(1024, 'a');
    
    size_t length = 10000*200;
    for(int i=0; i<length; i++) {
        mainClient->send(mainConn, msg, websocketpp::frame::opcode::value::BINARY, ec);
    }
    mainClient->send(mainConn, "end_", websocketpp::frame::opcode::value::TEXT, ec);
    end = std::chrono::high_resolution_clock::now();
    used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "send message done " << used.count() << "ms "  << std::endl;
    std::cout << "send message total " << (length)*msg.length() / (1024*1024.0) << " mb "  << std::endl;
    
    
    /**
    boost::asio::steady_timer timer(mainClient->get_io_service());
    timer.expires_after(std::chrono::seconds(20));
    timer.async_wait([](const boost::system::error_code&){
        mainConn->close(websocketpp::close::status::service_restart, "good baye");
    });*/
    
}


int websocket_pp_no_tts_test_main(int argc, const char * argv[]) {
    auto start = std::chrono::high_resolution_clock::now();
    auto end = std::chrono::high_resolution_clock::now();
    auto used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    
    client client;
    std::string uri = "ws://127.0.0.1:9001/wsg?role=server&serverAppId=081713074824&serverAppToken=Y8FG0tbs7pgujQccNcEABIuW1it2Rhfw";

    try {
        // Set logging to be pretty verbose (everything except message payloads)
        client.set_access_channels(websocketpp::log::alevel::none);
        //c.clear_access_channels(websocketpp::log::alevel::frame_payload);

        
        // Initialize ASIO
        client.init_asio();

        // Register our message handler
        client.set_message_handler(bind(&on_message,&client,::_1,::_2));

        websocketpp::lib::error_code ec;
        client::connection_ptr con =  client.get_connection(uri, ec);
        if (ec) {
            std::cout << "could not create connection because: " << ec.message() << std::endl;
            return 0;
        }
        
        con->set_fail_handler([](websocketpp::connection_hdl hdl) {
            std::cout << "client failed connect " << std::endl;
            mainClient = nullptr;
        });
        con->set_close_handler([](websocketpp::connection_hdl hdl){
            std::cout << "client be closed " << std::endl;
            mainClient = nullptr;
        });

        // Note that connect here only requests a connection. No network messages are
        // exchanged until the event loop starts running in the next line.
        mainConn = client.connect(con);
        mainClient = &client;
    
    
       //boost::asio::steady_timer timer(client.get_io_service());
       //timer.expires_after(std::chrono::seconds(1));
       //timer.async_wait(send_message_bench);

        // Start the ASIO io_service run loop
        // this will cause a single connection to be made to the server. c.run()
        // will exit when this connection is closed.
        
        client.run();
    } catch (websocketpp::exception const & e) {
        std::cout << "websocket exception " << e.what() << std::endl;
    }
    
    
    return 0;
}
