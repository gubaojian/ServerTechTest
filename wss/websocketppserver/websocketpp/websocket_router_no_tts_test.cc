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
#include "WsgRouterCommon.h"


struct ServerFinder {
    //ws协议hwss服务器id映射
    std::unordered_map<std::string,WsgGateway> wsServerMap;
    //wss协议hwss服务器id映射
    std::unordered_map<std::string,WsgGateway> wssServerMap;
    
    //ws协议asio的执行线程及相关信息：。仅在支持对应线程访问
    std::shared_ptr<plain_client> plainClient;
    std::unordered_map<std::string, std::shared_ptr<ws_connection_info>> wsConnMap;
    
    //wss协议asio的执行线程及相关信息：。仅在支持对应线程访问
    std::shared_ptr<tls_client> tlsClient;
    std::unordered_map<std::string, std::shared_ptr<wss_connection_info>> wssConnMap;
    
    //ws和wss两个线程分别对应两个parser。仅在支持对应线程访问
    simdjson::ondemand::parser plainParser;
    simdjson::ondemand::parser tlsParser;
    std::shared_ptr<boost::asio::steady_timer> tlsFlushTimer;
    
};

std::shared_ptr<ServerFinder> serviceFinder;

void try_connect_plain_later(const std::string wsgId);
void handleMsgFromWssRouter(std::shared_ptr<std::string> msg);
void flushTowsWsgId(const std::string& toWsgId);

void try_connect_tls_later(const std::string wsgId);
void handleMsgFromWsRouter(std::shared_ptr<std::string> msg);
void flushToWssWsgId(const std::string& toWsgId);

bool sendMessageToWsWsgId(const std::string& toWsgId, const std::string& playload) {
    auto connIt = serviceFinder->wsConnMap.find(toWsgId);
    if (connIt == serviceFinder->wsConnMap.end()) {
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
            std::cout << "wsgId ws " << toWsgId << " send error "<< send_error
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
            boost::asio::post(serviceFinder->plainClient->get_io_service(), [toWsgId] {
                flushTowsWsgId(toWsgId);
            });
        }
    }
    
    return true;
}

void flushTowsWsgId(const std::string& toWsgId) {
    auto connIt = serviceFinder->wsConnMap.find(toWsgId);
    if (connIt == serviceFinder->wsConnMap.end()) {
        return;
    }
    //缓存过大时丢弃之前消息
    while (connIt->second->messageQueue->size() >= 512*1024) {
        connIt->second->messageQueue->pop();
    }
    
    if (!connIt->second->conn) {
        std::cout << "flushTowsWsgId wss conn empty" << std::endl;
        return;
    }
    
    if (connIt->second->conn->get_state() != websocketpp::session::state::value::open) {
        return;
    }
    if (connIt->second->messageQueue->empty()) {
        return;
    }
    
    std::cout << "flushTowsWsgId ws " << connIt->second->messageQueue->size() << std::endl;
    
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
            std::cout << "wsgId wss " << toWsgId << " send error "<< send_error
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
            boost::asio::post(serviceFinder->plainClient->get_io_service(), [toWsgId] {
                flushTowsWsgId(toWsgId);
            });
        }
    }
}


