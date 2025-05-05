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

void connect_plain(const std::string hwssId) {
    if (hwssId.empty()) {
        std::cout << "illegal hwssId, is empty " << hwssId << std::endl;
        return;
    }
    auto findIt = serverFinder->wsServerMap.find(hwssId);
    if (findIt == serverFinder->wsServerMap.end()) {
        return;
    }
    const HwssServer& server = findIt->second;
    std::shared_ptr<plain_client> client = serverFinder->plainClient;
    websocketpp::lib::error_code ec;
    plain_client::connection_ptr con =  client->get_connection(server.url, ec);
    if (ec) {
        std::cout << "could not get_connection because: " << ec.message() << std::endl;
        try_connect_plain_later(hwssId);
        return;
    }
    con->set_pong_timeout(120*1000);
    con->set_open_handler([hwssId](websocketpp::connection_hdl hdl){
        std::cout << "router connect success " << hwssId << std::endl;
    });
    
    con->set_message_handler([hwssId](websocketpp::connection_hdl hdl, message_ptr msg) {
        //router current only handle json text
        if(msg->get_opcode() == websocketpp::frame::opcode::value::TEXT
           || msg->get_opcode() == websocketpp::frame::opcode::value::BINARY) {
            simdjson::ondemand::document  doc;
            simdjson::padded_string paddedJson = simdjson::padded_string(msg->get_payload());
            auto error = serverFinder->plainParser.iterate(paddedJson).get(doc);
            if (error) {
                std::cout << hwssId << " send wrong format message" << msg->get_payload() << std::endl;
                return;
            }
            auto hwssId = doc["hwssId"].get_string();
            if (hwssId.error()) {
                std::cout << hwssId << " send wrong format message" << std::endl;
                return;
            }
            //默认到ws协议的处理
            std::string hwssIdStr(hwssId.value().data(), hwssId.value().size());
            auto connIt = serverFinder->wsConnMap.find(hwssIdStr);
            if (connIt != serverFinder->wsConnMap.end()) {
                serverFinder->plainClient->send(connIt->second, msg->get_payload(), websocketpp::frame::opcode::value::BINARY);
                return;
            }
           
            if (serverFinder->wssServerMap.empty()) {
                std::cout << "hwssId " << hwssIdStr << " none config for router "<< std::endl;
                return;
            }
            //转发到wss协议的处理线程
            std::shared_ptr<std::string> cpMsg = std::make_shared<std::string>(msg->get_payload());
            boost::asio::post(serverFinder->tlsClient->get_io_service(), [cpMsg] {
                handleMsgFromWsRouter(cpMsg);
            });
        }
        
    });
    
    con->set_fail_handler([con, hwssId](websocketpp::connection_hdl hdl) {
        //connect will be free in inner
        std::cout << "router failed connect try reconnect " << hwssId << std::endl;
        try_connect_plain_later(hwssId);
        serverFinder->wsConnMap.erase(hwssId);
       
    });
    con->set_close_handler([con, hwssId](websocketpp::connection_hdl hdl){
        //connect will be free in inner
        std::cout << "router closed connect, try reconnect " << hwssId << std::endl;
        try_connect_plain_later(hwssId);
        serverFinder->wsConnMap.erase(hwssId);
        
    });
    
    serverFinder->wsConnMap[hwssId] = client->connect(con);
}

void try_connect_plain_later(const std::string hwssId) {
    std::shared_ptr<boost::asio::steady_timer> timer = std::make_shared<boost::asio::steady_timer>(serverFinder->plainClient->get_io_service());
    timer->expires_after(std::chrono::seconds(30 + std::rand()%30));
    timer->async_wait([timer, hwssId] (const boost::system::error_code& error_code){
        if (error_code) {
            std::cout << hwssId << " reconnect error code " << error_code.message() << std::endl;
            return ;
        }
        connect_plain(hwssId);
    });
}

