//
//  WsGatewayManager.cpp
//  websocketpp
//
//  Created by efurture on 2025/5/24.
//

#include "WsGatewayManager.hpp"


namespace wsg {
    namespace bus {
        void WsGatewayManager::run() {
            clockTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
            std::shared_ptr<plain_client> client = std::make_shared<plain_client>();
            plainClient = client;
            try {
                // Set logging to be pretty verbose (everything except message payloads)
                client->set_access_channels(websocketpp::log::alevel::none);
                client->clear_access_channels(websocketpp::log::alevel::none);
                client->set_error_channels(websocketpp::log::alevel::none);
                
                
                // Initialize ASIO
                client->init_asio();
               
                for(auto wsIt = wsServerMap.begin(); wsIt != wsServerMap.end(); wsIt++) {
                    wsConnMap[wsIt->first] = std::make_shared<ws_connection_info>();
                    connectGateway(wsIt->first);
                }
                
                pingGatewayTimer();
                
                clockTimeUpdateTimer();
                
                client->run();
            } catch (websocketpp::exception const & e) {
                std::cout << "WsGatewayManager connect websocket exception " << e.what()
                << e.code() << std::endl;
            }
            plainClient = nullptr;
            
        }
        
        void  WsGatewayManager::connectGateway(const std::string connWsgId) {
            if (connWsgId.empty()) {
                std::cout << "WsGatewayManager illegal wsgId, is empty " << connWsgId << std::endl;
                return;
            }
            auto findIt = wsServerMap.find(connWsgId);
            if (findIt == wsServerMap.end()) {
                return;
            }
            const WsgGateway& server = findIt->second;
            std::shared_ptr<plain_client> client = plainClient;
            websocketpp::lib::error_code ec;
            plain_client::connection_ptr conn =  client->get_connection(server.url, ec);
            if (ec) {
                std::cout << "WsGatewayManager could not get_connection because: " << ec.message() << std::endl;
                tryConnectGatewayLater(connWsgId);
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
                std::cout << "WsGatewayManager ping handler ws " << connWsgId << std::endl;
                return true; //send pong response
            });
            
            conn->set_pong_timeout_handler([connWsgId](websocketpp::connection_hdl hdl, const std::string msg){
                std::cout << "WsGatewayManager ws ping timeout " << connWsgId << std::endl;
            });
            
            conn->set_open_handler([this, conn, connWsgId](websocketpp::connection_hdl hdl){
                std::cout << "WsGatewayManager connect success " << connWsgId << std::endl;
                wsConnMap[connWsgId]->conn = conn;
                flushMessageQueue(connWsgId);
            });
            
            conn->set_message_handler([this, connWsgId](websocketpp::connection_hdl hdl, message_ptr msg) {
                //router current only handle json text
                if(msg->get_opcode() == websocketpp::frame::opcode::value::TEXT
                   || msg->get_opcode() == websocketpp::frame::opcode::value::BINARY) {
                    handleMessage(msg->get_payload(), connWsgId, true);
                }
                
            });
            
            conn->set_fail_handler([this, connWsgId](websocketpp::connection_hdl hdl) {
                //connect will be free in inner
                std::cout << "WsGatewayManager failed ws connect try reconnect " << connWsgId << std::endl;
                auto it = wsConnMap.find(connWsgId);
                if (it != wsConnMap.end()) {
                    if (it->second->conn) {
                        it->second->conn = nullptr;
                    }
                }
                tryConnectGatewayLater(connWsgId);
               
            });
            conn->set_close_handler([this, connWsgId](websocketpp::connection_hdl hdl){
                //connect will be free in inner
                std::cout << "WsGatewayManager closed ws connect, try reconnect " << connWsgId << std::endl;
                auto it = wsConnMap.find(connWsgId);
                if (it != wsConnMap.end()) {
                    if (it->second->conn) {
                        it->second->conn = nullptr;
                    }
                }
                tryConnectGatewayLater(connWsgId);
                
            });
            
            client->connect(conn);
        }
    
    void WsGatewayManager::postMessage(const std::string &msg, const std::string &fromWsgId) {
        if (wsServerMap.empty()) {
            return;
        }
        std::shared_ptr<std::string> cpMsg = std::make_shared<std::string>(msg);
        std::shared_ptr<std::string> cpFromWsgId = std::make_shared<std::string>(fromWsgId);
        boost::asio::post(plainClient->get_io_service(), [this, cpMsg, cpFromWsgId] {
            handleMessage(*cpMsg, *cpFromWsgId, false);
        });
    }
       
