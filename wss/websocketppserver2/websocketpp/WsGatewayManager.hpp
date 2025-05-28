//
//  WsGatewayManager.hpp
//  websocketpp
//
//  Created by efurture on 2025/5/24.
//

#ifndef WsGatewayManager_hpp
#define WsGatewayManager_hpp

#include <stdio.h>
#include "WsgBusCommon.h"


/**
 * 处理ws协议的gateway连接
 */
namespace wsg {
    namespace bus {
        class WsGatewayManager {
            public:
              void run();
              void postMessage(const std::string &msg,
                               const std::string& fromWsgId);
            public:
                void setServerMap(std::unordered_map<std::string,WsgGateway> map) {
                    wsServerMap = map;
                }
            private:
                void handleMessage(const std::string& message,
                                   const std::string& fromWsgId,
                                   const bool needRoute);
                 void flushMessageQueue(const std::string& wsgId);
                 void pingGatewayTimer();
                 void clockTimeUpdateTimer();
                 void connectGateway(const std::string connWsgId);
                 void tryConnectGatewayLater(const std::string wsgId);
            private:
                std::unordered_map<std::string,WsgGateway> wsServerMap;
                std::shared_ptr<plain_client> plainClient;
                std::unordered_map<std::string, std::shared_ptr<ws_connection_info>> wsConnMap;
               simdjson::ondemand::parser jsonParser;
               std::shared_ptr<boost::asio::steady_timer> flushQueueTimer;
               int64_t clockTime;
        };
    }
}

#endif /* WsGatewayManager_hpp */
