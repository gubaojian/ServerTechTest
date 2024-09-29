//
//  uuid_compare_test.cc
//  cplus
//
//  Created by efurture on 2024/9/26.
//


/**
 *  高性能uuid库
 * https://github.com/crashoz/uuid_v4
 */


#undef HAS_UUID_V4

//#define HAS_UUID_V4

#include <string>
#include <iostream>
#include <uuid/uuid.h>
#include <chrono>
#include <map>
#include <set>
#include <cstdlib>
#include <unistd.h>

#ifdef HAS_UUID_V4
  #include <uuid_v4/uuid_v4.h>
#endif
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
/**
 * https://github.com/crashoz/uuid_v4.git
 */

//not fast
static inline std::string uuid_to_string(uuid_t& uuid) {
    std::string str(37, '\n');
    char* dest =  (char*)str.c_str();
    uuid_unparse(uuid, dest);
    return str;
}

static  std::string genUUID() {
    uuid_t uuid;
    char uuidStrBuffer[37];
    uuid_generate_random(uuid);
    uuid_unparse(uuid, uuidStrBuffer);
    std::string str(uuidStrBuffer, 36);
    return str;
}



int uuid_int128_map_test_main(int argc, const char * argv[]) {
    auto start = std::chrono::high_resolution_clock::now();
    auto end = std::chrono::high_resolution_clock::now();
    auto used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::unordered_map<std::string, std::string> uuidMap;
    uuid_t uuid;
    uuid_generate_random(uuid);
    std::string key1((char*)uuid, 16);
    uuidMap[key1] = "world";
    
    std::cout << uuid_to_string(uuid) << std::endl;
    
    uuid_t uuidKeySame;
    std::memcpy(uuidKeySame, uuid, sizeof(uuid_t));
    std::cout << uuid_to_string(uuidKeySame) << std::endl;
    std::string key2((char*)uuidKeySame, 16);
    
    std::cout << "uuid map find by key " << std::boolalpha << (uuidMap.find(key2) != uuidMap.end()) << std::endl;
    
    uuidMap[key2] = "world";
    
    std::cout << "uuid map size " << uuidMap.size() << std::endl;
    
  
    
    return 0;
}
