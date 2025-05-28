//
//  WssGatewayManager.hpp
//  websocketpp
//
//  Created by efurture on 2025/5/24.
//

#ifndef WssGatewayManager_hpp
#define WssGatewayManager_hpp
#include <stdio.h>
#include "WsgRouterCommon.h"

namespace wsg {
    namespace router {
        class WssGatewayManager {
          public:
            void runManager();
            void postMessage(std::shared_ptr<std::string> msg);
        public:
            void setWssServerMap(std::unordered_map<std::string,WsgGateway> _wssServerMap) {
                wssServerMap = _wssServerMap;
            }
          private:
            void handleMessage(const std::string& message);
            void connect_tls(const std::string connwsgId);
          private:
            std::unordered_map<std::string,WsgGateway> wssServerMap;
            std::shared_ptr<tls_client> tlsClient;
            std::unordered_map<std::string, std::shared_ptr<wss_connection_info>> wssConnMap;
            simdjson::ondemand::parser tlsParser;
            
        };
    }
}

#endif /* WssGatewayManager_hpp */