        void WsGatewayManager::handleMessage(const std::string &msg, const std::string &fromWsgId, const bool needRoute) {
            if(msg.empty()
               || msg.at(0) != '{') { //only handle valid json
                return ;
            }
            simdjson::ondemand::document  doc;
            simdjson::padded_string paddedJson = simdjson::padded_string(msg);
            auto error = jsonParser.iterate(paddedJson).get(doc);
            if (error) {
                std::cout << fromWsgId << " send wrong format message" << std::endl;
                return;
            }
            auto wsgId = doc["wsgId"].get_string();
            if (wsgId.error()) {
                std::cout << fromWsgId << " send wrong format message, missing wsgId" << std::endl;
                return;
            }
            
            std::string toWsgId(wsgId.value().data(), wsgId.value().size());
            if (fromWsgId == toWsgId) {
                return;
            }
            
            auto connIt = wsConnMap.find(toWsgId);
            if (connIt != wsConnMap.end()) {
                connIt->second->messageQueue->push(msg);
                connIt->second->inMessageCount++;
                flushMessageQueue(toWsgId);
                return;
            }
            if (needRoute) {
                routeMessageToWssGatewayManager(msg, fromWsgId);
            }
        }
     
    
        void WsGatewayManager::flushMessageQueue(const std::string &toWsgId) {
            auto connIt = wsConnMap.find(toWsgId);
            if (connIt == wsConnMap.end()) {
                return;
            }
            
            //ws速度快，缓存比wss协议更大些。
            while (connIt->second->messageQueue->size() >= 512*1024) {
                connIt->second->messageQueue->pop();
            }
            
            if (!connIt->second->conn) {
                return;
            }
            
            if (connIt->second->conn->get_state() != websocketpp::session::state::value::open) {
                return;
            }
            
            if (connIt->second->outRateClockTime != clockTime) {
                connIt->second->outRate = 0;
            }
            
            //ws单连接限制速度为120MB/S，超过速度限制，延迟100ms后再发送
            //可以根据需要进行调整，其实可以更大些。
            if(connIt->second->outRate >= 128*1024) {
                if (flushQueueTimer) {
                    return;
                }
                std::shared_ptr<boost::asio::steady_timer> timer = std::make_shared<boost::asio::steady_timer>(plainClient->get_io_service());
                timer->expires_after(std::chrono::milliseconds(100));
                timer->async_wait([this, timer, toWsgId] (const boost::system::error_code& error_code){
                    if (error_code) {
                        std::cout << "WssGatewayManager flushMessageQueue timer error code " << error_code.message() << std::endl;
                        return;
                    }
                    flushMessageQueue(toWsgId);
                    flushQueueTimer = nullptr;
                });
                flushQueueTimer = timer;
                return;
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
                    std::cout << "WsGatewayManager wsgId " << toWsgId << " send error "<< send_error
                     << " state "<< connIt->second->conn->get_state() << std::endl;
                    break;
                } else {
                    connIt->second->messageQueue->pop();
                    connIt->second->outMessageCount++;
                    connIt->second->outRate++;
                }
            }
            
            //队列非空的话，再次发送
            if (!connIt->second->messageQueue->empty()) {
                if (connIt->second->conn->get_state() == websocketpp::session::state::value::open) {
                    boost::asio::post(plainClient->get_io_service(), [this, toWsgId] {
                        flushMessageQueue(toWsgId);
                    });
                }
            }
        }
    
        void WsGatewayManager::pingGatewayTimer() {
            std::shared_ptr<boost::asio::steady_timer> timer = std::make_shared<boost::asio::steady_timer>(plainClient->get_io_service());
            //router核心应用，ping保持连接,防止连接假死。可以多ping，保持连接。最好不超过30秒
            timer->expires_after(std::chrono::seconds(15 + std::rand()%15));
            timer->async_wait([this, timer] (const boost::system::error_code& error_code){
                if (error_code) {
                    std::cout << "WsGatewayManager ping_connect_tls error code " << error_code.message() << std::endl;
                    return ;
                }
                for(auto wsIt = wsServerMap.begin();
                    wsIt != wsServerMap.end(); wsIt++) {
                   const auto& wsgId = wsIt->first;
                    auto connIt = wsConnMap.find(wsgId);
                    if (connIt != wsConnMap.end()) {
                        if (connIt->second->conn
                            && connIt->second->conn->get_state() == websocketpp::session::state::value::open) {
                            connIt->second->conn->ping("");
                            std::cout << "WsGatewayManager ping_connect_plain keep alive "<< wsgId << std::endl;
                        }
                    }
                }
                pingGatewayTimer();
            });
        }
    
        void WsGatewayManager::clockTimeUpdateTimer() {
            std::shared_ptr<boost::asio::steady_timer> timer = std::make_shared<boost::asio::steady_timer>(plainClient->get_io_service());
            timer->expires_after(std::chrono::milliseconds(1000));
            timer->async_wait([this, timer] (const boost::system::error_code& error_code){
                if (error_code) {
                    std::cout << "WsGatewayManager clock time update timer error code " << error_code.message() << std::endl;
                    return;
                }
                clockTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
                clockTimeUpdateTimer();
            });
        }
    
        void WsGatewayManager::tryConnectGatewayLater(const std::string wsgId) {
            std::shared_ptr<boost::asio::steady_timer> timer = std::make_shared<boost::asio::steady_timer>(plainClient->get_io_service());
            timer->expires_after(std::chrono::milliseconds(1000 + std::rand()%2000));
            timer->async_wait([this, timer, wsgId] (const boost::system::error_code& error_code){
                if (error_code) {
                    std::cout << wsgId << " WsGatewayManager reconnect error code " << error_code.message() << std::endl;
                    return ;
                }
                connectGateway(wsgId);
            });
        }
        
    }
}
