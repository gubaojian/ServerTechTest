//
//  main.cpp
//  ImproveWs
//
//  Created by efurture on 2025/6/21.
//

#include <iostream>
#include "manager/WsSlotManager.hpp"
#include <chrono>
#include <thread>
#include <memory>


namespace websocketpp {
    namespace processor {
         thread_local none_lock_rng_type _m_fast_rng;
    }
}




int main(int argc, const char * argv[]) {
    // insert code here...
    std::cout << "Hello, World!\n";
    
    std::unordered_map<std::string, WsgSlot> map;
    map["testId"] = {"testId", "ws://127.0.0.1:9001/wsg?role=client&appId=434608808308&appToken=mtnXNik41BYaUSNgLDxWmxoDCmUyl9El"};
    
    wsg::bus::WsSlotManager wsSlotManager;
    wsSlotManager.setSlotsMap(map);
    wsg::bus::WsSlotManager* ptr = &wsSlotManager;
    std::thread test([ptr]{
         // 休眠 1 秒
         std::this_thread::sleep_for(std::chrono::seconds(1));
        std::string message(1025, 'b');
        auto toWsgId = std::make_shared<std::string>("testId");
        auto fromWsgId = std::make_shared<std::string>("pusherSource");
        
        
        auto start = std::chrono::high_resolution_clock::now();
        for(int i=0; i<1024*1024*4; i++) {
            auto routeMessage = std::make_shared<RouteMessage>();
            routeMessage->toWsgId = toWsgId;
            routeMessage->fromWsgId = fromWsgId;
            routeMessage->msg = std::make_shared<std::string>(message);
            ptr->postRouteMessageToEncodeFrameThread(routeMessage);
            //ptr->postRouteMessage(datas.at(i), toWsgId, fromWsgId);
        }
        auto end = std::chrono::high_resolution_clock::now();
        auto used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "send used " << used.count() << std::endl;
    });
    /**
    std::thread  two([]{
        std::unordered_map<std::string, WsgSlot> map;
        map["testId"] = {"testId", "ws://127.0.0.1:9001/wsg?role=client&appId=434608808308&appToken=mtnXNik41BYaUSNgLDxWmxoDCmUyl9El"};
        
        wsg::bus::WsSlotManager wsSlotManager2;
        wsSlotManager2.setSlotsMap(map);
        wsg::bus::WsSlotManager* ptr = &wsSlotManager2;
        
        std::thread test2([ptr]{
             // 休眠 1 秒
             std::this_thread::sleep_for(std::chrono::seconds(1));
            std::string message(1025, 'b');
            auto toWsgId = std::make_shared<std::string>("testId");
            auto fromWsgId = std::make_shared<std::string>("pusherSource");
         
            
            auto start = std::chrono::high_resolution_clock::now();
            for(int i=0; i<1024*1024*4; i++) {
                auto routeMessage = std::make_shared<RouteMessage>();
                routeMessage->toWsgId = toWsgId;
                routeMessage->fromWsgId = fromWsgId;
                routeMessage->msg = std::make_shared<std::string>(message);
                ptr->postRouteMessageToEncodeFrameThread(routeMessage);
                //ptr->postRouteMessage(datas.at(i), toWsgId, fromWsgId);
            }
            auto end = std::chrono::high_resolution_clock::now();
            auto used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            std::cout << "send used " << used.count() << std::endl;
        });
        wsSlotManager2.run();
    });*/
    
    wsSlotManager.run();
    
    
    
    return 0;
}