void connect_plain(const std::string connWsgId) {
    if (connWsgId.empty()) {
        std::cout << "illegal wsgId, is empty " << connWsgId << std::endl;
        return;
    }
    auto findIt = serviceFinder->wsServerMap.find(connWsgId);
    if (findIt == serviceFinder->wsServerMap.end()) {
        return;
    }
    const WsgGateway& server = findIt->second;
    std::shared_ptr<plain_client> client = serviceFinder->plainClient;
    websocketpp::lib::error_code ec;
    plain_client::connection_ptr conn =  client->get_connection(server.url, ec);
    if (ec) {
        std::cout << "could not get_connection because: " << ec.message() << std::endl;
        try_connect_plain_later(connWsgId);
        return;
    }
    conn->set_open_handshake_timeout(60*1000);
    conn->set_close_handshake_timeout(120*1000);
    conn->set_pong_timeout(60*1000);
    conn->set_socket_init_handler([](websocketpp::connection_hdl hdl, websocketpp::lib::asio::ip::tcp::socket& socket){        boost::asio::ip::tcp::no_delay no_delay(true);
        socket.set_option(no_delay);
        
        boost::asio::socket_base::keep_alive keep_alive(true);
        socket.set_option(keep_alive);
        
        boost::asio::socket_base::receive_buffer_size recv_size(65536);
        socket.set_option(recv_size);
        
        boost::asio::socket_base::send_buffer_size send_size(65536);
        socket.set_option(send_size);
        
    });
    conn->set_ping_handler([connWsgId](websocketpp::connection_hdl hdl, const std::string msg){
        std::cout << "router ping handler ws " << connWsgId << std::endl;
        return true; //send pong response
    });
    
    conn->set_pong_timeout_handler([connWsgId](websocketpp::connection_hdl hdl, const std::string msg){
        std::cout << "router ws ping timeout " << connWsgId << std::endl;
    });
    
    conn->set_open_handler([conn, connWsgId](websocketpp::connection_hdl hdl){
        std::cout << "router connect success " << connWsgId << std::endl;
        serviceFinder->wsConnMap[connWsgId]->conn = conn;
        flushTowsWsgId(connWsgId);
    });
    
    conn->set_message_handler([connWsgId](websocketpp::connection_hdl hdl, message_ptr msg) {
        if(msg->get_payload().empty()
           || msg->get_payload().at(0) != '{') { //only handle valid json
            return ;
        }
        //router current only handle json text
        if(msg->get_opcode() == websocketpp::frame::opcode::value::TEXT
           || msg->get_opcode() == websocketpp::frame::opcode::value::BINARY) {
            simdjson::ondemand::document  doc;
            simdjson::padded_string paddedJson = simdjson::padded_string(msg->get_payload());
            auto error = serviceFinder->plainParser.iterate(paddedJson).get(doc);
            if (error) {
                std::cout << connWsgId << " send wrong format message" << msg->get_payload() << std::endl;
                return;
            }
            auto wsgId = doc["wsgId"].get_string();
            if (wsgId.error()) {
                std::cout << connWsgId << " send wrong format message, missing wsgId" << std::endl;
                return;
            }
            //默认到ws协议的处理
            std::string toWsgId(wsgId.value().data(), wsgId.value().size());
            if (connWsgId == toWsgId) {
                return;
            }
            bool hasEnterSendQueue = sendMessageToWsWsgId(toWsgId, msg->get_payload());
            if (hasEnterSendQueue) {
                return;
            }
           
            if (serviceFinder->wssServerMap.empty()) {
                return;
            }
            //转发到wss协议的处理线程
            std::shared_ptr<tls_client> tlsClient = serviceFinder->tlsClient;
            if (tlsClient) {
                std::shared_ptr<std::string> cpMsg = std::make_shared<std::string>(msg->get_payload());
                boost::asio::post(tlsClient->get_io_service(), [cpMsg] {
                    handleMsgFromWsRouter(cpMsg);
                });
            }
        }
        
    });
    
    conn->set_fail_handler([connWsgId](websocketpp::connection_hdl hdl) {
        //connect will be free in inner
        std::cout << "router failed ws connect try reconnect " << connWsgId << std::endl;
        auto it = serviceFinder->wsConnMap.find(connWsgId);
        if (it != serviceFinder->wsConnMap.end()) {
            if (it->second->conn) {
                it->second->conn = nullptr;
            }
        }
        try_connect_plain_later(connWsgId);
       
    });
    conn->set_close_handler([connWsgId](websocketpp::connection_hdl hdl){
        //connect will be free in inner
        std::cout << "router closed ws connect, try reconnect " << connWsgId << std::endl;
        auto it = serviceFinder->wsConnMap.find(connWsgId);
        if (it != serviceFinder->wsConnMap.end()) {
            if (it->second->conn) {
                it->second->conn = nullptr;
            }
        }
        try_connect_plain_later(connWsgId);
        
    });
    
    client->connect(conn);
}

void try_connect_plain_later(const std::string wsgId) {
    std::shared_ptr<boost::asio::steady_timer> timer = std::make_shared<boost::asio::steady_timer>(serviceFinder->plainClient->get_io_service());
    timer->expires_after(std::chrono::milliseconds(1000 + std::rand()%2000));
    timer->async_wait([timer, wsgId] (const boost::system::error_code& error_code){
        if (error_code) {
            std::cout << wsgId << " reconnect error code " << error_code.message() << std::endl;
            return ;
        }
        connect_plain(wsgId);
    });
}

