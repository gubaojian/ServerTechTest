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
// This works around a bug in Boost <= 1.80.0 when using Clang >=18.
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

#include "simdjson/simdjson.h"

// core_client.hpp中修改：
// enable_multithreading should be set to false, 两处
// defalte should be disable
// 单独跑稳定，xcode中跑可能不稳定，有时会断开。

typedef websocketpp::client<websocketpp::config::asio_client> plain_client;
typedef websocketpp::client<websocketpp::config::asio_tls_client> tls_client;
typedef websocketpp::config::asio_client::message_type::ptr message_ptr;

struct HwssServer {
    std::string hwssId;
    std::string url;
};

struct plain_connection_info{
    plain_client::connection_ptr conn = nullptr;
    std::shared_ptr<std::queue<std::string>> messageQueue = std::make_shared<std::queue<std::string>>();
    int64_t inMessageCount = 0;
    int64_t outMessageCount = 0;
    int32_t outRate = 0;
};

struct tls_connection_info{
    tls_client::connection_ptr conn = nullptr;
    std::shared_ptr<std::queue<std::string>> messageQueue = std::make_shared<std::queue<std::string>>();
    int64_t inMessageCount = 0;
    int64_t outMessageCount = 0;
    int32_t outRate = 0;
};

struct ServerFinder {
    //ws协议hwss服务器id映射
    std::unordered_map<std::string,HwssServer> wsServerMap;
    //wss协议hwss服务器id映射
    std::unordered_map<std::string,HwssServer> wssServerMap;
    
    //ws协议asio的执行线程及相关信息：。仅在支持对应线程访问
    std::shared_ptr<plain_client> plainClient;
    std::unordered_map<std::string, std::shared_ptr<plain_connection_info>> wsConnMap;
    
    //wss协议asio的执行线程及相关信息：。仅在支持对应线程访问
    std::shared_ptr<tls_client> tlsClient;
    std::unordered_map<std::string, std::shared_ptr<tls_connection_info>> wssConnMap;
    
    //ws和wss两个线程分别对应两个parser。仅在支持对应线程访问
    simdjson::ondemand::parser plainParser;
    simdjson::ondemand::parser tlsParser;
    
};

std::shared_ptr<ServerFinder> serverFinder;

void try_connect_plain_later(const std::string hwssId);
void handleMsgFromWssRouter(std::shared_ptr<std::string> msg);
void flushToWsHwssId(const std::string& toHwssId);

void try_connect_tls_later(const std::string hwssId);
void handleMsgFromWsRouter(std::shared_ptr<std::string> msg);
void flushToWssHwssId(const std::string& toHwssId);

bool sendMessageToWsHwssId(const std::string& toHwssId, const std::string& playload) {
    auto connIt = serverFinder->wsConnMap.find(toHwssId);
    if (connIt == serverFinder->wsConnMap.end()) {
        return  false;
    }
    
    connIt->second->messageQueue->push(playload);
    connIt->second->inMessageCount++;
    
    //缓存个数，比wss协议大写，因为ws速度快
    while (connIt->second->messageQueue->size() >= 512*1024) {
        connIt->second->messageQueue->pop();
    }
    
    if (!connIt->second->conn) {
        return true;
    }
    
    if (connIt->second->conn->get_state() != websocketpp::session::state::value::open) {
        return true;
    }
    
    //一次batch提交个数
    for (int i=0; i<128; i++) {
        if (connIt->second->messageQueue->empty()) {
            break;
        }
        auto frontMessage = connIt->second->messageQueue->front();
        auto sendMessage = connIt->second->conn->get_message(websocketpp::frame::opcode::value::BINARY, frontMessage.size());
        sendMessage->set_payload(frontMessage);
        sendMessage->set_compressed(false); //disable compress
        websocketpp::lib::error_code send_error = connIt->second->conn->send(sendMessage);
        if (send_error) {
            std::cout << "hwssId ws " << toHwssId << " send error "<< send_error
             << " state "<< connIt->second->conn->get_state() << std::endl;
            break;
        } else {
            connIt->second->messageQueue->pop();
            connIt->second->outMessageCount++;
        }
    }
    
    //队列非空的话，再次发送
    if (!connIt->second->messageQueue->empty()) {
        if (connIt->second->conn->get_state() == websocketpp::session::state::value::open) {
            boost::asio::post(serverFinder->plainClient->get_io_service(), [toHwssId] {
                flushToWsHwssId(toHwssId);
            });
        }
    }
    
    return true;
}

