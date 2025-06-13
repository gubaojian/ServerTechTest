//
//  main.cpp
//  OutputInputTest
//
//  Created by baojian on 2025/6/10.
//

#include <iostream>
#include "output.h"
#include "input.h"


void test_normal_format() {
    char buffer[1024];
    
    Output output((uint8_t*)buffer, 0);
    Input input((uint8_t*)buffer, 0, 1024);
    
    output.writeShortStringUtf8("hello world");
    output.writeLongStringUtf8("你好");
    
    std::cout << "normal format " << output.getSize() << std::endl;
    
    std::cout << input.readShortStringUtf8() << std::endl;
    std::cout << input.readLongStringUtf8() << std::endl;
    
}

void test_invalid_format() {
    char buffer[1024];
    
    Output output((uint8_t*)buffer, 0);
    output.writeShortStringUtf8("hello world");
    output.writeLongStringUtf8("你好");
    
    //change invalid to bigger len;
    buffer[0] = 88;
    
    Input input((uint8_t*)buffer, 0, output.getSize());
   
    std::cout << "invalid format read will return empty std::string_view " << std::endl;
    
    std::cout << input.readShortStringUtf8() << std::endl;
    std::cout << input.readLongStringUtf8() << std::endl;
    
}

int main(int argc, const char * argv[]) {
    
    test_normal_format();
 
    test_invalid_format();
    
    return 0;
}