void handleMsgFromWssRouter(std::shared_ptr<std::string> msg) {
    if (!serviceFinder->plainClient) {
        return;
    }
    simdjson::ondemand::document  doc;
    simdjson::padded_string paddedJson = simdjson::padded_string(*msg);
    auto error = serviceFinder->plainParser.iterate(paddedJson).get(doc);
    if (error) {
        return;
    }
    auto wsgId = doc["wsgId"].get_string();
    if (wsgId.error()) {
        return;
    }
    std::string toWsgId(wsgId.value().data(), wsgId.value().size());
    sendMessageToWsWsgId(toWsgId, *msg);
}

void ping_connect_plain_timer() {
    std::shared_ptr<boost::asio::steady_timer> timer = std::make_shared<boost::asio::steady_timer>(serviceFinder->plainClient->get_io_service());
    //router核心应用，ping保持连接,防止连接假死。可以多ping，保持连接。最好不超过30秒
    timer->expires_after(std::chrono::seconds(15 + std::rand()%15));
    timer->async_wait([timer] (const boost::system::error_code& error_code){
        if (error_code) {
            std::cout << "ping_connect_tls error code " << error_code.message() << std::endl;
            return ;
        }
        for(auto wsIt = serviceFinder->wsServerMap.begin();
            wsIt != serviceFinder->wsServerMap.end(); wsIt++) {
           const auto& wsgId = wsIt->first;
            auto connIt = serviceFinder->wsConnMap.find(wsgId);
            if (connIt != serviceFinder->wsConnMap.end()) {
                if (connIt->second->conn
                    && connIt->second->conn->get_state() == websocketpp::session::state::value::open) {
                    connIt->second->conn->ping("");
                    std::cout << "ping_connect_plain keep alive "<< wsgId << std::endl;
                }
            }
        }
        ping_connect_plain_timer();
    });
}



bool sendMessageToWssWsgId(const std::string& toWsgId,
                            const std::string& msg) {
    auto connIt = serviceFinder->wssConnMap.find(toWsgId);
    if (connIt == serviceFinder->wssConnMap.end()) {
        return false;
    }
    connIt->second->messageQueue->push(msg);
    connIt->second->inMessageCount++;
    
    //缓存过大时丢弃之前消息
    while (connIt->second->messageQueue->size() >= 256*1024) {
        connIt->second->messageQueue->pop();
    }
    
    if (!connIt->second->conn) {
        return true;
    }
    
    if (connIt->second->conn->get_state()
        != websocketpp::session::state::value::open) {
        return true;
    }
    
    //超过速度限制，延迟30ms后再发送
    if(connIt->second->outRate >= 64*1024) {
        if (serviceFinder->tlsFlushTimer) {
            return true;
        }
        std::shared_ptr<boost::asio::steady_timer> timer = std::make_shared<boost::asio::steady_timer>(serviceFinder->tlsClient->get_io_service());
        //router核心应用，ping保持连接,防止连接假死。可以多ping，保持连接。最好不超过30秒
        timer->expires_after(std::chrono::milliseconds(100));
        timer->async_wait([timer, toWsgId] (const boost::system::error_code& error_code){
            if (error_code) {
                std::cout << "flushToWssWsgId timer error code " << error_code.message() << std::endl;
                return;
            }
            flushToWssWsgId(toWsgId);
            serviceFinder->tlsFlushTimer = nullptr;
        });
        serviceFinder->tlsFlushTimer = timer;
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
            std::cout << "wsgId wss " << toWsgId << " send error "<< send_error
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
            boost::asio::post(serviceFinder->tlsClient->get_io_service(), [toWsgId] {
                flushToWssWsgId(toWsgId);
            });
        }
    }
    
    return true;
}

void reset_limit_connect_tls_rate_timer() {
    std::shared_ptr<boost::asio::steady_timer> timer = std::make_shared<boost::asio::steady_timer>(serviceFinder->tlsClient->get_io_service());
    //router核心应用，ping保持连接,防止连接假死。可以多ping，保持连接。最好不超过30秒
    timer->expires_after(std::chrono::milliseconds(500));
    timer->async_wait([timer] (const boost::system::error_code& error_code){
        if (error_code) {
            std::cout << "reset_limit_connect_tls_rate_timer error code " << error_code.message() << std::endl;
            return;
        }
        for(auto wssIt = serviceFinder->wssConnMap.begin();
            wssIt != serviceFinder->wssConnMap.end(); wssIt++) {
            std::cout << "reset_limit_connect_tls_rate_timer " << wssIt->first
            << " current rate" << wssIt->second->outRate << std::endl;
            wssIt->second->outRate = 0;
        }
        reset_limit_connect_tls_rate_timer();
    });
}

