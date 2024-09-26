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
 
 
 std::unordered_map int find used 603ms
 std::unordered_map int miss key find used 626ms
 std::map int find used 2336ms 453330920
 std::map int miss key find used 2492ms -1570091909
 
 release mode
 
 std::unordered_map int find used 202ms
 std::unordered_map int miss key find used 322ms
 std::map int find used 1ms -1229735852
 std::map int miss key find used 1ms 54148455
 
 https://github.com/Cyan4973/xxHash/releases/tag/v0.8.2
 */



int map_find_test_int_main(int argc, const char * argv[]) {
    auto start = std::chrono::high_resolution_clock::now();
    auto end = std::chrono::high_resolution_clock::now();
    auto used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
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
