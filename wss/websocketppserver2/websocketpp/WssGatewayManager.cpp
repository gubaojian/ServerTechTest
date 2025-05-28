//
//  WssGatewayManager.cpp
//  websocketpp
//
//  Created by efurture on 2025/5/24.
//

#include "WssGatewayManager.hpp"

namespace wsg {
    namespace bus {
        void WssGatewayManager::run() {
            clockTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
            std::shared_ptr<tls_client> client = std::make_shared<tls_client>();
            websocketpp::lib::shared_ptr<boost::asio::ssl::context> ctx;
            tlsClient = client;
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

                for(auto wssIt = wssServerMap.begin(); wssIt != wssServerMap.end(); wssIt++) {
                    wssConnMap[wssIt->first] = std::make_shared<wss_connection_info>();
                    connectGateway(wssIt->first);
                }
                
                pingGatewayTimer();
                
                clockTimeUpdateTimer();
                
                client->run();
            } catch (websocketpp::exception const & e) {
                std::cout << "WssGatewayManager connect websocket exception " << e.what()
                << e.code() << std::endl;
            } catch (std::exception const & e) {
                std::cout << "WssGatewayManager connect websocket exception " << e.what() << std::endl;
            }
            tlsClient = nullptr;
        }
        
       
    
        void WssGatewayManager::connectGateway(const std::string connWsgId) {
            auto findIt = wssServerMap.find(connWsgId);
            if (findIt == wssServerMap.end()) {
                return;
            }
            const WsgGateway& server = findIt->second;
            std::shared_ptr<tls_client> client = tlsClient;
            websocketpp::lib::error_code ec;
            tls_client::connection_ptr conn =  client->get_connection(server.url, ec);
            if (ec) {
                std::cout << "could not get_connection because: " << ec.message() << std::endl;
                this->tryConnectGatewayLater(connWsgId);
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
            conn->set_ping_handler([connWsgId](websocketpp::connection_hdl hdl, const std::string msg){
                std::cout << "WssGatewayManager ping handler " << connWsgId << std::endl;
                return true; //send pong response
            });
            
            conn->set_pong_timeout_handler([connWsgId](websocketpp::connection_hdl hdl, const std::string msg){
                std::cout << "WssGatewayManager wss ping timeout " << connWsgId << std::endl;
            });

            
            conn->set_pong_handler([connWsgId](websocketpp::connection_hdl hdl, std::string msg){
                std::cout << "WssGatewayManager pong handler " << connWsgId << std::endl;
            });
            
            conn->set_open_handler([this, conn, connWsgId](websocketpp::connection_hdl hdl){
                std::cout << "WssGatewayManager connect success " << connWsgId << "pid "  << std::this_thread::get_id()  << std::endl;
                 wssConnMap[connWsgId]->conn = conn;
                 flushMessageQueue(connWsgId);
            });
            
            conn->set_message_handler([this,  connWsgId](websocketpp::connection_hdl hdl, message_ptr msg) {
                //router current only handle json text
                if(msg->get_opcode() == websocketpp::frame::opcode::value::TEXT
                   || msg->get_opcode() == websocketpp::frame::opcode::value::BINARY) {
                    handleMessage(msg->get_payload(), connWsgId, true);
                }
            });
            
            conn->set_fail_handler([this, connWsgId](websocketpp::connection_hdl hdl) {
                std::cout << "WssGatewayManager failed wss connect try reconnect tls " << connWsgId << std::endl;
                auto it = wssConnMap.find(connWsgId);
                if (it != wssConnMap.end()) {
                    if (it->second->conn) {
                        it->second->conn = nullptr;
                    }
                }
                tryConnectGatewayLater(connWsgId);
               
            });
            conn->set_close_handler([this,connWsgId](websocketpp::connection_hdl hdl){
                //connect will be free in inner
                auto it = wssConnMap.find(connWsgId);
                if (it != wssConnMap.end()) {
                    if (it->second->conn) {
                        it->second->conn = nullptr;
                    }
                }
                tryConnectGatewayLater(connWsgId);
                std::cout << "WssGatewayManager close wss connect, try reconnect tls "  << connWsgId
                << " queue " << it->second->messageQueue->size()
                << " inMessage " << it->second->inMessageCount
                << " outMessage " << it->second->outMessageCount
                << " outRate " << it->second->outRate
                << " time " << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count() << std::endl;
                
            });
            client->connect(conn);

        }
    
    void WssGatewayManager::postMessage(const std::string &msg, const std::string &fromWsgId) {
            if(wssServerMap.empty()) {
                return;
            }
            std::shared_ptr<std::string> cpMsg = std::make_shared<std::string>(msg);
            std::shared_ptr<std::string> cpFromWsgId = std::make_shared<std::string>(fromWsgId);
            boost::asio::post(tlsClient->get_io_service(), [this, cpMsg, cpFromWsgId] {
                handleMessage(*cpMsg, *cpFromWsgId, false);
            });
        }
    
        void WssGatewayManager::handleMessage(const std::string &msg,
                                              const std::string& fromWsgId,
                                              const bool needTryRoute) {
            if(msg.empty()
               || msg.at(0) != '{') {
               return ;
            }
            simdjson::ondemand::document  doc;
            simdjson::padded_string paddedJson = simdjson::padded_string(msg);
            auto error = jsonParser.iterate(paddedJson).get(doc);
            if (error) {
                return;
            }
            auto wsgId = doc["wsgId"].get_string();
            if (wsgId.error()) {
                std::cout << fromWsgId << "send wrong format message" << std::endl;
                return;
            }
            
            std::string toWsgId(wsgId.value().data(), wsgId.value().size());
            if (toWsgId == fromWsgId) {
                return;
            }
            auto connIt = wssConnMap.find(toWsgId);
            if (connIt != wssConnMap.end()) {
                connIt->second->messageQueue->push(msg);
                connIt->second->inMessageCount++;
                flushMessageQueue(toWsgId);
                return;
            }
            //默认到ws协议的处理
            if (needTryRoute) {
                routeMessageToWsGatewayManager(msg,fromWsgId);
            }
        }
    
    void WssGatewayManager::flushMessageQueue(const std::string &toWsgId) {
        auto connIt = wssConnMap.find(toWsgId);
        if (connIt == wssConnMap.end()) {
            return;
        }
        
        //队列堆积过大，清理消息
        while (connIt->second->messageQueue->size() >= 256*1024) {
            connIt->second->messageQueue->pop();
        }
        
        //连接已断开，暂时不处理，等待连接后再发起处理。
        if (!connIt->second->conn) {
            return;
        }
        
        //连接未open，暂时不处理，等待连接后再发起处理。
        if (connIt->second->conn->get_state()
            != websocketpp::session::state::value::open) {
            return;
        }
        
        if (connIt->second->outRateClockTime != clockTime) {
            connIt->second->outRate = 0;
        }
        
        //wss限制速度为60MB/S，超过速度限制，延迟100ms后再发送。速度过快底层可能不稳定。
        //openssl加密解密速度也就100-200MB/S
        if(connIt->second->outRate >= 64*1024) {
            if (flushQueueTimer) {
                return;
            }
            std::shared_ptr<boost::asio::steady_timer> timer = std::make_shared<boost::asio::steady_timer>(tlsClient->get_io_service());
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
                std::cout << "WssGatewayManager wsgId wss " << toWsgId << " send error "<< send_error
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
            if (connIt->second->conn
                && connIt->second->conn->get_state() == websocketpp::session::state::value::open) {
                boost::asio::post(tlsClient->get_io_service(), [this, toWsgId] {
                    flushMessageQueue(toWsgId);
                });
            }
        }
    }
        
        void WssGatewayManager::pingGatewayTimer() {
            std::shared_ptr<boost::asio::steady_timer> timer = std::make_shared<boost::asio::steady_timer>(tlsClient->get_io_service());
            //router核心应用，ping保持连接,防止连接假死。可以多ping，保持连接。最好不超过30秒
            timer->expires_after(std::chrono::seconds(15 + std::rand()%15));
            timer->async_wait([this, timer] (const boost::system::error_code& error_code){
                if (error_code) {
                    std::cout << "WssGatewayManager ping_connect_tls error code " << error_code.message() << std::endl;
                    return ;
                }
                for(auto wssIt = wssConnMap.begin();
                    wssIt != wssConnMap.end(); wssIt++) {
                    if (wssIt->second->conn
                        && wssIt->second->conn->get_state() == websocketpp::session::state::value::open) {
                        websocketpp::lib::error_code ec;
                        wssIt->second->conn->ping("", ec);
                        std::cout << "WssGatewayManager ping_connect_tls keep alive "<< ec << wssIt->first
                        << " in " << wssIt->second->inMessageCount
                        << " out" << wssIt->second->outMessageCount
                        << " outRate " << wssIt->second->outRate
                        << " queue " << wssIt->second->messageQueue->size()  << std::endl;
                    }
                }
                pingGatewayTimer();
            });
        }
    
        void WssGatewayManager::clockTimeUpdateTimer() {
            std::shared_ptr<boost::asio::steady_timer> timer = std::make_shared<boost::asio::steady_timer>(tlsClient->get_io_service());
            timer->expires_after(std::chrono::milliseconds(1000));
            timer->async_wait([this, timer] (const boost::system::error_code& error_code){
                if (error_code) {
                    std::cout << "WssGatewayManager clock time update timer error code " << error_code.message() << std::endl;
                    return;
                }
                clockTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
                clockTimeUpdateTimer();
            });
        }
    
    
        void WssGatewayManager::tryConnectGatewayLater(const std::string wsgId) {
            std::shared_ptr<boost::asio::steady_timer> timer = std::make_shared<boost::asio::steady_timer>(tlsClient->get_io_service());
            //router数量不多，考虑网络抖动导致连接断开，尽快连接恢复即可。
            timer->expires_after(std::chrono::milliseconds(1000 + std::rand()%2000));
            timer->async_wait([this, timer, wsgId] (const boost::system::error_code& error_code){
                if (error_code) {
                    std::cout << wsgId << " reconnect error code " << error_code.message() << std::endl;
                    return ;
                }
                std::cout << "WssGatewayManager try reconnect timer fired " << wsgId  << std::endl;
                connectGateway(wsgId);
            });
        }
    }
}
