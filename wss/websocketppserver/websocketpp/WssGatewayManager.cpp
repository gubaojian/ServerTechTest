//
//  WssGatewayManager.cpp
//  websocketpp
//
//  Created by efurture on 2025/5/24.
//

#include "WssGatewayManager.hpp"

namespace wsg {
    namespace router {
        void WssGatewayManager::runManager() {
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
            tlsClient = nullptr;
        }
    }
}
