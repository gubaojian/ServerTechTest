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

// clang++ -E -v -x c++ /dev/null
// see include path
#include <xxhash.h>

#include "lib/tsl/robin_map.h"

// 需要开启SSE4
//#define HAS_FOLLY

#ifdef  HAS_FOLLY
  #include <folly/container/F14Map.h>
#endif

// #include <absl/container/flat_hash_map.h>
// absl 依赖较多，cmake处理更方便
/**
 
 release mode
  200 * 10000 规模：
 tsl::robin_map find used 367ms
 tsl::robin_map miss key find used 325ms
 folly::fast_map find used 454ms
 folly::fast_map miss key find used 188ms
 std::unordered_map find used 583ms
 std::unordered_map miss key find used 558ms
 std::unordered_map small find used 149ms
 std::unordered_map small miss key find used 138ms
 tsl::robin_map small find used 141ms
 tsl::robin_map small miss key find used 138ms
 std::unordered_map xxhash find used 614ms
 std::unordered_map xxhash miss key find used 623ms
 std::map find used 5411ms
 std::map miss key find used 5782ms
 
 https://github.com/Cyan4973/xxHash/releases/tag/v0.8.2
 
 https://github.com/Tessil/robin-map
 
 */

struct StringxxHash {
    std::size_t operator()(const std::string& key) const {
        return XXH32(key.c_str(), key.size(), 0);
    }
};


int map_find_test_main(int argc, const char * argv[]) {
    auto start = std::chrono::high_resolution_clock::now();
    auto end = std::chrono::high_resolution_clock::now();
    auto used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::unordered_map<std::string, void*> clients;
    std::unordered_map<std::string, void*> admins;
    std::unordered_map<std::string, void*, StringxxHash> clientsX;
    std::map<std::string, void*> clients2;
    tsl::robin_map<std::string, void*> robinMap;
    tsl::robin_map<std::string, void*> admins2;
    
#ifdef  HAS_FOLLY
    folly::F14FastMap<std::string, void*> follyMap;
#endif
    
    std::vector<std::string> keys;
    std::vector<std::string> missKeys;
    void* ptr = (void*)(&clients);
    start = std::chrono::high_resolution_clock::now();
    for(int i=0; i<10000*150; i++){
        uuid_t uuid;
        uuid_generate_random(uuid);
        uuid_string_t uuidStr;
        uuid_unparse(uuid, uuidStr);
        std::string key(uuidStr, 36);
        clients[key] = ptr;
        clients2[key] = ptr;
        
        clientsX[key] = ptr;
        
        robinMap[key] = ptr;
        
#ifdef  HAS_FOLLY
        follyMap[key] = ptr;
#endif
        
        keys.emplace_back(uuidStr);
        if (admins.size() < 512) {
            admins[key] = ptr;
            admins2[key] = ptr;
        }
        
        std::string missKey(uuidStr, 36);
        std::reverse(missKey.begin(), missKey.end());
        missKeys.emplace_back(missKey);
    }
    end = std::chrono::high_resolution_clock::now();
    used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "prepare data used " << used.count() << "ms" << std::endl;
    std::cout << "clients " << clients.size()  << std::endl;
    std::cout << "clients2 " << clients2.size() << std::endl;
    std::cout << "clientsx " << clientsX.size() << std::endl;
    std::cout << "robinMap " << robinMap.size() << std::endl;
  
    
    start = std::chrono::high_resolution_clock::now();
    for(const auto& key : keys) {
        robinMap.find(key);
    }
    end = std::chrono::high_resolution_clock::now();
    used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "tsl::robin_map find used " << used.count() << "ms" << std::endl;
  
    start = std::chrono::high_resolution_clock::now();
    for(const auto& key : missKeys) {
        robinMap.find(key);
    }
    end = std::chrono::high_resolution_clock::now();
    used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "tsl::robin_map miss key find used " << used.count() << "ms" << std::endl;
    
#ifdef  HAS_FOLLY
    start = std::chrono::high_resolution_clock::now();
    for(const auto& key : keys) {
        follyMap.find(key);
    }
    end = std::chrono::high_resolution_clock::now();
    used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "folly::fast_map find used " << used.count() << "ms" << std::endl;
  
    start = std::chrono::high_resolution_clock::now();
    for(const auto& key : missKeys) {
        follyMap.find(key);
    }
    end = std::chrono::high_resolution_clock::now();
    used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "folly::fast_map miss key find used " << used.count() << "ms" << std::endl;
#endif
   
    
    
    start = std::chrono::high_resolution_clock::now();
    for(const auto& key : keys) {
        clients.find(key);
    }
    end = std::chrono::high_resolution_clock::now();
    used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "std::unordered_map find used " << used.count() << "ms" << std::endl;
  
    start = std::chrono::high_resolution_clock::now();
    for(const auto& key : missKeys) {
        clients.find(key);
    }
    end = std::chrono::high_resolution_clock::now();
    used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "std::unordered_map miss key find used " << used.count() << "ms" << std::endl;
    
    start = std::chrono::high_resolution_clock::now();
    for(const auto& key : keys) {
        admins.find(key);
    }
    end = std::chrono::high_resolution_clock::now();
    used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "std::unordered_map small find used " << used.count() << "ms" << std::endl;
  
    start = std::chrono::high_resolution_clock::now();
    for(const auto& key : missKeys) {
        admins.find(key);
    }
    end = std::chrono::high_resolution_clock::now();
    used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "std::unordered_map small miss key find used " << used.count() << "ms" << std::endl;
    
    
    start = std::chrono::high_resolution_clock::now();
    for(const auto& key : keys) {
        admins.find(key);
    }
    end = std::chrono::high_resolution_clock::now();
    used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "tsl::robin_map small find used " << used.count() << "ms" << std::endl;
  
    start = std::chrono::high_resolution_clock::now();
    for(const auto& key : missKeys) {
        admins.find(key);
    }
    end = std::chrono::high_resolution_clock::now();
    used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "tsl::robin_map small miss key find used " << used.count() << "ms" << std::endl;
    
    
    
    start = std::chrono::high_resolution_clock::now();
    for(const auto& key : keys) {
        clientsX.find(key);
    }
    end = std::chrono::high_resolution_clock::now();
    used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "std::unordered_map xxhash find used " << used.count() << "ms" << std::endl;
  
    start = std::chrono::high_resolution_clock::now();
    for(const auto& key : missKeys) {
        clientsX.find(key);
    }
    end = std::chrono::high_resolution_clock::now();
    used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "std::unordered_map xxhash miss key find used " << used.count() << "ms" << std::endl;
    
    
    start = std::chrono::high_resolution_clock::now();
    for(const auto& key : keys) {
        clients2.find(key);
    }
    end = std::chrono::high_resolution_clock::now();
    used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "std::map find used " << used.count() << "ms" << std::endl;
  
    start = std::chrono::high_resolution_clock::now();
    for(const auto& key : missKeys) {
        clients2.find(key);
    }
    end = std::chrono::high_resolution_clock::now();
    used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "std::map miss key find used " << used.count() << "ms" << std::endl;
  
    

    

    return 0;
}
