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

#include "simdjson/simdjson.h"


//enable_multithreading should be set to false

typedef websocketpp::client<websocketpp::config::asio_client> plain_client;
typedef websocketpp::client<websocketpp::config::asio_tls_client> tls_client;
typedef websocketpp::config::asio_client::message_type::ptr message_ptr;

struct HwssServer {
    std::string hwssId;
    std::string url;
};

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

struct ServerFinder {
    //ws协议hwss服务器id映射
    std::unordered_map<std::string,HwssServer> wsServerMap;
    //wss协议hwss服务器id映射
    std::unordered_map<std::string,HwssServer> wssServerMap;
    
    //ws协议asio的执行线程及相关信息：。仅在支持对应线程访问
    std::shared_ptr<plain_client> plainClient;
    std::unordered_map<std::string, plain_client::connection_ptr> wsConnMap;
    
    //wss协议asio的执行线程及相关信息：。仅在支持对应线程访问
    std::shared_ptr<tls_client> tlsClient;
    std::unordered_map<std::string, tls_client::connection_ptr> wssConnMap;
    
    //ws和wss两个线程分别对应两个parser。仅在支持对应线程访问
    simdjson::ondemand::parser plainParser;
    simdjson::ondemand::parser tlsParser;
    
};

std::shared_ptr<ServerFinder> serverFinder;

void try_connect_plain_later(const std::string hwssId);
void handleMsgFromWssRouter(std::shared_ptr<std::string> msg);

void try_connect_tls_later(const std::string hwssId);
void handleMsgFromWsRouter(std::shared_ptr<std::string> msg);

void connect_plain(const std::string connHwssId) {
    if (connHwssId.empty()) {
        std::cout << "illegal hwssId, is empty " << connHwssId << std::endl;
        return;
    }
    auto findIt = serverFinder->wsServerMap.find(connHwssId);
    if (findIt == serverFinder->wsServerMap.end()) {
        return;
    }
    const HwssServer& server = findIt->second;
    std::shared_ptr<plain_client> client = serverFinder->plainClient;
    websocketpp::lib::error_code ec;
    plain_client::connection_ptr conn =  client->get_connection(server.url, ec);
    if (ec) {
        std::cout << "could not get_connection because: " << ec.message() << std::endl;
        try_connect_plain_later(connHwssId);
        return;
    }
    conn->set_open_handshake_timeout(240*1000);
    conn->set_close_handshake_timeout(240*1000);
    conn->set_pong_timeout(480*1000);
    conn->set_ping_handler([](websocketpp::connection_hdl hdl, const std::string msg){
        return true; //send pong response
    });
    
    conn->set_pong_timeout_handler([connHwssId](websocketpp::connection_hdl hdl, const std::string msg){
        std::cout << "router ws ping timeout " << connHwssId << std::endl;
    });
    
    conn->set_open_handler([conn, connHwssId](websocketpp::connection_hdl hdl){
        std::cout << "router pong handler success " << connHwssId << std::endl;
        serverFinder->wsConnMap[connHwssId] = conn;
    });
    
    conn->set_message_handler([connHwssId](websocketpp::connection_hdl hdl, message_ptr msg) {
        //router current only handle json text
        if(msg->get_opcode() == websocketpp::frame::opcode::value::TEXT
           || msg->get_opcode() == websocketpp::frame::opcode::value::BINARY) {
            simdjson::ondemand::document  doc;
            simdjson::padded_string paddedJson = simdjson::padded_string(msg->get_payload());
            auto error = serverFinder->plainParser.iterate(paddedJson).get(doc);
            if (error) {
                std::cout << connHwssId << " send wrong format message" << msg->get_payload() << std::endl;
                return;
            }
            auto hwssId = doc["hwssId"].get_string();
            if (hwssId.error()) {
                std::cout << hwssId << " send wrong format message" << std::endl;
                return;
            }
            //默认到ws协议的处理
            std::string toHwssId(hwssId.value().data(), hwssId.value().size());
            if (connHwssId == toHwssId) {
                return;
            }
            auto connIt = serverFinder->wsConnMap.find(toHwssId);
            if (connIt != serverFinder->wsConnMap.end()) {
                websocketpp::lib::error_code send_error;
                auto sendMessage = connIt->second->get_message(websocketpp::frame::opcode::value::BINARY, msg->get_payload().size());
                sendMessage->set_payload(msg->get_payload());
                sendMessage->set_compressed(false); //disable compress, improve performance
                serverFinder->plainClient->send(connIt->second, sendMessage send_error);
                if (send_error) {
                    std::cout << "hwssId ws " << toHwssId << " send error "<< send_error
                      << "state " << connIt->second->get_state()<< std::endl;
                  
                }
                return;
            }
           
            if (serverFinder->wssServerMap.empty()) {
                return;
            }
            //转发到wss协议的处理线程
            std::shared_ptr<std::string> cpMsg = std::make_shared<std::string>(msg->get_payload());
            std::shared_ptr<tls_client> tlsClient = serverFinder->tlsClient;
            if (tlsClient) {
                boost::asio::post(tlsClient->get_io_service(), [cpMsg] {
                    handleMsgFromWsRouter(cpMsg);
                });
            }
        }
        
    });
    
    conn->set_fail_handler([connHwssId](websocketpp::connection_hdl hdl) {
        //connect will be free in inner
        std::cout << "router failed ws connect try reconnect " << connHwssId << std::endl;
        serverFinder->wsConnMap.erase(connHwssId);
        try_connect_plain_later(connHwssId);
       
    });
    conn->set_close_handler([connHwssId](websocketpp::connection_hdl hdl){
        //connect will be free in inner
        std::cout << "router closed ws connect, try reconnect " << connHwssId << std::endl;
        serverFinder->wsConnMap.erase(connHwssId);
        try_connect_plain_later(connHwssId);
        
    });
    
    client->connect(conn);
}

