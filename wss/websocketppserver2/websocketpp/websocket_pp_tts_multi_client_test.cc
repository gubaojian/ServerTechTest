
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

#include <thread>

typedef websocketpp::client<websocketpp::config::asio_tls_client> client;

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

// pull out the type of messages sent by our config
typedef websocketpp::config::asio_client::message_type::ptr message_ptr;



void on_message_hello(client* c, websocketpp::connection_hdl hdl, message_ptr msg) {
    std::cout << "on_message called with hdl: " << hdl.lock().get()
              << " and message: " << msg->get_payload()
              << std::endl;


    websocketpp::lib::error_code ec;

    c->send(hdl, msg->get_payload(), msg->get_opcode(), ec);
    if (ec) {
        std::cout << "Echo failed because: " << ec.message() << std::endl;
    }
}


class WsClient {
public:
    WsClient(const std::string& _url) {
        url = _url;
    }
    
    void on_message(client* c, websocketpp::connection_hdl hdl, message_ptr msg) {
        std::cout << "on_message called with hdl: " << hdl.lock().get()
                  << " and message: " << msg->get_payload()
                  << std::endl;


        websocketpp::lib::error_code ec;

        c->send(hdl, msg->get_payload(), msg->get_opcode(), ec);
        if (ec) {
            std::cout << "Echo failed because: " << ec.message() << std::endl;
        }
    }
    
    void send_message_bench(const boost::system::error_code&) {
        
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
        std::cout << "send message speed " << (length)*msg.length() / (1024*1024.0) << " mb "  << std::endl;
        
        
        /**
        boost::asio::steady_timer timer(mainClient->get_io_service());
        timer.expires_after(std::chrono::seconds(20));
        timer.async_wait([](const boost::system::error_code&){
            mainConn->close(websocketpp::close::status::service_restart, "good baye");
        });*/
        
    }
    
    void init() {
        client client;
        mainClient = &client;
        client.set_access_channels(websocketpp::log::alevel::none);
        // Set logging to be pretty verbose (everything except message payloads)
        client.set_access_channels(websocketpp::log::alevel::none);
        //c.clear_access_channels(websocketpp::log::alevel::frame_payload);

        
        // Initialize ASIO
        client.init_asio();
        
        
       ctx = websocketpp::lib::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::tlsv13_client);
        //client.set_tls_init_handler();
        client.set_tls_init_handler([this](websocketpp::connection_hdl hdl) {
            //std::cout << ctx << std::endl;
            /**
            ctx->set_options(boost::asio::ssl::context::default_workarounds |
                                     boost::asio::ssl::context::no_sslv2 |
                                     boost::asio::ssl::context::no_sslv3 |
                                     boost::asio::ssl::context::single_dh_use);
            ctx->set_verify_mode(boost::asio::ssl::verify_peer);
            
            ctx->set_verify_callback([](bool preverified, boost::asio::ssl::verify_context& ctx){
                std::cout << "verify connect" << preverified << std::endl;
                return true;
            }); */
            return ctx;
         });

        // Register our message handler
        client.set_message_handler(bind(&WsClient::on_message, this, &client, ::_1,::_2));

        websocketpp::lib::error_code ec;
        client::connection_ptr con =  client.get_connection(url, ec);
        if (ec) {
            std::cout << "could not create connection because: " << ec.message() << std::endl;
            return;
        }
        
        con->set_fail_handler([this](websocketpp::connection_hdl hdl) {
            std::cout << "client failed connect " << std::endl;
            mainClient = nullptr;
        });
        con->set_close_handler([this](websocketpp::connection_hdl hdl){
            std::cout << "client be closed " << std::endl;
            mainClient = nullptr;
        });

        // Note that connect here only requests a connection. No network messages are
        // exchanged until the event loop starts running in the next line.
        mainConn = client.connect(con);
    
    
        /**
    
       boost::asio::steady_timer timer(client.get_io_service());
       timer.expires_after(std::chrono::seconds(1));
        timer.async_wait(bind(&WsClient::send_message_bench, this, ::_1));
         */
        // Start the ASIO io_service run loop
        // this will cause a single connection to be made to the server. c.run()
        // will exit when this connection is closed.
        
        client.run();
       
    }
    
private:
    std::string url;
    client* mainClient;
    client::connection_ptr mainConn;
    websocketpp::lib::shared_ptr<boost::asio::ssl::context> ctx;
};


/**
 1KB数据：
 1个线程：30万每秒 300mb/s
 2个线程：50万每秒  600MB/s
 3个线程：80万每秒  900MB/s
 4个线程：60万每秒 700-800MB/s
 
 说明服务端实现的性能很高。完全发挥了性能。
  
 
 // 创建一个线程执行类的成员函数
    MyClass obj;
    std::thread t2(&MyClass::printMessage, &obj);
 */
int websocket_pp_tts_test_multi_client_main(int argc, const char * argv[]) {
    auto start = std::chrono::high_resolution_clock::now();
    auto end = std::chrono::high_resolution_clock::now();
    auto used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
 
    try {
        std::thread senderOne([]{
            std::cout << "websocket send one start " << std::endl;
            std::string url = "wss://localhost:9001";
            WsClient client(url);
            client.init();
        });
        
        std::thread senderTwo([]{
            std::cout << "websocket send two start " << std::endl;
            //std::string url = "wss://localhost:9001";
            //WsClient client(url);
            //client.init();
        });
        
        std::thread senderThree([]{
            std::cout << "websocket send threee start " << std::endl;
            //std::string url = "wss://localhost:9001";
            //WsClient client(url);
            //client.init();
        });
        
        std::thread senderFour([]{
            std::cout << "websocket send threee start " << std::endl;
            //std::string url = "wss://localhost:9001";
            //WsClient client(url);
            //client.init();
        });
        
        senderOne.join();
        senderTwo.join();
        senderThree.join();
        senderFour.join();
    } catch (websocketpp::exception const & e) {
        std::cout << "websocket exception " << e.what() << std::endl;
    }
    
    
    return 0;
}
