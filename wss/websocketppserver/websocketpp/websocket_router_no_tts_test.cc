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

#include <websocketpp/config/asio_client.hpp>
#include "websocketpp/client.hpp"



typedef websocketpp::client<websocketpp::config::asio_client> plain_client;
typedef websocketpp::client<websocketpp::config::asio_tls_client> tls_client;
typedef websocketpp::config::asio_client::message_type::ptr message_ptr;

struct plain_connection{
    plain_client::connection_ptr conn = nullptr;
    int64_t receiveMessageCount = 0;
    int64_t sendMessageCount = 0;
};

struct tls_connection{
    tls_client::connection_ptr conn = nullptr;
    int64_t receiveMessageCount = 0;
    int64_t sendMessageCount = 0;
};

void connect_plain(plain_client* client, const std::string uri) {
    websocketpp::lib::error_code ec;
    plain_client::connection_ptr con =  client->get_connection(uri, ec);
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
        //https://stackoverflow.com/questions/76819327/asio-async-wait-gives-operation-canceled
        std::shared_ptr<boost::asio::steady_timer> timer = std::make_shared<boost::asio::steady_timer>(client->get_io_service());
        timer->expires_after(std::chrono::seconds(30 + std::rand()%30));
        timer->async_wait([timer, client, uri] (const boost::system::error_code& error_code){
            if (error_code) {
                std::cout << "error code " << error_code.message()
                << " " << " " << error_code.location() << std::endl;
                return ;
            }
            std::cout << "client failed connect reconnect " << error_code << std::endl;
            connect_plain(client, uri);
            std::cout << "client failed connect connect again " << std::endl;
            
        });
        con->set_fail_handler(nullptr);
    });
    con->set_close_handler([=](websocketpp::connection_hdl hdl){
        std::cout << "client be closed " << &hdl << std::endl;
        //timer
        std::shared_ptr<boost::asio::steady_timer> timer = std::make_shared<boost::asio::steady_timer>(client->get_io_service());
        timer->expires_after(std::chrono::seconds(30 + std::rand()%30));
        timer->async_wait([timer, client, uri] (const boost::system::error_code& error_code){
            if (error_code) {
                std::cout << "error code " << error_code.message()
                << " " << " " << error_code.location() << std::endl;
                return ;
            }
            std::cout << "client failed connect reconnect " << error_code << std::endl;
            connect_plain(client, uri);
            std::cout << "client failed connect connect again " << std::endl;
            
        });
        con->set_close_handler(nullptr);
    });
    con->set_message_handler([](websocketpp::connection_hdl hdl, message_ptr msg){
        std::cout << "on message client" << &hdl << " " << msg->get_payload() << std::endl;
        
    });
   plain_client::connection_ptr mainConn = client->connect(con);
  
    mainConn = nullptr;
   
}

void connect_tls(const tls_client* client, const std::string uri) {
    
}

void runWSRouter() {
    auto start = std::chrono::high_resolution_clock::now();
    auto end = std::chrono::high_resolution_clock::now();
    auto used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    
    plain_client client;
    std::string uri = "ws://127.0.0.1:9001/wsg?role=server&serverAppId=081713074824&serverAppToken=Y8FG0tbs7pgujQccNcEABIuW1it2Rhfw";

    try {
        // Set logging to be pretty verbose (everything except message payloads)
        client.set_access_channels(websocketpp::log::alevel::none);
        client.set_error_channels(websocketpp::log::elevel::none);
        
        // Initialize ASIO
        client.init_asio();
        // Register our message handler
        connect_plain(&client, uri);
        connect_plain(&client, uri);
       //boost::asio::steady_timer timer(client.get_io_service());
       //timer.expires_after(std::chrono::seconds(1));
       //timer.async_wait(send_message_bench);

        // Start the ASIO io_service run loop
        // this will cause a single connection to be made to the server. c.run()
        // will exit when this connection is closed.
        plain_client* ptr = &client;
        std::thread background([ptr] {
            auto start = std::chrono::high_resolution_clock::now();
            for(int i=0; i<10000*200; i++) {
                boost::asio::post(ptr->get_io_service(), [] {
                    //std::cout << "post execute command";
                });
            }
            auto end = std::chrono::high_resolution_clock::now();
            auto used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            std::cout << "post message done " << used.count() << "ms "  << std::endl;
           
        });
        
        client.run();
    } catch (websocketpp::exception const & e) {
        std::cout << "router connect websocket exception " << e.what() << std::endl;
    }
}

void runWSSRouter() {
    auto start = std::chrono::high_resolution_clock::now();
    auto end = std::chrono::high_resolution_clock::now();
    auto used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    
    tls_client client;
    websocketpp::lib::shared_ptr<boost::asio::ssl::context> ctx;
   
    std::string uri = "ws://127.0.0.1:9001/wsg?role=server&serverAppId=081713074824&serverAppToken=Y8FG0tbs7pgujQccNcEABIuW1it2Rhfw";

    try {
        // Set logging to be pretty verbose (everything except message payloads)
        client.set_access_channels(websocketpp::log::alevel::all);
        client.set_error_channels(websocketpp::log::elevel::all);
        //c.clear_access_channels(websocketpp::log::alevel::frame_payload);

        
        // Initialize ASIO
        client.init_asio();
        
        ctx = websocketpp::lib::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::tlsv13_client);
         client.set_tls_init_handler([=](websocketpp::connection_hdl hdl) {
             return ctx;
          });

        // Register our message handler
        connect_tls(&client, uri);
        connect_tls(&client, uri);
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
}

int websocket_router_no_tts_test_main(int argc, const char * argv[]) {
    runWSRouter();
    
    return 0;
}
