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
#include "WsgBusCommon.h"
#include "WsGatewayManager.hpp"
#include "WssGatewayManager.hpp"

/**
 
 IF(APPLE)
   ADD_COMPILE_FLAG("-D_GNU_SOURCE")
 endif()
 
 */
#ifndef _GNU_SOURCE
#   define _GNU_SOURCE
#endif

#define BOOST_STACKTRACE_USE_ADDR2LINE 1
#define BOOST_STACKTRACE_ENABLE_BACKTRACE 1

#include <boost/stacktrace.hpp>

#include "tbox_backtrace.h"

#include <backtrace.h>


struct ServerFinder {
    //ws协议wsg gateway服务器id映射
    std::unordered_map<std::string,WsgGateway> wsServerMap;
    //wss协议wsg gateway服务器id映射
    std::unordered_map<std::string,WsgGateway> wssServerMap;
};

using namespace wsg::bus;

std::shared_ptr<ServerFinder> serviceFinder;
std::shared_ptr<WsGatewayManager> wsGatewayManager;
std::shared_ptr<WssGatewayManager> wssGatewayManager;


void routeMessageToWsGatewayManager(const std::string &msg,
                                    const std::string& fromWsgId) {
    if (wsGatewayManager) {
        wsGatewayManager->postMessage(msg, fromWsgId);
    }
}

void routeMessageToWssGatewayManager(const std::string &msg,
                                     const std::string& fromWsgId) {
    if (wssGatewayManager) {
        wssGatewayManager->postMessage(msg, fromWsgId);
    }
}

static int callback(void *data, uintptr_t pc, const char *filename,
                   int lineno, const char *function) {
    printf("  #%d: %s at %s:%d\n", (int)(size_t)data,
           function ? function : "??",
           filename ? filename : "??", lineno);
    return 0;
}

// 生成回溯信息
void print_backtrace(void) {
    backtrace_state *state = backtrace_create_state(NULL, 1, NULL, NULL);
    if (!state) {
        fprintf(stderr, "Failed to create backtrace state\n");
        return;
    }

    printf("Backtrace:\n");
    backtrace_full(state, 1, callback, NULL, NULL);
}


int websocket_router_no_tts_test_main(int argc, const char * argv[]) {
    serviceFinder = std::make_shared<ServerFinder>();
    wsGatewayManager = std::make_shared<WsGatewayManager>();
    wssGatewayManager = std::make_shared<WssGatewayManager>();
    
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
    
    wsGatewayManager->setServerMap(serviceFinder->wsServerMap);
    wssGatewayManager->setServerMap(serviceFinder->wssServerMap);
    
    boost::stacktrace::enable_backtrace_symbols_resolution();
    
    std::cout << boost::stacktrace::stacktrace() << std::endl;
    
    std::cout << tbox::DumpBacktrace() << std::endl;
    
    auto frame = boost::stacktrace::stacktrace()[0];
    std::cout << frame << std::endl;
    std::cout << frame.source_file() << ":" << frame.source_line();
    
    print_backtrace();
    
    if(serviceFinder->wsServerMap.empty() && serviceFinder->wssServerMap.empty()) {
        std::cout << "non config hwss servers for router " << std::endl;
        return 0;
    }
    if (!serviceFinder->wsServerMap.empty() && !serviceFinder->wssServerMap.empty()) {
        std::thread wssThread([] {
            wssGatewayManager->run();
        });
        wsGatewayManager->run();
        wssThread.join();
        return 0;
    }
    
    if (!serviceFinder->wsServerMap.empty()) {
        wsGatewayManager->run();
        return 0;
    }
    
    if (!serviceFinder->wssServerMap.empty()) {
        wssGatewayManager->run();
        return 0;
    }
    
    return 0;
}