void handleMsgFromWssRouter(std::shared_ptr<std::string> msg) {
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
        serverFinder->plainClient->send(connIt->second, *msg, websocketpp::frame::opcode::value::BINARY);
        return;
    }
    std::cout << "hwssId " << hwssIdStr << " none config for router "<< std::endl;
}



void connect_tls(const std::string hwssId) {
    auto findIt = serverFinder->wssServerMap.find(hwssId);
    if (findIt == serverFinder->wssServerMap.end()) {
        return;
    }
    const HwssServer& server = findIt->second;
    std::shared_ptr<tls_client> client = serverFinder->tlsClient;
    websocketpp::lib::error_code ec;
    tls_client::connection_ptr con =  client->get_connection(server.url, ec);
    if (ec) {
        std::cout << "could not get_connection because: " << ec.message() << std::endl;
        try_connect_tls_later(hwssId);
        return;
    }
    con->set_pong_timeout(120*1000);
    con->set_open_handler([hwssId](websocketpp::connection_hdl hdl){
        std::cout << "router connect success wss " << hwssId << std::endl;
    });
    
    con->set_message_handler([hwssId](websocketpp::connection_hdl hdl, message_ptr msg) {
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
                std::cout << hwssId << " send wrong format message" << std::endl;
                return;
            }
            //默认到ws协议的处理
            std::string hwssIdStr(hwssId.value().data(), hwssId.value().size());
            auto connIt = serverFinder->wssConnMap.find(hwssIdStr);
            if (connIt != serverFinder->wssConnMap.end()) {
                serverFinder->tlsClient->send(connIt->second, msg->get_payload(), websocketpp::frame::opcode::value::BINARY);
                return;
            }
           
            if (serverFinder->wsServerMap.empty()) {
                std::cout << "hwssId " << hwssIdStr << " none config for router "<< std::endl;
                return;
            }
            //转发到ws协议的处理线程
            std::shared_ptr<std::string> cpMsg = std::make_shared<std::string>(msg->get_payload());
            boost::asio::post(serverFinder->plainClient->get_io_service(), [cpMsg] {
                handleMsgFromWssRouter(cpMsg);
            });
        }
        
    });
    
    con->set_fail_handler([con, hwssId](websocketpp::connection_hdl hdl) {
        //connect will be free in inner
        std::cout << "router failed connect try reconnect tls " << hwssId << std::endl;
        try_connect_tls_later(hwssId);
        serverFinder->wsConnMap.erase(hwssId);
       
    });
    con->set_close_handler([con, hwssId](websocketpp::connection_hdl hdl){
        //connect will be free in inner
        std::cout << "router closed connect, try reconnect tls "  << hwssId << std::endl;
        try_connect_tls_later(hwssId);
        serverFinder->wsConnMap.erase(hwssId);
        
    });
    
    serverFinder->wssConnMap[hwssId] = client->connect(con);
}


void try_connect_tls_later(const std::string hwssId) {
    std::shared_ptr<boost::asio::steady_timer> timer = std::make_shared<boost::asio::steady_timer>(serverFinder->tlsClient->get_io_service());
    timer->expires_after(std::chrono::seconds(30 + std::rand()%30));
    timer->async_wait([timer, hwssId] (const boost::system::error_code& error_code){
        if (error_code) {
            std::cout << hwssId << " reconnect error code " << error_code.message() << std::endl;
            return ;
        }
        connect_tls(hwssId);
    });
}

void handleMsgFromWsRouter(std::shared_ptr<std::string> msg) {
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
        serverFinder->tlsClient->send(connIt->second, *msg, websocketpp::frame::opcode::value::BINARY);
        return;
    }
    std::cout << "hwssId " << hwssIdStr << " none config for router "<< std::endl;
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
        auto wssIt = wssServerMap.begin();
        for(auto wssIt = wssServerMap.begin(); wssIt != wssServerMap.end(); wssIt++) {
            connect_tls(wssIt->first);
        }

        
        client->run();
    } catch (websocketpp::exception const & e) {
        std::cout << "wss router connect websocket exception " << e.what()
        << e.code() << std::endl;
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
        std::thread wssThread([]{
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
