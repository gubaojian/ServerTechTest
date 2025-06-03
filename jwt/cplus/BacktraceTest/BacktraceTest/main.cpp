//
//  main.cpp
//  BacktraceTest
//
//  Created by efurture on 2025/6/3.
//

#include <iostream>
#include <thread>
#include <string>
#include "backward.hpp"

/**
 * https://github.com/bombela/backward-cpp
 */

void test1() {
    std::cout << "test1" << std::endl;
    std::string* s = nullptr;
    s->append("hello world");
}

void test() {
    test1();
}

int main(int argc, const char * argv[]) {
    // insert code here...
    std::cout << "Hello, World!\n";
    backward::SignalHandling handle;
    
    backward::StackTrace st; st.load_here(32);

    backward::TraceResolver tr;
    tr.load_stacktrace(st);
    for (size_t i = 0; i < st.size(); ++i) {
        backward::ResolvedTrace trace = tr.resolve(st[i]);
        std::cout << "#" << i
            << " " << trace.object_filename
            << " " << trace.object_function
            << " [" << trace.addr << "]"
            << " [" << trace.source.filename << "]"
            << " [" << trace.source.line << "]"
        << std::endl;
    }
    
    std::thread th([]{
        test();
    });
    th.join();
    
    return 0;
}
