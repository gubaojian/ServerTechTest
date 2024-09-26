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

/**
 debug  mode
 
 
 std::unordered_map find used 1145ms
 std::unordered_map miss key find used 985ms
 std::unordered_map xxhash find used 1118ms
 std::unordered_map xxhash miss key find used 1045ms
 
 std::map find used 11096ms
 std::map miss key find used 5561ms
 
 release mode
 
 std::unordered_map find used 525ms
 std::unordered_map miss key find used 526ms
 std::unordered_map xxhash find used 527ms
 std::unordered_map xxhash miss key find used 531ms
 std::map find used 4815ms
 std::map miss key find used 609ms
 
 https://github.com/Cyan4973/xxHash/releases/tag/v0.8.2
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
    std::unordered_map<std::string, void*, StringxxHash> clientsX;
    std::map<std::string, void*> clients2;
    std::vector<std::string> keys;
    std::vector<std::string> missKeys;
    void* ptr = (void*)(&clients);
    start = std::chrono::high_resolution_clock::now();
    for(int i=0; i<10000*200; i++){
        uuid_t uuid;
        uuid_generate_random(uuid);
        uuid_string_t uuidStr;
        uuid_unparse(uuid, uuidStr);
        clients[uuidStr] = ptr;
        clients2[uuidStr] = ptr;
        
        clientsX[uuidStr] = ptr;
        
        keys.emplace_back(uuidStr);
        std::string missKey = "miss_";
        missKey.append(uuidStr);
        missKeys.emplace_back(missKey);
    }
    end = std::chrono::high_resolution_clock::now();
    used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "prepare data used " << used.count() << "ms" << std::endl;
    std::cout << "clients " << clients.size()  << std::endl;
    std::cout << "clients2 " << clients2.size() << std::endl;
    std::cout << "clientsx " << clientsX.size() << std::endl;
  
    
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