void flushToWsHwssId(const std::string& toHwssId) {
    auto connIt = serverFinder->wsConnMap.find(toHwssId);
    if (connIt == serverFinder->wsConnMap.end()) {
        return;
    }
    //缓存过大时丢弃之前消息
    while (connIt->second->messageQueue->size() >= 512*1024) {
        connIt->second->messageQueue->pop();
    }
    
    if (!connIt->second->conn) {
        std::cout << "flushToWsHwssId wss conn empty" << std::endl;
        return;
    }
    
    if (connIt->second->conn->get_state() != websocketpp::session::state::value::open) {
        return;
    }
    if (connIt->second->messageQueue->empty()) {
        return;
    }
    
    std::cout << "flushToWsHwssId ws " << connIt->second->messageQueue->size() << std::endl;
    
    //batch提交会不会更好，降低流量压力
    for(int i=0; i<2048; i++) {
        if (connIt->second->messageQueue->empty()) {
            break;
        }
        auto frontMessage = connIt->second->messageQueue->front();
        auto sendMessage = connIt->second->conn->get_message(websocketpp::frame::opcode::value::BINARY, frontMessage.size());
        sendMessage->set_payload(frontMessage);
        sendMessage->set_compressed(false); //disable compress
        websocketpp::lib::error_code send_error = connIt->second->conn->send(sendMessage);
        if (send_error) {
            std::cout << "hwssId wss " << toHwssId << " send error "<< send_error
             << " state "<< connIt->second->conn->get_state() << std::endl;
            break;
        } else {
            connIt->second->messageQueue->pop();
            connIt->second->outMessageCount++;
            connIt->second->outRate++;
        }
    }
    //没发送完成，下个周期发送
    if (!connIt->second->messageQueue->empty()) {
        if (connIt->second->conn->get_state() == websocketpp::session::state::value::open) {
            boost::asio::post(serverFinder->plainClient->get_io_service(), [toHwssId] {
                flushToWsHwssId(toHwssId);
            });
        }
    }
}


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
    conn->set_socket_init_handler([](websocketpp::connection_hdl hdl, websocketpp::lib::asio::ip::tcp::socket& socket){        boost::asio::ip::tcp::no_delay no_delay(true);
        socket.set_option(no_delay);
        
        
        boost::asio::socket_base::keep_alive keep_alive(true);
        socket.set_option(keep_alive);
        
        boost::asio::socket_base::receive_buffer_size recv_size(65536);
        socket.set_option(recv_size);
        
        boost::asio::socket_base::send_buffer_size send_size(65536);
        socket.set_option(send_size);
        
    });
    conn->set_ping_handler([connHwssId](websocketpp::connection_hdl hdl, const std::string msg){
        std::cout << "router ping handler ws " << connHwssId << std::endl;
        return true; //send pong response
    });
    
    conn->set_pong_timeout_handler([connHwssId](websocketpp::connection_hdl hdl, const std::string msg){
        std::cout << "router ws ping timeout " << connHwssId << std::endl;
    });
    
    conn->set_open_handler([conn, connHwssId](websocketpp::connection_hdl hdl){
        std::cout << "router open handler success " << connHwssId << std::endl;
        serverFinder->wsConnMap[connHwssId]->conn = conn;
        flushToWsHwssId(connHwssId);
    });
    
    conn->set_message_handler([connHwssId](websocketpp::connection_hdl hdl, message_ptr msg) {
        if(msg->get_payload().empty()
           || msg->get_payload().at(0) != '{') { //only handle valid json
            return ;
        }
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
            bool hasEnterSendQueue = sendMessageToWsHwssId(toHwssId, msg->get_payload());
            if (hasEnterSendQueue) {
                return;
            }
           
            if (serverFinder->wssServerMap.empty()) {
                return;
            }
            //转发到wss协议的处理线程
            std::shared_ptr<tls_client> tlsClient = serverFinder->tlsClient;
            if (tlsClient) {
                std::shared_ptr<std::string> cpMsg = std::make_shared<std::string>(msg->get_payload());
                boost::asio::post(tlsClient->get_io_service(), [cpMsg] {
                    handleMsgFromWsRouter(cpMsg);
                });
            }
        }
        
    });
    
    conn->set_fail_handler([connHwssId](websocketpp::connection_hdl hdl) {
        //connect will be free in inner
        std::cout << "router failed ws connect try reconnect " << connHwssId << std::endl;
        auto it = serverFinder->wsConnMap.find(connHwssId);
        if (it != serverFinder->wsConnMap.end()) {
            if (it->second->conn) {
                it->second->conn = nullptr;
            }
        }
        try_connect_plain_later(connHwssId);
       
    });
    conn->set_close_handler([connHwssId](websocketpp::connection_hdl hdl){
        //connect will be free in inner
        std::cout << "router closed ws connect, try reconnect " << connHwssId << std::endl;
        auto it = serverFinder->wsConnMap.find(connHwssId);
        if (it != serverFinder->wsConnMap.end()) {
            if (it->second->conn) {
                it->second->conn = nullptr;
            }
        }
        try_connect_plain_later(connHwssId);
        
    });
    
    client->connect(conn);
}

