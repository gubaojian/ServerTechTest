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

/**
 debug  mode
 
 

 
 release mode
  
 
 https://github.com/ibireme/yyjson
 
 https://github.com/simdjson/simdjson?tab=readme-ov-file#quick-start
 
 */



int json_test_main(int argc, const char * argv[]) {
    auto start = std::chrono::high_resolution_clock::now();
    auto end = std::chrono::high_resolution_clock::now();
    auto used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Create a mutable doc
    yyjson_mut_doc *doc = yyjson_mut_doc_new(NULL);
    yyjson_mut_val *root = yyjson_mut_obj(doc);
    yyjson_mut_doc_set_root(doc, root);
    
    
    yyjson_mut_obj_add_str(doc, root, "appId", "444");
    yyjson_mut_obj_add_int(doc, root, "star", 4);
    
    std::unordered_map<int32_t, void*> clients;
    std::map<int32_t, void*> clients2;
    std::vector<int32_t> keys;
    std::vector<int32_t> missKeys;
    void* ptr = (void*)(&clients);
    start = std::chrono::high_resolution_clock::now();
    for(int i=0; i<10000*200; i++){
        int32_t num = arc4random();
        clients[num] = ptr;
        clients2[num] = ptr;
        keys.emplace_back(num);
        missKeys.emplace_back(arc4random());
    }
    end = std::chrono::high_resolution_clock::now();
    used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "prepare data used " << used.count() << "ms" << std::endl;
    std::cout << "clients " << clients.size()  << std::endl;
    std::cout << "clients2 " << clients2.size() << std::endl;
  
    
    start = std::chrono::high_resolution_clock::now();
    for(const auto& key : keys) {
        clients.find(key);
    }
    end = std::chrono::high_resolution_clock::now();
    used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "std::unordered_map int find used " << used.count() << "ms" << std::endl;
  
    start = std::chrono::high_resolution_clock::now();
    for(const auto& key : missKeys) {
        clients.find(key);
    }
    end = std::chrono::high_resolution_clock::now();
    used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "std::unordered_map int miss key find used " << used.count() << "ms" << std::endl;
    
   
    
    start = std::chrono::high_resolution_clock::now();
    int sum = 0;
    for(const auto& key : keys) {
        clients2.find(key);
        sum += key;
    }
    end = std::chrono::high_resolution_clock::now();
    used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "std::map int find used " << used.count() << "ms " << sum << std::endl;
  
    start = std::chrono::high_resolution_clock::now();
    for(const auto& key : missKeys) {
        clients2.find(key);
        sum += key;
    }
    end = std::chrono::high_resolution_clock::now();
    used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "std::map int miss key find used " << used.count() << "ms "  << sum  << std::endl;
  

    return 0;
}
