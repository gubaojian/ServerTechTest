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



#include "lib/yyjson/yyjson.h"

#include "lib/simdjson/simdjson.h"

/**
 debug  mode
 
 

 
 release mode
  
 
 https://github.com/ibireme/yyjson
 
 https://github.com/simdjson/simdjson?tab=readme-ov-file#quick-start
 
 */
static std::string genUUID() {
    uuid_t uuid;
    char uuidStrBuffer[37];
    uuid_generate_random(uuid);
    uuid_unparse(uuid, uuidStrBuffer);
    std::string str(uuidStrBuffer, 36);
    return str;
}


/**
 
 yyjson tojson used 1807ms
 yyjson tojson speed 2305.98mb
 yyjson parse json used 1041ms
 yyjson parse json speed 2305.98mb
 yyjson parse instu json used 779ms
 yyjson parse instu json speed 2305.98mb
 
 yyjson tojson used 1698ms
 yyjson tojson speed 2305.98mb
 yyjson parse json used 1011ms
 yyjson parse json speed 2305.98mb
 yyjson parse instu json used 839ms
 yyjson parse instu json speed 2305.98mb
 simdjson parse json used 495ms
 simdjson parse json speed 2305.98mb
 
 */

int json_test_main(int argc, const char * argv[]) {
    auto start = std::chrono::high_resolution_clock::now();
    auto end = std::chrono::high_resolution_clock::now();
    auto used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::string msg(1024, 'b');
    std::string authId = genUUID();
    std::string connId = genUUID();
    std::string appId = std::to_string(rand());
    std::string hwssId = genUUID();
    std::string msgJson;
    // Create a mutable doc
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
    if (json) {
        std::string_view jsonView(json, len);
        std::cout << "tojson " << jsonView  << std::endl;
    }
    
    msgJson.append(json, len);
   
    if (json) {
        free((void *)json);
    }
    // Free the doc
    yyjson_mut_doc_free(doc);
    size_t length =10000*200;
   
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
        if (!json) {
            std::cout << "parse error " << std::endl;
            break;
        }
        
        
       
        if (json) {
            free((void *)json);
        }
        // Free the doc
        yyjson_mut_doc_free(doc);
    }
    end = std::chrono::high_resolution_clock::now();
    used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "yyjson tojson used " << used.count() << "ms" << std::endl;
    std::cout << "yyjson tojson speed " << (length*len)/(1024*1024.0) << "mb" << std::endl;
   
    
    start = std::chrono::high_resolution_clock::now();
  
    for(int i=0; i<length; i++){
        yyjson_doc *doc = yyjson_read(msgJson.data(), msgJson.size(), 0);
        yyjson_val *root = yyjson_doc_get_root(doc);

        // Get root["name"]
        yyjson_val *action = yyjson_obj_get(root, "action");
        yyjson_val *msg = yyjson_obj_get(root, "msg");
        yyjson_val *connId = yyjson_obj_get(root, "connId");
        yyjson_val *authId = yyjson_obj_get(root, "authId");
        yyjson_val *hwssId = yyjson_obj_get(root, "hwssId");
        if (action == NULL
            || msg == NULL
            || connId == NULL
            || authId == NULL) {
            break;
        }
        // Free the doc
        yyjson_doc_free(doc);
    }
    end = std::chrono::high_resolution_clock::now();
    used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "yyjson parse json used " << used.count() << "ms" << std::endl;
    std::cout << "yyjson parse json speed " << (length*len)/(1024*1024.0) << "mb" << std::endl;
   

    

    start = std::chrono::high_resolution_clock::now();
    
    
    //yyjson_alc *alc = yyjson_alc_dyn_new();

    for(int i=0; i<length; i++) {
        char buffer[2048];
        std::memcpy(buffer, msgJson.data(), msgJson.size());
        yyjson_doc *doc = yyjson_read_opts(buffer, msgJson.size(), YYJSON_READ_INSITU, NULL, NULL);
        yyjson_val *root = yyjson_doc_get_root(doc);

        // Get root["name"]
        yyjson_val *action = yyjson_obj_get(root, "action");
        yyjson_val *msg = yyjson_obj_get(root, "msg");
        yyjson_val *connId = yyjson_obj_get(root, "connId");
        yyjson_val *authId = yyjson_obj_get(root, "authId");
        yyjson_val *hwssId = yyjson_obj_get(root, "hwssId");
        if (action == NULL
            || msg == NULL
            || connId == NULL
            || authId == NULL) {
            std::cout << "parse error " << std::endl;
            break;
        }
        // Free the doc
        yyjson_doc_free(doc);
    }
    // free the allocator
    //yyjson_alc_dyn_free(alc);
    
    end = std::chrono::high_resolution_clock::now();
    used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "yyjson parse instu json used " << used.count() << "ms" << std::endl;
    std::cout << "yyjson parse instu json speed " << (length*len)/(1024*1024.0) << "mb" << std::endl;
    
    
    
    start = std::chrono::high_resolution_clock::now();
    //解析器共用,提前初始化好，不然会耗时。
    simdjson::ondemand::parser parser;
    
    for(int i=0; i<length; i++) {
    
        simdjson::ondemand::document  doc;
        auto error = parser.iterate(msgJson).get(doc);
        if(error) {
            std::cout << "parse error " << std::endl;
        }
       
        auto action = doc["action"].get_string();
        auto authId = doc["authId"].get_string();
        auto msg = doc["msg"].get_string();
        auto connId = doc["connId"].get_string();
        auto hwssId = doc["hwssId"].get_string();
        if(action.error()
           || authId.error()
           || msg.error()
           || connId.error()
           || hwssId.error()) {
            std::cout << "parse error eee " << std::endl;
        }
    }
   
    
    end = std::chrono::high_resolution_clock::now();
    used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "simdjson parse json used " << used.count() << "ms" << std::endl;
    std::cout << "simdjson parse json speed " << (length*len)/(1024*1024.0) << "mb" << std::endl;
    {
        simdjson::ondemand::document  doc;
        auto error = parser.iterate(msgJson).get(doc);
        if(error) {
            std::cout << "parse error " << std::endl;
        }
       
        auto actionNot = doc["msg_edd"].get_string();
        auto authId = doc["authId"].get_string();
        auto error2 = actionNot.error();
        
        //std::cout << "authId " << " " << simdjson::error_message(error2) << std::endl;
        
        std::cout << "authId " << authId.value_unsafe() << std::endl;
    }
    
    
   
    
    
    return 0;
}
