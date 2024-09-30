//
//  map_find_test.cc
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
#include <thread>

#include "dexode/EventBus.hpp"

#include "lib/yyjson/yyjson.h"



static auto bus = std::make_shared<dexode::EventBus>();

namespace event // optional namespace
{
    struct RouteMessage
    {
        int32_t hwssId;
        std::shared_ptr<std::string> msg;
    };
}

static std::string genUUID() {
    uuid_t uuid;
    char uuidStrBuffer[37];
    uuid_generate_random(uuid);
    uuid_unparse(uuid, uuidStrBuffer);
    std::string str(uuidStrBuffer, 36);
    return str;
}

static std::atomic<int> msgCount;
 /**
  
  eventbus send event used 875ms
  eventbus reveive message end2000000
  eventbus reveive message 2000000
  eventbus thread handle essage used 875ms
  
  */
int event_bus_json_router_test_main(int argc, const char * argv[]) {
    auto start = std::chrono::high_resolution_clock::now();
    auto end = std::chrono::high_resolution_clock::now();
    auto used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::string msg(2048, 'a');
   
    std::string authId = genUUID();
    std::string connId = genUUID();
    std::string appId = std::to_string(rand());
    std::string hwssId = genUUID();
    
    dexode::EventBus::Listener listener{bus};
    listener.listen([hwssId](const event::RouteMessage& event) {
        msgCount++;
        if (msgCount >= 10000*200) {
            std::cout << "eventbus reveive message end" << msgCount << std::endl;
        }
    });
    
    size_t length =10000*200;
    std::thread processThread([length, start]{
        int i=0;
        while(i < length) {
            i += bus->process();
            usleep(100);
        }
        auto end = std::chrono::high_resolution_clock::now();
        auto used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "eventbus reveive message " << i << std::endl;
        std::cout << "eventbus thread handle essage used " << used.count() << "ms" << std::endl;
    });
    
    
    start = std::chrono::high_resolution_clock::now();
    for(int i=0; i<length; i++){
        bus->postpone(event::RouteMessage{
            .hwssId  = 5,
            .msg = std::make_shared<std::string>(msg),
        });
    }
    end = std::chrono::high_resolution_clock::now();
    used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "eventbus send event used " << used.count() << "ms" << std::endl;
   
    processThread.join();
    
    return 0;
}