void try_connect_plain_later(const std::string hwssId) {
    std::shared_ptr<boost::asio::steady_timer> timer = std::make_shared<boost::asio::steady_timer>(serverFinder->plainClient->get_io_service());
    timer->expires_after(std::chrono::seconds(10 + std::rand()%20));
    timer->async_wait([timer, hwssId] (const boost::system::error_code& error_code){
        if (error_code) {
            std::cout << hwssId << " reconnect error code " << error_code.message() << std::endl;
            return ;
        }
        connect_plain(hwssId);
    });
}

void handleMsgFromWssRouter(std::shared_ptr<std::string> msg) {
    if (!serverFinder->plainClient) {
        return;
    }
    simdjson::ondemand::document  doc;
    simdjson::padded_string paddedJson = simdjson::padded_string(*msg);
    auto error = serverFinder->plainParser.iterate(paddedJson).get(doc);
    if (error) {
        return;
    }
    auto hwssId = doc["hwssId"].get_string();
    if (hwssId.error()) {
        return;
    }
    std::string hwssIdStr(hwssId.value().data(), hwssId.value().size());
    auto connIt = serverFinder->wsConnMap.find(hwssIdStr);
    if (connIt != serverFinder->wsConnMap.end()) {
        websocketpp::lib::error_code send_error;
        auto sendMessage = connIt->second->get_message(websocketpp::frame::opcode::value::BINARY, msg->size());
        sendMessage->set_payload(*msg);
        sendMessage->set_compressed(false);
        serverFinder->plainClient->send(connIt->second, sendMessage, send_error);
        if (send_error) {
            std::cout << "hwssId ws " << hwssIdStr << " send error "<< send_error << std::endl;
          
        }
        return;
    }
}

void ping_connect_plain() {
    std::shared_ptr<boost::asio::steady_timer> timer = std::make_shared<boost::asio::steady_timer>(serverFinder->plainClient->get_io_service());
    timer->expires_after(std::chrono::seconds(60 + std::rand()%60));
    timer->async_wait([timer] (const boost::system::error_code& error_code){
        if (error_code) {
            std::cout << "ping_connect_tls error code " << error_code.message() << std::endl;
            return ;
        }
        for(auto wsIt = serverFinder->wsServerMap.begin();
            wsIt != serverFinder->wsServerMap.end(); wsIt++) {
           const auto& hwssId = wsIt->first;
            auto connIt = serverFinder->wsConnMap.find(hwssId);
            if (connIt != serverFinder->wsConnMap.end()) {
                if (connIt->second->get_state() == websocketpp::session::state::value::open) {
                    connIt->second->ping("");
                    std::cout << "ping_connect_plain keep alive "<< hwssId << std::endl;
                }
            }
        }
        std::cout << "ping_connect_tls keep alive" << std::endl;
        ping_connect_plain();
    });
}



