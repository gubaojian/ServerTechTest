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
    struct ClientMessage2
    {
        int32_t type;
        std::shared_ptr<std::string> msg;
        std::shared_ptr<std::string> adminId;
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

/**
 eventbus send event used 1656ms (json parse)
 eventbus send event used 3013ms
 eventbus reveive message 2000000
 eventbus thread handle essage used 3013ms
 **/
int event_bus_json_test2_main(int argc, const char * argv[]) {
    auto start = std::chrono::high_resolution_clock::now();
    auto end = std::chrono::high_resolution_clock::now();
    auto used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::string msg(1024, 'a');
   
    std::string authId = genUUID();
    std::string connId = genUUID();
    std::string appId = std::to_string(rand());
    std::string hwssId = genUUID();
    
    dexode::EventBus::Listener listener{bus};
    listener.listen([hwssId](const event::ClientMessage2& event) {
      
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
        yyjson_mut_doc *doc = yyjson_mut_doc_new(NULL);
        yyjson_mut_val *root = yyjson_mut_obj(doc);
        yyjson_mut_doc_set_root(doc, root);
        
        yyjson_mut_obj_add_str(doc, root, "action", "msg");
        yyjson_mut_obj_add_strn(doc, root, "msg", msg.data(), msg.size());
        yyjson_mut_obj_add_strn(doc, root, "authId", authId.data(), authId.length());
        yyjson_mut_obj_add_strn(doc, root, "connId", connId.data(), connId.length());
        yyjson_mut_obj_add_strn(doc, root, "appId", appId.data(), appId.length());
        yyjson_mut_obj_add_strn(doc, root, "hwssId", hwssId.data(), hwssId.length());
        size_t len;
        const char *json = yyjson_mut_write(doc, 0, &len);
        /**
        bus->postpone(event::ClientMessage2{
            .type  = 5,
            .msg = std::make_shared<std::string>(json, len),
            .adminId = std::make_shared<std::string>(connId)
        }); */
        
        if (json) {
            free((void *)json);
        }
        // Free the doc
        yyjson_mut_doc_free(doc);
    }
    end = std::chrono::high_resolution_clock::now();
    used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "eventbus send event used " << used.count() << "ms" << std::endl;
   
    processThread.join();
    
    return 0;
}
