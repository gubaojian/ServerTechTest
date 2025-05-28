//
//  WssGatewayManager.hpp
//  websocketpp
//
//  Created by efurture on 2025/5/24.
//

#ifndef WssGatewayManager_hpp
#define WssGatewayManager_hpp
#include <stdio.h>
#include "WsgBusCommon.h"

/**
 * 处理wss协议的gateway连接
 */
namespace wsg {
    namespace bus {
        class WssGatewayManager {
          public:
            void run();
            void postMessage(const std::string &msg,
                             const std::string& fromWsgId);
        public:
            void setServerMap(std::unordered_map<std::string,WsgGateway> map) {
                wssServerMap = map;
            }
          private:
            void handleMessage(const std::string& message,
                               const std::string& fromWsgId,
                               const bool needTryRoute);
            void connectGateway(const std::string connWsgId);
            void pingGatewayTimer();
            void clockTimeUpdateTimer();
            void tryConnectGatewayLater(const std::string wsgId);
            void flushMessageQueue(const std::string& wsgId);
          private:
            std::unordered_map<std::string,WsgGateway> wssServerMap;
            std::shared_ptr<tls_client> tlsClient;
            std::unordered_map<std::string, std::shared_ptr<wss_connection_info>> wssConnMap;
            simdjson::ondemand::parser jsonParser;
            std::shared_ptr<boost::asio::steady_timer> flushQueueTimer;
            int64_t clockTime;
            
        };
    }
}

#endif /* WssGatewayManager_hpp */