void try_connect_plain_later(const std::string hwssId) {
    std::shared_ptr<boost::asio::steady_timer> timer = std::make_shared<boost::asio::steady_timer>(serverFinder->plainClient->get_io_service());
    timer->expires_after(std::chrono::milliseconds(1000 + std::rand()%2000));
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
    std::string toHwssId(hwssId.value().data(), hwssId.value().size());
    sendMessageToWsHwssId(toHwssId, *msg);
}

void ping_connect_plain_timer() {
    std::shared_ptr<boost::asio::steady_timer> timer = std::make_shared<boost::asio::steady_timer>(serverFinder->plainClient->get_io_service());
    //router核心应用，ping保持连接,防止连接假死。可以多ping，保持连接。最好不超过30秒
    timer->expires_after(std::chrono::seconds(15 + std::rand()%15));
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
                if (connIt->second->conn
                    && connIt->second->conn->get_state() == websocketpp::session::state::value::open) {
                    connIt->second->conn->ping("");
                    std::cout << "ping_connect_plain keep alive "<< hwssId << std::endl;
                }
            }
        }
        ping_connect_plain_timer();
    });
}



bool sendMessageToWssHwssId(const std::string& toHwssId,
                            const std::string& msg) {
    auto connIt = serverFinder->wssConnMap.find(toHwssId);
    if (connIt == serverFinder->wssConnMap.end()) {
        return false;
    }
    connIt->second->messageQueue->push(msg);
    connIt->second->inMessageCount++;
    
    //缓存过大时丢弃之前消息
    while (connIt->second->messageQueue->size() >= 256*1024) {
        connIt->second->messageQueue->pop();
        std::cout << "message be cropped " << std::endl;
    }
    
    if (!connIt->second->conn) {
        return true;
    }
    
    if (connIt->second->conn->get_state()
        != websocketpp::session::state::value::open) {
        return true;
    }
    
    //超过速度限制，延迟30ms后再发送
    if(connIt->second->outRate >= 20*1024) {
        std::shared_ptr<boost::asio::steady_timer> timer = std::make_shared<boost::asio::steady_timer>(serverFinder->tlsClient->get_io_service());
        //router核心应用，ping保持连接,防止连接假死。可以多ping，保持连接。最好不超过30秒
        timer->expires_after(std::chrono::milliseconds(100));
        timer->async_wait([timer, toHwssId] (const boost::system::error_code& error_code){
            if (error_code) {
                std::cout << "flushToWssHwssId timer error code " << error_code.message() << std::endl;
                return;
            }
            flushToWssHwssId(toHwssId);
        });
        return true;
    }
    
    //一次batch提交个数
    for(int i=0; i < 128; i++) {
        if (connIt->second->messageQueue->empty()){
            break;
        }
        auto frontMessage = connIt->second->messageQueue->front();
        auto sendMessage = connIt->second->conn->get_message(websocketpp::frame::opcode::value::BINARY, frontMessage.size());
        sendMessage->set_payload(frontMessage);
        sendMessage->set_compressed(false); //disable compress
        websocketpp::lib::error_code send_error = connIt->second->conn->send(sendMessage);
        if (send_error) {
            std::cout << "hwssId wss " << toHwssId << " send error "<< send_error
             << " state "<< connIt->second->conn->get_state() << std::endl;
            break;
        } else {
            connIt->second->messageQueue->pop();
            connIt->second->outMessageCount++;
            connIt->second->outRate++;
        }
    }
    
    //没有发送完成，下次继续调度发送。
    if (!connIt->second->messageQueue->empty()) {
        if (connIt->second->conn->get_state() == websocketpp::session::state::value::open) {
            boost::asio::post(serverFinder->tlsClient->get_io_service(), [toHwssId] {
                flushToWssHwssId(toHwssId);
            });
        }
    }
    
    return true;
}

