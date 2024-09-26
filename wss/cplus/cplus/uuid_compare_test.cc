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
 * debug mode:
 * system uuid 417ms
 * uuidv4 uuid 327ms  (need enable avx512) 兼容性问题比较多。
 * boost uuid 256ms
 *
 * release mode
 * system uuid 372ms
 * boost uuid 68ms  (boost默认uuid性能好一些)
 * boost uuid 7f6abeb9-facf-45b3-93bf-7f12b6db8048
 * boost 19937 uuid 218ms
 */
int uuid_compare_test_main(int argc, const char * argv[]) {
    auto start = std::chrono::high_resolution_clock::now();
    auto end = std::chrono::high_resolution_clock::now();
    auto used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
  
    start = std::chrono::high_resolution_clock::now();
    for(int i=0; i<10000*100; i++) {
        uuid_t uuid;
        uuid_generate_random(uuid);
        std::string str;
        str.resize(37);
        char* uuidStr2 =  (char*)str.c_str();
        uuid_unparse(uuid, uuidStr2);
    }
    end = std::chrono::high_resolution_clock::now();
    used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "system uuid " << used.count() << "ms " << std::endl;
  
#ifdef HAS_UUID_V4
    UUIDv4::UUIDGenerator<std::mt19937_64> uuidGenerator;
    start = std::chrono::high_resolution_clock::now();
    for(int i=0; i<10000*100; i++) {
        UUIDv4::UUID uuid = uuidGenerator.getUUID();
        std::string str;
        uuid.str(str);
    }
    end = std::chrono::high_resolution_clock::now();
    used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "uuidv4 uuid " << used.count() << "ms " << std::endl;
#endif
    
    
    start = std::chrono::high_resolution_clock::now();
    for(int i=0; i<10000*100; i++) {
        boost::uuids::uuid u;
        std::string str = boost::uuids::to_string(u);
    }
    end = std::chrono::high_resolution_clock::now();
    used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    boost::uuids::uuid u;
    std::string str =  boost::uuids::to_string(u);
    std::cout << "boost uuid " << used.count() << "ms " << std::endl;
    std::cout << "boost uuid " << str << std::endl;
    
   
    boost::uuids::basic_random_generator<boost::mt19937> gen;
    start = std::chrono::high_resolution_clock::now();
    for(int i=0; i<10000*100; i++) {
        boost::uuids::uuid u = gen();
        std::string str = boost::uuids::to_string(u);
    }
    end = std::chrono::high_resolution_clock::now();
    used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "boost 19937 uuid " << used.count() << "ms " << std::endl;
  
   

    
    return 0;
}
