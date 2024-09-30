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


static auto bus = std::make_shared<dexode::EventBus>();

namespace event // optional namespace
{
    struct Gold
    {
        int goldReceived = 0;
    };

    struct OK {}; // Simple event when user press "OK" button
}

/**
 * 200 万
 * eventbus send event used 566ms
 * eventbus reveive message 2000001
 * eventbus thread handle essage used 567ms
 */
int event_bus_test_main(int argc, const char * argv[]) {
    auto start = std::chrono::high_resolution_clock::now();
    auto end = std::chrono::high_resolution_clock::now();
    auto used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
   
    
    dexode::EventBus::Listener listener{bus};
    listener.listen([](const event::Gold& event) {
                        
    });
    
    bus->postpone<event::Gold>({12}); // 2 way
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
        bus->postpone(event::Gold{12});
    }
    end = std::chrono::high_resolution_clock::now();
    used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "eventbus send event used " << used.count() << "ms" << std::endl;
   
    processThread.join();
    
    return 0;
}