void reset_limit_connect_tls_rate_timer() {
    std::shared_ptr<boost::asio::steady_timer> timer = std::make_shared<boost::asio::steady_timer>(serverFinder->tlsClient->get_io_service());
    //router核心应用，ping保持连接,防止连接假死。可以多ping，保持连接。最好不超过30秒
    timer->expires_after(std::chrono::milliseconds(500));
    timer->async_wait([timer] (const boost::system::error_code& error_code){
        if (error_code) {
            std::cout << "reset_limit_connect_tls_rate_timer error code " << error_code.message() << std::endl;
            return;
        }
        for(auto wssIt = serverFinder->wssConnMap.begin();
            wssIt != serverFinder->wssConnMap.end(); wssIt++) {
            std::cout << "reset_limit_connect_tls_rate_timer " << wssIt->first
            << " rate" << wssIt->second->outRate << std::endl;
            wssIt->second->outRate = 0;
        }
        reset_limit_connect_tls_rate_timer();
    });
}

void flushToWssHwssId(const std::string& toHwssId) {
    auto connIt = serverFinder->wssConnMap.find(toHwssId);
    if (connIt == serverFinder->wssConnMap.end()) {
        return;
    }
    
    if (!connIt->second->conn) {
        std::cout << "flushToWssHwssId wss conn empty" << std::endl;
        return;
    }
    if (connIt->second->messageQueue->empty()) {
        return;
    }
    
    if (connIt->second->conn->get_state() != websocketpp::session::state::value::open) {
        return;
    }
    
    std::cout << "flushToWssHwssId wss " << connIt->second->messageQueue->size()
     << "out" <<  connIt->second->outRate << std::endl;
    while (connIt->second->messageQueue->size() >= 256*1024) {
        connIt->second->messageQueue->pop();
    }
    
    //超过速度限制，延迟30ms后再发送
    if(connIt->second->outRate >= 20*1024) {
        std::shared_ptr<boost::asio::steady_timer> timer = std::make_shared<boost::asio::steady_timer>(serverFinder->tlsClient->get_io_service());
        //router核心应用，ping保持连接,防止连接假死。可以多ping，保持连接。最好不超过30秒
        timer->expires_after(std::chrono::milliseconds(500));
        timer->async_wait([timer, toHwssId] (const boost::system::error_code& error_code){
            if (error_code) {
                std::cout << "flushToWssHwssId timer error code " << error_code.message() << std::endl;
                return;
            }
            flushToWssHwssId(toHwssId);
        });
        return;
    }
    
    for(int i=0; i<1024; i++) {
        if (connIt->second->messageQueue->empty()){
            break;
        }
        auto frontMessage = connIt->second->messageQueue->front();
        auto sendMessage = connIt->second->conn->get_message(websocketpp::frame::opcode::value::BINARY, frontMessage.size());
        sendMessage->set_payload(frontMessage);
        sendMessage->set_compressed(false); //disable compress
        websocketpp::lib::error_code send_error = connIt->second->conn->send(sendMessage);
        if (send_error) {
            std::cout << "hwssId wss " << toHwssId << " send error "<< send_error
             << " state "<< connIt->second->conn->get_state() << std::endl;
            break;
        } else {
            connIt->second->messageQueue->pop();
            connIt->second->outMessageCount++;
            connIt->second->outRate++;
        }
    }
    
    if (!connIt->second->messageQueue->empty()) {
        if (connIt->second->conn->get_state() == websocketpp::session::state::value::open) {
            boost::asio::post(serverFinder->tlsClient->get_io_service(), [toHwssId] {
                flushToWssHwssId(toHwssId);
            });
        }
    }
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
    
    conn->set_socket_init_handler([](websocketpp::connection_hdl hdl, websocketpp::lib::asio::ssl::stream<
                                     websocketpp::lib::asio::ip::tcp::socket>& stream){
        websocketpp::lib::asio::ip::tcp::socket& socket = stream.next_layer();
        boost::asio::ip::tcp::no_delay no_delay(true);
        socket.set_option(no_delay);
        
        
        boost::asio::socket_base::keep_alive keep_alive(true);
        socket.set_option(keep_alive);
        
        boost::asio::socket_base::receive_buffer_size recv_size(65536);
        socket.set_option(recv_size);
        
        boost::asio::socket_base::send_buffer_size send_size(65536);
        socket.set_option(send_size);
        
    });
    conn->set_ping_handler([connHwssId](websocketpp::connection_hdl hdl, const std::string msg){
        std::cout << "router ping handler wss " << connHwssId << std::endl;
        return true; //send pong response
    });
    
    conn->set_pong_timeout_handler([connHwssId](websocketpp::connection_hdl hdl, const std::string msg){
        std::cout << "router wss ping timeout " << connHwssId << std::endl;
    });

    
    conn->set_pong_handler([connHwssId](websocketpp::connection_hdl hdl, std::string msg){
        std::cout << "router pong handler wss " << connHwssId << std::endl;
    });
    
    conn->set_open_handler([conn, connHwssId](websocketpp::connection_hdl hdl){
        std::cout << "router connect success wss " << connHwssId << "pid "  << std::this_thread::get_id()  << std::endl;
        serverFinder->wssConnMap[connHwssId]->conn = conn;
        flushToWssHwssId(connHwssId);
    });
    
    conn->set_message_handler([connHwssId](websocketpp::connection_hdl hdl, message_ptr msg) {
        if(msg->get_payload().empty()
           || msg->get_payload().at(0) != '{') {
            return ;
        }
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
            bool hasEnterQueue = sendMessageToWssHwssId(toHwssId, msg->get_payload());
            if (hasEnterQueue) {
                return;
            }
           
            if (serverFinder->wsServerMap.empty()) {
                return;
            }
            //转发到ws协议的处理线程
            std::shared_ptr<plain_client> plainClient = serverFinder->plainClient;
            if (plainClient) {
                std::shared_ptr<std::string> cpMsg = std::make_shared<std::string>(msg->get_payload());
                boost::asio::post(plainClient->get_io_service(), [cpMsg] {
                    handleMsgFromWssRouter(cpMsg);
                });
            }
        }
        
    });
    
    conn->set_fail_handler([connHwssId](websocketpp::connection_hdl hdl) {
        std::cout << "router failed wss connect try reconnect tls " << connHwssId << std::endl;
        auto it = serverFinder->wssConnMap.find(connHwssId);
        if (it != serverFinder->wssConnMap.end()) {
            if (it->second->conn) {
                it->second->conn = nullptr;
            }
        }
        try_connect_tls_later(connHwssId);
       
    });
    conn->set_close_handler([connHwssId](websocketpp::connection_hdl hdl){
        //connect will be free in inner
        auto it = serverFinder->wssConnMap.find(connHwssId);
        if (it != serverFinder->wssConnMap.end()) {
            if (it->second->conn) {
                it->second->conn = nullptr;
            }
        }
        try_connect_tls_later(connHwssId);
        std::cout << "router closed wss connect, try reconnect tls "  << connHwssId
        << " queue " << it->second->messageQueue->size()
        << " inMessage " << it->second->inMessageCount
        << " outMessage " << it->second->outMessageCount
        << " outRate " << it->second->outRate
        << " time " << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count() << std::endl;
        
    });
    client->connect(conn);
}


