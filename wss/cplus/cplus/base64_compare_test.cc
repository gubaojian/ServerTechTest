//
//  uuid_compare_test.cc
//  cplus
//
//  Created by efurture on 2024/9/26.
//


/**
 *  高性能base64库
 * https://github.com/powturbo/Turbo-Base64?tab=readme-ov-file
 *
 * https://github.com/aklomp/base64
 *
 * https://github.com/tobiaslocker/base64
 *
 * https://github.com/matheusgomes28/base64pp
 *
 install:
 cp libtb64.so /usr/local/lib/
 cp libtb64.a /usr/local/lib/
 cp turbob64.h /usr/local/include
 *
 * https://github.com/aklomp/base64
 */


#include <string>
#include <iostream>
#include <chrono>
#include <map>
#include <set>
#include <cstdlib>
#include <unistd.h>

#include <turbob64.h>

#include <libbase64.h>

#include <boost/beast/core/detail/base64.hpp>

#include "lib/base64.hpp"


#include "lib/base64pp.h"

#include "lib/modp_b64/modp_b64.h"



/**
 *
 * debug mode:
 * turbo base64 175ms
 turbo base64 with share_buffer 87ms
 boost base64 2074ms
 boost with share buffer base64 1911ms
 libbase64 base64 397ms
 libbase64 base64 share buffer 307ms
 * base64 header base64 4046ms
 *
 *
 *
 * release mode
 *  turbo base64 142ms
 *  turbo base64 with share_buffer 69ms
 *  boost base64 704ms
 *  boost with share buffer base64 617ms
 *  libbase64 base64 362ms
 *  libbase64 base64 share buffer 296ms
 *  base64 header base64 578ms
 *   mod base64 723ms
 *  base64pp base64 4419ms （数据减小10倍）
 */
int base64_compare_test_main(int argc, const char * argv[]) {
    auto start = std::chrono::high_resolution_clock::now();
    auto end = std::chrono::high_resolution_clock::now();
    auto used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    start = std::chrono::high_resolution_clock::now();
    char bts[1024];
    arc4random_buf(bts, 1024);
    
    
    start = std::chrono::high_resolution_clock::now();
    for(int i=0; i<10000*100; i++) {
        std::string buffer(2048, '\n');
        unsigned char *in = (unsigned char *)bts;
        unsigned char *out = (unsigned char *)buffer.c_str();
        size_t len = tb64enc(in, 1024, out);
        buffer.resize(len);
    }
    end = std::chrono::high_resolution_clock::now();
    used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "turbo base64 " << used.count() << "ms " << std::endl;
    
    start = std::chrono::high_resolution_clock::now();
    std::string share_buffer(2048, '\n');
     for(int i=0; i<10000*100; i++) {
         unsigned char *in = (unsigned char *)bts;
         unsigned char *out = (unsigned char *)share_buffer.c_str();
         size_t len = tb64enc(in, 1024, out);
         share_buffer.resize(len);
     }
     end = std::chrono::high_resolution_clock::now();
     used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
     std::cout << "turbo base64 with share_buffer " << used.count() << "ms " << std::endl;
     
    
    start = std::chrono::high_resolution_clock::now();
    for(int i=0; i<10000*100; i++) {
        std::string buffer(2048, '\n');
        void* dest = (void*)buffer.c_str();
        size_t len = boost::beast::detail::base64::encode(dest, bts, 1024);
        buffer.resize(len);
    }
    end = std::chrono::high_resolution_clock::now();
    used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "boost base64 " << used.count() << "ms " << std::endl;
    
    
    start = std::chrono::high_resolution_clock::now();
    for(int i=0; i<10000*100; i++) {
        void* dest = (void*)share_buffer.c_str();
        size_t len = boost::beast::detail::base64::encode(dest, bts, 1024);
        share_buffer.resize(len);
    }
    end = std::chrono::high_resolution_clock::now();
    used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "boost with share buffer base64 " << used.count() << "ms " << std::endl;
    
    start = std::chrono::high_resolution_clock::now();
    for(int i=0; i<10000*100; i++) {
        std::string buffer(2048, '\n');
        char* dest = (char*)buffer.c_str();
        const char* in = (const char *)bts;
        size_t len;
        base64_encode(in, 1024, dest, &len, 0);
        buffer.resize(len);
    }
    end = std::chrono::high_resolution_clock::now();
    used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "libbase64 base64 " << used.count() << "ms " << std::endl;
    

    start = std::chrono::high_resolution_clock::now();
    for(int i=0; i<10000*100; i++) {
        char* dest = (char*)share_buffer.c_str();
        const char* in = (const char *)bts;
        size_t len;
        base64_encode(in, 1024, dest, &len, 0);
        share_buffer.resize(len);
    }
    end = std::chrono::high_resolution_clock::now();
    used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "libbase64 base64 share buffer " << used.count() << "ms " << std::endl;
    
    
    start = std::chrono::high_resolution_clock::now();
    std::string_view data(bts, 1024);
    for(int i=0; i<10000*100; i++) {
        base64::to_base64(data);
    }
    end = std::chrono::high_resolution_clock::now();
    used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "base64 header base64 " << used.count() << "ms " << std::endl;
    
    
    start = std::chrono::high_resolution_clock::now();
    for(int i=0; i<10000*100; i++) {
        std::string data(bts, 1024);
        std::string r = modp_b64_encode(data);
    }
    end = std::chrono::high_resolution_clock::now();
    used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::string str(bts, 1024);
    std::cout << "mod base64 " << used.count() << "ms " << std::endl;
    
    /** too slow
    start = std::chrono::high_resolution_clock::now();
    for(int i=0; i<10000*10; i++) {
        base64pp::encode_str(data);
    }
    end = std::chrono::high_resolution_clock::now();
    used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "base64pp base64 " << used.count() << "ms " << base64pp::encode_str(data) << std::endl;
     */
  
   
    
    return 0;
}