void connect_tls(const std::string connHwssId) {
    auto findIt = serverFinder->wssServerMap.find(connHwssId);
    if (findIt == serverFinder->wssServerMap.end()) {
        return;
    }
    const HwssServer& server = findIt->second;
    std::shared_ptr<tls_client> client = serverFinder->tlsClient;
    websocketpp::lib::error_code ec;
    tls_client::connection_ptr conn =  client->get_connection(server.url, ec);
    if (ec) {
        std::cout << "could not get_connection because: " << ec.message() << std::endl;
        try_connect_tls_later(connHwssId);
        return;
    }
    conn->set_open_handshake_timeout(240*1000);
    conn->set_close_handshake_timeout(240*1000);
    conn->set_pong_timeout(480*1000);
    conn->set_ping_handler([](websocketpp::connection_hdl hdl, const std::string msg){
        return true; //send pong response
    });
    
    conn->set_pong_timeout_handler([connHwssId](websocketpp::connection_hdl hdl, const std::string msg){
        std::cout << "router wss ping timeout " << connHwssId << std::endl;
    });

    
    conn->set_pong_handler([connHwssId](websocketpp::connection_hdl hdl, std::string msg){
        std::cout << "router pong handler wss " << connHwssId << std::endl;
    });
    
    conn->set_open_handler([conn, connHwssId](websocketpp::connection_hdl hdl){
        std::cout << "router connect success wss " << connHwssId << std::endl;
        serverFinder->wssConnMap[connHwssId] = conn;
    });
    
    conn->set_message_handler([connHwssId](websocketpp::connection_hdl hdl, message_ptr msg) {
        //router current only handle json text
        if(msg->get_opcode() == websocketpp::frame::opcode::value::TEXT
           || msg->get_opcode() == websocketpp::frame::opcode::value::BINARY) {
            simdjson::ondemand::document  doc;
            simdjson::padded_string paddedJson = simdjson::padded_string(msg->get_payload());
            auto error = serverFinder->tlsParser.iterate(paddedJson).get(doc);
            if (error) {
                return;
            }
            auto hwssId = doc["hwssId"].get_string();
            if (hwssId.error()) {
                std::cout << connHwssId << " send wrong format message" << std::endl;
                return;
            }
            //默认到ws协议的处理
            std::string toHwssId(hwssId.value().data(), hwssId.value().size());
            if (toHwssId == connHwssId) {
                return;
            }
            auto connIt = serverFinder->wssConnMap.find(toHwssId);
            if (connIt != serverFinder->wssConnMap.end()) {
                //if (connIt->second->get_state() == websocketpp::session::state::value::open) {
                    websocketpp::lib::error_code send_error;
                    auto sendMessage = connIt->second->get_message(websocketpp::frame::opcode::value::BINARY, msg->get_payload().size());
                sendMessage->set_payload(msg->get_payload());
                    sendMessage->set_compressed(false); //disable compress
                    serverFinder->tlsClient->send(connIt->second,sendMessage, send_error);
                    if (send_error) {
                        std::cout << "hwssId wss " << toHwssId << " send error "<< send_error
                         << " state "<< connIt->second->get_state() << std::endl;
                      
                    }
                //}
                return;
            }
           
            if (serverFinder->wsServerMap.empty()) {
                return;
            }
            //转发到ws协议的处理线程
            std::shared_ptr<std::string> cpMsg = std::make_shared<std::string>(msg->get_payload());
            std::shared_ptr<plain_client> plainClient = serverFinder->plainClient;
            if (plainClient) {
                boost::asio::post(plainClient->get_io_service(), [cpMsg] {
                    handleMsgFromWssRouter(cpMsg);
                });
            }
        }
        
    });
    
    conn->set_fail_handler([connHwssId](websocketpp::connection_hdl hdl) {
        //connect will be free in inner
        std::cout << "router failed wss connect try reconnect tls " << connHwssId << std::endl;
        serverFinder->wssConnMap.erase(connHwssId);
        try_connect_tls_later(connHwssId);
       
    });
    conn->set_close_handler([connHwssId](websocketpp::connection_hdl hdl){
        //connect will be free in inner
        serverFinder->wssConnMap.erase(connHwssId);
        try_connect_tls_later(connHwssId);
        std::cout << "router closed wss connect, try reconnect tls "  << connHwssId <<
         (serverFinder->wssConnMap.find(connHwssId) == serverFinder->wssConnMap.end()) << true
         << false << " size " << serverFinder->wssConnMap.size() << std::endl;
        
    });
    client->connect(conn);
}


