//
//  main.cpp
//  StringReserve
//
//  Created by baojian on 2025/7/17.
//

#include <iostream>

int main(int argc, const char * argv[]) {
    // insert code here...
    std::cout << "Hello, World!\n";
    
    std::string data;
    data.reserve(1024);
    
    std::cout << data.capacity() << std::endl; //FastMessage
    
    return 0;
}
