//
//  main.cpp
//  OutputInputTest
//
//  Created by baojian on 2025/6/10.
//

#include <iostream>
#include "output.h"
#include "input.h"

int main(int argc, const char * argv[]) {
    // insert code here...
    std::cout << "Hello, World!\n";
    char buffer[1024];
    
    Output output((uint8_t*)buffer, 0);
    Input input((uint8_t*)buffer, 0);
    
    output.writeShortStringUtf8("hello world");
    output.writeLongStringUtf8("你好");
    
    std::cout << input.readShortStringUtf8() << std::endl;
    
    std::cout << input.readLongStringUtf8() << std::endl;
    
    
    
    return 0;
}