void try_connect_tls_later(const std::string hwssId) {
    std::shared_ptr<boost::asio::steady_timer> timer = std::make_shared<boost::asio::steady_timer>(serverFinder->tlsClient->get_io_service());
    timer->expires_after(std::chrono::seconds(10 + std::rand()%20));
    timer->async_wait([timer, hwssId] (const boost::system::error_code& error_code){
        if (error_code) {
            std::cout << hwssId << " reconnect error code " << error_code.message() << std::endl;
            return ;
        }
        std::cout << "router try reconnect timer fired "  << std::endl;
        connect_tls(hwssId);
    });
}

void handleMsgFromWsRouter(std::shared_ptr<std::string> msg) {
    if (!serverFinder->tlsClient) {
        return;
    }
    simdjson::ondemand::document  doc;
    simdjson::padded_string paddedJson = simdjson::padded_string(*msg);
    auto error = serverFinder->tlsParser.iterate(paddedJson).get(doc);
    if (error) {
        return;
    }
    auto hwssId = doc["hwssId"].get_string();
    if (hwssId.error()) {
        return;
    }
    std::string hwssIdStr(hwssId.value().data(), hwssId.value().size());
    auto connIt = serverFinder->wssConnMap.find(hwssIdStr);
    if (connIt != serverFinder->wssConnMap.end()) {
        //if (connIt->second->get_state() == websocketpp::session::state::value::open) {
            websocketpp::lib::error_code send_error;
        auto sendMessage = connIt->second->get_message(websocketpp::frame::opcode::value::BINARY, msg->size());
        sendMessage->set_payload(*msg);
        sendMessage->set_compressed(false); //disable compress
            serverFinder->tlsClient->send(connIt->second, sendMessage, send_error);
            if (send_error) {
                std::cout << "hwssId wss " << hwssIdStr << " send error  "<< send_error << "state"
                << connIt->second->get_state()
                << std::endl;
                
            }
            return;
        //}
    }
}


void ping_connect_tls() {
    std::shared_ptr<boost::asio::steady_timer> timer = std::make_shared<boost::asio::steady_timer>(serverFinder->tlsClient->get_io_service());
    timer->expires_after(std::chrono::seconds(60 + std::rand()%60));
    timer->async_wait([timer] (const boost::system::error_code& error_code){
        if (error_code) {
            std::cout << "ping_connect_tls error code " << error_code.message() << std::endl;
            return ;
        }
        for(auto wsIt = serverFinder->wssServerMap.begin();
            wsIt != serverFinder->wssServerMap.end(); wsIt++) {
           const auto& hwssId = wsIt->first;
            auto connIt = serverFinder->wssConnMap.find(hwssId);
            if (connIt != serverFinder->wssConnMap.end()) {
                if (connIt->second->get_state() == websocketpp::session::state::value::open) {
                    connIt->second->ping("");
                    std::cout << "ping_connect_tls keep alive "<< hwssId << std::endl;
                }
            }
        }
        std::cout << "ping_connect_tls keep alive" << std::endl;
        ping_connect_tls();
    });
}

void runWSRouter() {
    std::shared_ptr<plain_client> client = std::make_shared<plain_client>();
    serverFinder->plainClient = client;

    try {
        // Set logging to be pretty verbose (everything except message payloads)
        client->set_access_channels(websocketpp::log::alevel::none);
        client->clear_access_channels(websocketpp::log::alevel::none);
        client->set_error_channels(websocketpp::log::elevel::none);
        
        // Initialize ASIO
        client->init_asio();
        
        
        const auto& wsServerMap = serverFinder->wsServerMap;
        auto wsIt = wsServerMap.begin();
        for(auto wsIt = wsServerMap.begin(); wsIt != wsServerMap.end(); wsIt++) {
            connect_plain(wsIt->first);
        }
        
        ping_connect_plain();
        
        client->run();
    } catch (websocketpp::exception const & e) {
        std::cout << "router connect websocket exception " << e.what()
        << e.code() << std::endl;
    }
    serverFinder->plainClient = nullptr;
    
}