void flushToWssWsgId(const std::string& toWsgId) {
    auto connIt = serviceFinder->wssConnMap.find(toWsgId);
    if (connIt == serviceFinder->wssConnMap.end()) {
        return;
    }
    
    if (!connIt->second->conn) {
        std::cout << "flushToWssWsgId wss conn empty" << std::endl;
        return;
    }
    if (connIt->second->messageQueue->empty()) {
        return;
    }
    
    if (connIt->second->conn->get_state() != websocketpp::session::state::value::open) {
        return;
    }
    
    std::cout << "flushToWssWsgId wss queue " << connIt->second->messageQueue->size()
     << " out " <<  connIt->second->outRate << std::endl;
    while (connIt->second->messageQueue->size() >= 256*1024) {
        connIt->second->messageQueue->pop();
    }
    
    //超过速度限制，延迟30ms后再发送
    if(connIt->second->outRate >= 64*1024) {
        if (serviceFinder->tlsFlushTimer) {
            return;
        }
        std::shared_ptr<boost::asio::steady_timer> timer = std::make_shared<boost::asio::steady_timer>(serviceFinder->tlsClient->get_io_service());
        //router核心应用，ping保持连接,防止连接假死。可以多ping，保持连接。最好不超过30秒
        timer->expires_after(std::chrono::milliseconds(500));
        timer->async_wait([timer, toWsgId] (const boost::system::error_code& error_code){
            if (error_code) {
                std::cout << "flushToWssWsgId timer error code " << error_code.message() << std::endl;
                return;
            }
            flushToWssWsgId(toWsgId);
            serviceFinder->tlsFlushTimer = nullptr;
        });
        serviceFinder->tlsFlushTimer = timer;
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
            std::cout << "wsgId wss " << toWsgId << " send error "<< send_error
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
            boost::asio::post(serviceFinder->tlsClient->get_io_service(), [toWsgId] {
                flushToWssWsgId(toWsgId);
            });
        }
    }
}


