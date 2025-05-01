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

#define HAS_UUID_V4

#include <string>
#include <iostream>
#include <uuid/uuid.h>
#include <folly/Random.h>
#include <chrono>
#include <map>
#include <set>
#include <cstdlib>
#include <unistd.h>

#ifdef HAS_UUID_V4
  #include <uuid_v4/uuid_v4.h>
#endif


#include "std_uuid.h"

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/random/linear_congruential.hpp>
/**
 * https://github.com/crashoz/uuid_v4.git
 * https://sourceforge.net/projects/libuuid/
 *
 * debug mode:
 * system uuid 417ms
 * uuidv4 uuid 327ms  (need enable avx512) 兼容性问题比较多。
 * boost uuid 256ms
 *
 * release mode

 system uuid 363ms
 system uuid share buffer 303ms
 std uuid share buffer 111ms
 uuidv4 uuid 70ms
 boost uuid random 1566ms
 boost with share buffer uuid 1455ms
 boost 19937 uuid 202ms
 boost rand48 uuid 227ms
 *
 *system uuid 373ms
 system uuid share buffer 297ms
 std uuid share buffer 114ms
 uuidv4 uuid 71ms
 boost uuid random 1572ms
 boost with share buffer uuid 1477ms
 boost 19937 uuid 163ms
 boost rand48 uuid 224ms
 
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
    
    start = std::chrono::high_resolution_clock::now();
    static char uuidShareBuffer[37];
    uuid_t uuid_share;
    for(int i=0; i<10000*100; i++) {
        uuid_t uuid_share2;
        uuid_generate_random(uuid_share2);
        char uuidShareBuffer2[37];
        uuid_unparse_lower(uuid_share2, uuidShareBuffer2);
        std::string (uuidShareBuffer2, 36);
    }
    end = std::chrono::high_resolution_clock::now();
    used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "system uuid share buffer " << used.count() << "ms " << std::endl;
  
    
    
    start = std::chrono::high_resolution_clock::now();
    std::random_device rd;
    auto seed_data = std::array<int, std::mt19937::state_size> {};
    std::generate(std::begin(seed_data), std::end(seed_data), std::ref(rd));
    std::seed_seq seq(std::begin(seed_data), std::end(seed_data));
    std::mt19937 engine(seq);
    static uuids::uuid_random_generator stdGen(engine);
    std::string stdUuid;
    for(int i=0; i<10000*100; i++) {
        auto uuid = stdGen();
        stdUuid = uuids::to_string(uuid);
    }
    end = std::chrono::high_resolution_clock::now();
    used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "std uuid share buffer " << used.count() << "ms " << std::endl;
  
  
#ifdef HAS_UUID_V4
    UUIDv4::UUIDGenerator<std::mt19937_64> uuidGenerator;
    start = std::chrono::high_resolution_clock::now();
    for(int i=0; i<10000*100; i++) {
        UUIDv4::UUID uuid = uuidGenerator.getUUID();
        std::string str = uuid.str();
    }
    end = std::chrono::high_resolution_clock::now();
    used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "uuidv4 uuid " << used.count() << "ms " << std::endl;
#endif
    
    
    start = std::chrono::high_resolution_clock::now();
    auto gen = boost::uuids::random_generator();
    for(int i=0; i<10000*100; i++) {
        boost::uuids::uuid u = gen();
        std::string str = boost::uuids::to_string(u);
    }
    end = std::chrono::high_resolution_clock::now();
    used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "boost uuid random " << used.count() << "ms " << std::endl;
    
    start = std::chrono::high_resolution_clock::now();
    for(int i=0; i<10000*100; i++) {
        boost::uuids::uuid u = gen();
        boost::uuids::to_chars(u, uuidShareBuffer);
        std::string str(uuidShareBuffer, 36);
    }
    end = std::chrono::high_resolution_clock::now();
    used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "boost with share buffer uuid " << used.count() << "ms " << std::endl;
    
    
    
    
     boost::uuids::basic_random_generator<std::mt19937> gen_mt;
     start = std::chrono::high_resolution_clock::now();
     for(int i=0; i<10000*100; i++) {
         boost::uuids::uuid u = gen_mt();
         //std::string str = boost::uuids::to_string(u);
         char uuidShareBuffer2[37];
         uuid_unparse(uuid_share, uuidShareBuffer2);
         boost::uuids::to_chars(u, uuidShareBuffer2);
         std::string str(uuidShareBuffer2, 36);
     }
     end = std::chrono::high_resolution_clock::now();
     used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
     std::cout << "boost 19937 uuid " << used.count() << "ms " << std::endl;
   
    
    boost::uuids::basic_random_generator<boost::rand48> gen_mt2;
    start = std::chrono::high_resolution_clock::now();
    for(int i=0; i<10000*100; i++) {
        boost::uuids::uuid u = gen_mt2();
        std::string str = boost::uuids::to_string(u);
    }
    end = std::chrono::high_resolution_clock::now();
    used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "boost rand48 uuid " << used.count() << "ms " << std::endl;
  
    
    {
        boost::uuids::uuid u = boost::uuids::random_generator()();
        boost::uuids::to_chars(u, uuidShareBuffer);
        std::string str(uuidShareBuffer, 36);
        std::cout << "boost uuid " << str << " " << std::endl;
    }
    
    {
        boost::uuids::uuid u = boost::uuids::random_generator()();
        std::string str =  boost::uuids::to_string(u);
        std::cout << "boost uuid " << str << " " << std::endl;
    }
    
    {
        boost::uuids::uuid u;
        std::string str =  boost::uuids::to_string(u);
        std::cout << "boost uuid bad useage " << str << " " << std::endl;
    }
    
    {
        boost::uuids::uuid u;
        std::string str =  boost::uuids::to_string(u);
        std::cout << "boost uuid bad useage " << str << " " << std::endl;
    }
  

    {
        boost::uuids::uuid u = gen_mt();
        std::string str =  boost::uuids::to_string(u);
        std::cout << "boost uuid 19937 useage " << str << " " << std::endl;
    }
    
    {
        boost::uuids::uuid u = gen_mt();
        std::string str =  boost::uuids::to_string(u);
        std::cout << "boost uuid 19937 useage " << str << " " << std::endl;
    }
    
    {
        uuid_generate_random(uuid_share);
        char uuidShareBuffer2[37];
        uuid_unparse_lower(uuid_share, uuidShareBuffer2);
        std::string str(uuidShareBuffer2, 36);
        std::cout << "system uuid useage " << str << " " << std::endl;
    }
    
    {
        for(int i=0; i<10; i++) {
            std::cout << uuids::to_string(stdGen()) << std::endl;
        }
        std::cout << "std uuid useage " << stdUuid << " " << std::endl;
    }

    
    
    
    return 0;
}
