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
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "websocketpp/config/asio_no_tls_client.hpp"
#include "websocketpp/client.hpp"

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
    if (msg->get_payload().starts_with("start_")) {
        receive_start = std::chrono::high_resolution_clock::now();
        std::cout << "receive message start " << std::endl;
    }
    receive_count ++;

    if (msg->get_payload().starts_with("end_") || receive_count % 10000 == 0) {
        auto end = std::chrono::high_resolution_clock::now();
        auto used = std::chrono::duration_cast<std::chrono::milliseconds>(end - receive_start);
        std::cout << "receive message " << used.count() << "ms " << receive_count  << std::endl;
    }
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
websocketpp::lib::shared_ptr<boost::asio::ssl::context> ctx;

void connect(client* client, const std::string uri) {
    websocketpp::lib::error_code ec;
    client::connection_ptr con =  client->get_connection(uri, ec);
    if (ec) {
        std::cout << "could not create connection because: " << ec.message() << std::endl;
        return;
    }
    con->set_open_handler([](websocketpp::connection_hdl hdl){
        std::cout << "client connect open " << &hdl << std::endl;
       
    });
    
    con->set_fail_handler([=](websocketpp::connection_hdl hdl) {
        std::cout << "client failed connect " << &hdl << std::endl;
        std::cout << "client failed connect try reconnect " << std::endl;
        
        boost::asio::steady_timer timer(mainClient->get_io_service());
        timer.expires_after(std::chrono::seconds(3));
        timer.async_wait([=] (const boost::system::error_code& error_code){
            std::cout << "client failed connect reconnect " << std::endl;
            connect(client, uri);
            std::cout << "client failed connect connect again " << std::endl;
            
        });
    });
    con->set_close_handler([=](websocketpp::connection_hdl hdl){
        std::cout << "client be closed " << &hdl << std::endl;
        //timer
        boost::asio::steady_timer timer(mainClient->get_io_service());
        timer.expires_after(std::chrono::seconds(3));
        timer.async_wait([=] (const boost::system::error_code& error_code){
            std::cout << "client failed connect reconnect " << std::endl;
            connect(mainClient, uri);
            std::cout << "client failed connect connect again " << std::endl;
            
        });
    });
    con->set_message_handler([](websocketpp::connection_hdl hdl, message_ptr msg){
        std::cout << "on message client" << &hdl << " " << msg->get_payload() << std::endl;
        
    });
    client::connection_ptr mainConn = client->connect(con);
   
}

int websocket_router_no_tts_test_main(int argc, const char * argv[]) {
    auto start = std::chrono::high_resolution_clock::now();
    auto end = std::chrono::high_resolution_clock::now();
    auto used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    
    client client;
    std::string uri = "ws://127.0.0.1:9001/wsg?role=server&serverAppId=081713074824&serverAppToken=Y8FG0tbs7pgujQccNcEABIuW1it2Rhfw";

    try {
        // Set logging to be pretty verbose (everything except message payloads)
        client.set_access_channels(websocketpp::log::alevel::all);
        client.set_error_channels(websocketpp::log::elevel::all);
        //c.clear_access_channels(websocketpp::log::alevel::frame_payload);

        
        // Initialize ASIO
        client.init_asio();
        
        ctx = websocketpp::lib::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::tlsv13_client);
         client.set_tls_init_handler([](websocketpp::connection_hdl hdl) {
             return ctx;
          });

        // Register our message handler
        connect(&client, uri);
        connect(&client, uri);
        mainClient = &client;
       //boost::asio::steady_timer timer(client.get_io_service());
       //timer.expires_after(std::chrono::seconds(1));
       //timer.async_wait(send_message_bench);

        // Start the ASIO io_service run loop
        // this will cause a single connection to be made to the server. c.run()
        // will exit when this connection is closed.
        
        client.run();
    } catch (websocketpp::exception const & e) {
        std::cout << "router connect websocket exception " << e.what() << std::endl;
    }
    
    
    return 0;
}
