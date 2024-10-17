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
 install:
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
 *
 *
 * release mode
 * turbo base64 144ms
 * turbo base64 with share_buffer 70ms
 */
int base64_usage_test_main(int argc, const char * argv[]) {
    auto start = std::chrono::high_resolution_clock::now();
    auto end = std::chrono::high_resolution_clock::now();
    auto used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    start = std::chrono::high_resolution_clock::now();
    char bts[1024];
    arc4random_buf(bts, 1024);
    
    {
        std::string buffer(2048, '\n');
        unsigned char *in = (unsigned char *)bts;
        unsigned char *out = (unsigned char *)buffer.c_str();
        size_t len = tb64enc(in, 1024, out);
        buffer.resize(len);
        
        std::cout << "turboBase64 " << std::endl << buffer << " " << buffer.capacity()  << std::endl;
    }
    
    {
        std::string buffer(2048, '\n');
        void* dest = (void*)buffer.c_str();
        size_t len = boost::beast::detail::base64::encode(dest, bts, 1024);
        buffer.resize(len);
        std::cout << "boostBase64 " << std::endl << buffer << " " << buffer.capacity()  << std::endl;
    }
    
    {
        std::string buffer(2048, '\n');
        char* dest = (char*)buffer.c_str();
        const char* in = (const char *)bts;
        size_t len;
        base64_encode(in, 1024, dest, &len, 0);
        buffer.resize(len);
        
        std::cout << "libBase64 " << std::endl << buffer << " " << buffer.capacity()  << std::endl;
    }
    
    {
        std::string_view data(bts, 1024);
        std::string buffer =  base64::to_base64(data);
        
        std::cout << "base64 header " << std::endl << buffer << " " << buffer.capacity()  << std::endl;
    }
    
    {
        std::string_view data(bts, 1024);
        std::cout << "base64pp base64 " << used.count() << "ms " << base64pp::encode_str(data) << std::endl;
    }
    
    
    {
        std::string str(bts, 1024);
        std::cout << "mod base64 " << used.count() << "ms " << modp_b64_encode(str) << std::endl;
    }
    
   
   
    
    return 0;
}