void connect_tls(const std::string connwsgId) {
    auto findIt = serviceFinder->wssServerMap.find(connwsgId);
    if (findIt == serviceFinder->wssServerMap.end()) {
        return;
    }
    const WsgGateway& server = findIt->second;
    std::shared_ptr<tls_client> client = serviceFinder->tlsClient;
    websocketpp::lib::error_code ec;
    tls_client::connection_ptr conn =  client->get_connection(server.url, ec);
    if (ec) {
        std::cout << "could not get_connection because: " << ec.message() << std::endl;
        try_connect_tls_later(connwsgId);
        return;
    }
    conn->set_open_handshake_timeout(60*1000);
    conn->set_close_handshake_timeout(120*1000);
    conn->set_pong_timeout(60*1000);
    
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
    conn->set_ping_handler([connwsgId](websocketpp::connection_hdl hdl, const std::string msg){
        std::cout << "router ping handler wss " << connwsgId << std::endl;
        return true; //send pong response
    });
    
    conn->set_pong_timeout_handler([connwsgId](websocketpp::connection_hdl hdl, const std::string msg){
        std::cout << "router wss ping timeout " << connwsgId << std::endl;
    });

    
    conn->set_pong_handler([connwsgId](websocketpp::connection_hdl hdl, std::string msg){
        std::cout << "router pong handler wss " << connwsgId << std::endl;
    });
    
    conn->set_open_handler([conn, connwsgId](websocketpp::connection_hdl hdl){
        std::cout << "router connect success wss " << connwsgId << "pid "  << std::this_thread::get_id()  << std::endl;
        serviceFinder->wssConnMap[connwsgId]->conn = conn;
        flushToWssWsgId(connwsgId);
    });
    
    conn->set_message_handler([connwsgId](websocketpp::connection_hdl hdl, message_ptr msg) {
        if(msg->get_payload().empty()
           || msg->get_payload().at(0) != '{') {
            return ;
        }
        //router current only handle json text
        if(msg->get_opcode() == websocketpp::frame::opcode::value::TEXT
           || msg->get_opcode() == websocketpp::frame::opcode::value::BINARY) {
            simdjson::ondemand::document  doc;
            simdjson::padded_string paddedJson = simdjson::padded_string(msg->get_payload());
            auto error = serviceFinder->tlsParser.iterate(paddedJson).get(doc);
            if (error) {
                return;
            }
            auto wsgId = doc["wsgId"].get_string();
            if (wsgId.error()) {
                std::cout << connwsgId << " send wrong format message" << std::endl;
                return;
            }
            //默认到ws协议的处理
            std::string toWsgId(wsgId.value().data(), wsgId.value().size());
            if (toWsgId == connwsgId) {
                return;
            }
            bool hasEnterQueue = sendMessageToWssWsgId(toWsgId, msg->get_payload());
            if (hasEnterQueue) {
                return;
            }
           
            if (serviceFinder->wsServerMap.empty()) {
                return;
            }
            //转发到ws协议的处理线程
            std::shared_ptr<plain_client> plainClient = serviceFinder->plainClient;
            if (plainClient) {
                std::shared_ptr<std::string> cpMsg = std::make_shared<std::string>(msg->get_payload());
                boost::asio::post(plainClient->get_io_service(), [cpMsg] {
                    handleMsgFromWssRouter(cpMsg);
                });
            }
        }
        
    });
    
    conn->set_fail_handler([connwsgId](websocketpp::connection_hdl hdl) {
        std::cout << "router failed wss connect try reconnect tls " << connwsgId << std::endl;
        auto it = serviceFinder->wssConnMap.find(connwsgId);
        if (it != serviceFinder->wssConnMap.end()) {
            if (it->second->conn) {
                it->second->conn = nullptr;
            }
        }
        try_connect_tls_later(connwsgId);
       
    });
    conn->set_close_handler([connwsgId](websocketpp::connection_hdl hdl){
        //connect will be free in inner
        auto it = serviceFinder->wssConnMap.find(connwsgId);
        if (it != serviceFinder->wssConnMap.end()) {
            if (it->second->conn) {
                it->second->conn = nullptr;
            }
        }
        try_connect_tls_later(connwsgId);
        std::cout << "router closed wss connect, try reconnect tls "  << connwsgId
        << " queue " << it->second->messageQueue->size()
        << " inMessage " << it->second->inMessageCount
        << " outMessage " << it->second->outMessageCount
        << " outRate " << it->second->outRate
        << " time " << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count() << std::endl;
        
    });
    client->connect(conn);
}


void try_connect_tls_later(const std::string wsgId) {
    std::shared_ptr<boost::asio::steady_timer> timer = std::make_shared<boost::asio::steady_timer>(serviceFinder->tlsClient->get_io_service());
    //router数量不多，考虑网络抖动导致连接断开，尽快连接恢复即可。
    timer->expires_after(std::chrono::milliseconds(1000 + std::rand()%2000));
    timer->async_wait([timer, wsgId] (const boost::system::error_code& error_code){
        if (error_code) {
            std::cout << wsgId << " reconnect error code " << error_code.message() << std::endl;
            return ;
        }
        std::cout << "router try reconnect timer fired " << wsgId  << std::endl;
        connect_tls(wsgId);
    });
}

void handleMsgFromWsRouter(std::shared_ptr<std::string> msg) {
    if (!serviceFinder->tlsClient) {
        return;
    }
    simdjson::ondemand::document  doc;
    simdjson::padded_string paddedJson = simdjson::padded_string(*msg);
    auto error = serviceFinder->tlsParser.iterate(paddedJson).get(doc);
    if (error) {
        return;
    }
    auto wsgId = doc["wsgId"].get_string();
    if (wsgId.error()) {
        return;
    }
    std::string toWsgId(wsgId.value().data(), wsgId.value().size());
    sendMessageToWssWsgId(toWsgId, *msg);
}