void runWSSRouter() {
    std::shared_ptr<tls_client> client = std::make_shared<tls_client>();
    websocketpp::lib::shared_ptr<boost::asio::ssl::context> ctx;
    serverFinder->tlsClient = client;
    try {
        // Set logging to be pretty verbose (everything except message payloads)
        client->set_access_channels(websocketpp::log::alevel::none);
        client->clear_access_channels(websocketpp::log::alevel::none);
        client->set_error_channels(websocketpp::log::elevel::none);
        
        // Initialize ASIO
        client->init_asio();
        
        ctx = websocketpp::lib::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::tlsv13_client);
         client->set_tls_init_handler([=](websocketpp::connection_hdl hdl) {
             return ctx;
          });

        const auto& wssServerMap = serverFinder->wssServerMap;
        for(auto wssIt = wssServerMap.begin(); wssIt != wssServerMap.end(); wssIt++) {
            connect_tls(wssIt->first);
        }

        ping_connect_tls();
        
        
        client->run();
    } catch (websocketpp::exception const & e) {
        std::cout << "wss router connect websocket exception " << e.what()
        << e.code() << std::endl;
    } catch (std::exception const & e) {
        std::cout << "wss router connect websocket exception " << e.what() << std::endl;
    }
    serverFinder->tlsClient = nullptr;
}

int websocket_router_no_tts_test_main(int argc, const char * argv[]) {
    serverFinder = std::make_shared<ServerFinder>();
    
    
    {
        
        HwssServer wsServer2;
        wsServer2.url = "ws://127.0.0.1:9001/wsg?role=router&routerAppId=162374203942&routerAppToken=ZnPMZ3Wv8GR5ofP3LdNYHFIT3eNGcApg";
        wsServer2.hwssId = "ws_685208380980";
        std::string wsHwssId2 = "ws_685208380980";
        serverFinder->wsServerMap[wsHwssId2] = wsServer2;
    }
 
    {
        
        HwssServer wsServer2;
        wsServer2.url = "ws://127.0.0.1:9002/wsg?role=router&routerAppId=162374203942&routerAppToken=ZnPMZ3Wv8GR5ofP3LdNYHFIT3eNGcApg";
        wsServer2.hwssId = "ws_685208380981";
        std::string wsHwssId2 = "ws_685208380981";
        serverFinder->wsServerMap[wsHwssId2] = wsServer2;
    }
    
    {
        
        HwssServer wsServer;
        wsServer.url = "wss://127.0.0.1:8001/wsg?role=router&routerAppId=162374203942&routerAppToken=ZnPMZ3Wv8GR5ofP3LdNYHFIT3eNGcApg";
        wsServer.hwssId = "wss_685208380980";
        std::string wsHwssId = "wss_685208380980";
        serverFinder->wssServerMap[wsHwssId] = wsServer;
    }
    
    {
        HwssServer wsServer;
        wsServer.url = "wss://127.0.0.1:8002/wsg?role=router&routerAppId=162374203942&routerAppToken=ZnPMZ3Wv8GR5ofP3LdNYHFIT3eNGcApg";
        wsServer.hwssId = "wss_685208380981";
        std::string wsHwssId = "wss_685208380981";
        serverFinder->wssServerMap[wsHwssId] = wsServer;
    }
    
    
    if(serverFinder->wsServerMap.empty() && serverFinder->wssServerMap.empty()) {
        std::cout << "non config hwss servers for router " << std::endl;
        return 0;
    }
    if (!serverFinder->wsServerMap.empty() && !serverFinder->wssServerMap.empty()) {
        std::thread wssThread([] {
            runWSSRouter();
        });
        runWSRouter();
        wssThread.join();
        return 0;
    }
    
    if (!serverFinder->wsServerMap.empty()) {
        runWSRouter();
        return 0;
    }
    
    if (!serverFinder->wssServerMap.empty()) {
        runWSSRouter();
        return 0;
    }
    
    return 0;
}
