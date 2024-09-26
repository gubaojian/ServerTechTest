//
//  string_memory_test.cc
//  cplus
//
//  Created by efurture on 2024/9/26.
//
#include <string>
#include <iostream>
#include <uuid/uuid.h>
#include <chrono>



/**
 * uuid first used 325727723ns 325ms
 *  uuid2 second used 332609046ns 332ms
 */
int string_memory_test_main(int argc, const char * argv[]) {
    std::string hello = "hello";
    std::string world = "hello world";
    
    std::cout << "hello " <<  " size " << hello.size() << " capcity " << hello.capacity() << std::endl;
    
    std::cout << "hello world " << " size " << world.size() << " capcity " << world.capacity() << std::endl;
    
    hello.shrink_to_fit();
    world.shrink_to_fit();
    
    
    std::cout << "after shrink to fit " << std::endl;
    
    std::cout << "hello " <<  " size " << hello.size() << " capcity " << hello.capacity() << std::endl;
    
    std::cout << "hello world " << " size " << world.size() << " capcity " << world.capacity() << std::endl;
    
    std::string str;
    
    auto start = std::chrono::high_resolution_clock::now();
    for(int i=0; i<10000*100; i++) {
        uuid_t uuid;
        uuid_generate_random(uuid);
        uuid_string_t uuidStr;
        uuid_unparse(uuid, uuidStr);
        str = uuidStr;
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto put_ns = end - start;
    auto put_ms = std::chrono::duration_cast<std::chrono::milliseconds>(put_ns);
    std::cout << "uuid first used " << put_ns.count() << "ns " << put_ms.count() << "ms" << std::endl;
    
    std::string str2;
    str2.resize(37);
    start = std::chrono::high_resolution_clock::now();
    for(int i=0; i<10000*100; i++) {
        uuid_t uuid;
        uuid_generate_random(uuid);
        char* uuidStr2 =  (char*)str2.c_str();
        uuid_unparse(uuid, uuidStr2);
    }
    end = std::chrono::high_resolution_clock::now();
    put_ns = end - start;
    put_ms = std::chrono::duration_cast<std::chrono::milliseconds>(put_ns);
    std::cout << "uuid2 second used " << put_ns.count() << "ns " << put_ms.count() << "ms" << std::endl;

    
    std::cout << "uuid " << str << " capcity " << str.capacity() << std::endl;
    
    std::cout << "uuid2 " << str2 << " capcity " << str.capacity() << std::endl;
    
    
    
    return 0;
}
