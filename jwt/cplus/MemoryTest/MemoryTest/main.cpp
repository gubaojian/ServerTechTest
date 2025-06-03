//
//  main.cpp
//  MemoryTest
//
//  Created by efurture on 2025/6/3.
//

#include <iostream>
#include <unistd.h>
#include <memory>
#include <queue>

int main(int argc, const char * argv[]) {
    // insert code here...
    std::queue<int> queue;
    for(int i=0; i<1024*1024*128; i++) {
        queue.push(i);
    }
    sleep(5);
    while (!queue.empty()) {
        queue.pop();
    }
    for(int i=0; i<1000; i++) {
        sleep(5);
    }
    
    return 0;
}