void try_connect_tls_later(const std::string hwssId) {
    std::shared_ptr<boost::asio::steady_timer> timer = std::make_shared<boost::asio::steady_timer>(serverFinder->tlsClient->get_io_service());
    //router数量不多，考虑网络抖动导致连接断开，尽快连接恢复即可。
    timer->expires_after(std::chrono::milliseconds(1000 + std::rand()%2000));
    timer->async_wait([timer, hwssId] (const boost::system::error_code& error_code){
        if (error_code) {
            std::cout << hwssId << " reconnect error code " << error_code.message() << std::endl;
            return ;
        }
        std::cout << "router try reconnect timer fired " << hwssId  << std::endl;
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
    std::string toHwssId(hwssId.value().data(), hwssId.value().size());
    sendMessageToWssHwssId(toHwssId, *msg);
}


void ping_connect_tls_timer() {
    std::shared_ptr<boost::asio::steady_timer> timer = std::make_shared<boost::asio::steady_timer>(serverFinder->tlsClient->get_io_service());
    //router核心应用，ping保持连接,防止连接假死。可以多ping，保持连接。最好不超过30秒
    timer->expires_after(std::chrono::seconds(15 + std::rand()%15));
    timer->async_wait([timer] (const boost::system::error_code& error_code){
        if (error_code) {
            std::cout << "ping_connect_tls error code " << error_code.message() << std::endl;
            return ;
        }
        for(auto wssIt = serverFinder->wssConnMap.begin();
            wssIt != serverFinder->wssConnMap.end(); wssIt++) {
            if (wssIt->second->conn
                && wssIt->second->conn->get_state() == websocketpp::session::state::value::open) {
                websocketpp::lib::error_code ec;
                wssIt->second->conn->ping("", ec);
                std::cout << "ping_connect_tls keep alive "<< ec << wssIt->first
                << " in " << wssIt->second->inMessageCount
                << " out" << wssIt->second->outMessageCount
                << " outRate " << wssIt->second->outRate
                << " queue " << wssIt->second->messageQueue->size()  << std::endl;
            }
        }
        ping_connect_tls_timer();
    });
}

void runWSRouter() {
    std::shared_ptr<plain_client> client = std::make_shared<plain_client>();
    serverFinder->plainClient = client;

    try {
        // Set logging to be pretty verbose (everything except message payloads)
        client->set_access_channels(websocketpp::log::alevel::none);
        client->clear_access_channels(websocketpp::log::alevel::none);
        client->set_error_channels(websocketpp::log::alevel::none);
        
        
        
        // Initialize ASIO
        client->init_asio();
        
        
        const auto& wsServerMap = serverFinder->wsServerMap;
        for(auto wsIt = wsServerMap.begin(); wsIt != wsServerMap.end(); wsIt++) {
            serverFinder->wsConnMap[wsIt->first] = std::make_shared<plain_connection_info>();
            connect_plain(wsIt->first);
        }
        
        ping_connect_plain_timer();
        
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
        client->set_error_channels(websocketpp::log::alevel::none);
        
        // Initialize ASIO
        client->init_asio();
        
        ctx = websocketpp::lib::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::tlsv13_client);
        ctx->set_options(boost::asio::ssl::context::default_workarounds |
                                 boost::asio::ssl::context::no_sslv2 |
                                 boost::asio::ssl::context::no_sslv3 |
                                 boost::asio::ssl::context::no_compression |
                         boost::asio::ssl::context::verify_peer |
                                 boost::asio::ssl::context::single_dh_use);
        ctx->set_default_verify_paths();
         client->set_tls_init_handler([=](websocketpp::connection_hdl hdl) {
             return ctx;
          });

        const auto& wssServerMap = serverFinder->wssServerMap;
        for(auto wssIt = wssServerMap.begin(); wssIt != wssServerMap.end(); wssIt++) {
            serverFinder->wssConnMap[wssIt->first] = std::make_shared<tls_connection_info>();
            connect_tls(wssIt->first);
        }
        
        ping_connect_tls_timer();
        
        reset_limit_connect_tls_rate_timer();
        
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