void ping_connect_tls_timer() {
    std::shared_ptr<boost::asio::steady_timer> timer = std::make_shared<boost::asio::steady_timer>(serviceFinder->tlsClient->get_io_service());
    //router核心应用，ping保持连接,防止连接假死。可以多ping，保持连接。最好不超过30秒
    timer->expires_after(std::chrono::seconds(15 + std::rand()%15));
    timer->async_wait([timer] (const boost::system::error_code& error_code){
        if (error_code) {
            std::cout << "ping_connect_tls error code " << error_code.message() << std::endl;
            return ;
        }
        for(auto wssIt = serviceFinder->wssConnMap.begin();
            wssIt != serviceFinder->wssConnMap.end(); wssIt++) {
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
    serviceFinder->plainClient = client;

    try {
        // Set logging to be pretty verbose (everything except message payloads)
        client->set_access_channels(websocketpp::log::alevel::none);
        client->clear_access_channels(websocketpp::log::alevel::none);
        client->set_error_channels(websocketpp::log::alevel::none);
        
        
        // Initialize ASIO
        client->init_asio();
        
        
        const auto& wsServerMap = serviceFinder->wsServerMap;
        for(auto wsIt = wsServerMap.begin(); wsIt != wsServerMap.end(); wsIt++) {
            serviceFinder->wsConnMap[wsIt->first] = std::make_shared<ws_connection_info>();
            connect_plain(wsIt->first);
        }
        
        ping_connect_plain_timer();
        
        client->run();
    } catch (websocketpp::exception const & e) {
        std::cout << "router connect websocket exception " << e.what()
        << e.code() << std::endl;
    }
    serviceFinder->plainClient = nullptr;
    
}

void runWSSRouter() {
    std::shared_ptr<tls_client> client = std::make_shared<tls_client>();
    websocketpp::lib::shared_ptr<boost::asio::ssl::context> ctx;
    serviceFinder->tlsClient = client;
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

        const auto& wssServerMap = serviceFinder->wssServerMap;
        for(auto wssIt = wssServerMap.begin(); wssIt != wssServerMap.end(); wssIt++) {
            serviceFinder->wssConnMap[wssIt->first] = std::make_shared<wss_connection_info>();
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
    serviceFinder->tlsClient = nullptr;
}

int websocket_router_no_tts_test_main(int argc, const char * argv[]) {
    serviceFinder = std::make_shared<ServerFinder>();
    
    
    {
        
        WsgGateway wsServer2;
        wsServer2.url = "ws://127.0.0.1:9001/wsg?role=router&routerAppId=162374203942&routerAppToken=ZnPMZ3Wv8GR5ofP3LdNYHFIT3eNGcApg";
        wsServer2.wsgId = "ws_685208380980";
        std::string wsWsgId2 = "ws_685208380980";
        serviceFinder->wsServerMap[wsWsgId2] = wsServer2;
    }
 
    {
        
        WsgGateway wsServer2;
        wsServer2.url = "ws://127.0.0.1:9002/wsg?role=router&routerAppId=162374203942&routerAppToken=ZnPMZ3Wv8GR5ofP3LdNYHFIT3eNGcApg";
        wsServer2.wsgId = "ws_685208380981";
        std::string wsWsgId2 = "ws_685208380981";
        serviceFinder->wsServerMap[wsWsgId2] = wsServer2;
    }
    
    {
        
        WsgGateway wsServer;
        wsServer.url = "wss://127.0.0.1:8001/wsg?role=router&routerAppId=162374203942&routerAppToken=ZnPMZ3Wv8GR5ofP3LdNYHFIT3eNGcApg";
        wsServer.wsgId = "wss_685208380980";
        std::string wsWsgId = "wss_685208380980";
        serviceFinder->wssServerMap[wsWsgId] = wsServer;
    }
    
    {
        WsgGateway wsServer;
        wsServer.url = "wss://127.0.0.1:8002/wsg?role=router&routerAppId=162374203942&routerAppToken=ZnPMZ3Wv8GR5ofP3LdNYHFIT3eNGcApg";
        wsServer.wsgId = "wss_685208380981";
        std::string wsWsgId = "wss_685208380981";
        serviceFinder->wssServerMap[wsWsgId] = wsServer;
    }
    
    
    if(serviceFinder->wsServerMap.empty() && serviceFinder->wssServerMap.empty()) {
        std::cout << "non config hwss servers for router " << std::endl;
        return 0;
    }
    if (!serviceFinder->wsServerMap.empty() && !serviceFinder->wssServerMap.empty()) {
        std::thread wssThread([] {
            runWSSRouter();
        });
        runWSRouter();
        wssThread.join();
        return 0;
    }
    
    if (!serviceFinder->wsServerMap.empty()) {
        runWSRouter();
        return 0;
    }
    
    if (!serviceFinder->wssServerMap.empty()) {
        runWSSRouter();
        return 0;
    }
    
    return 0;
}
