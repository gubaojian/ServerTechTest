//
//  main.cpp
//  MethodCallTest
//
//  Created by efurture on 2025/6/6.
//

#include <iostream>
#include <map>
#include <string>
#include <chrono>

class MyClass {
public:
    void method1() const { /* 模拟方法调用 */ }
    void method2() const { /* 模拟方法调用 */ }
    void method3() const { /* 模拟方法调用 */ }
};

class Wrapper {
public:
    MyClass* getMember() { return &obj; }
private:
    MyClass obj;
};


/**
 版本1耗时: 283 毫秒
 版本2耗时: 162 毫秒
 性能提升: 42.7562%
 */

int main() {
    std::map<std::string, Wrapper> myMap;
    myMap["key"] = Wrapper();
    
    auto it = myMap.find("key");
    if (it == myMap.end()) return 1;
    
    const int iterations = 10000000;
    
    // 测试版本1
    auto start1 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        it->second.getMember()->method1();
        it->second.getMember()->method2();
        it->second.getMember()->method3();
    }
    auto end1 = std::chrono::high_resolution_clock::now();
    auto duration1 = std::chrono::duration_cast<std::chrono::milliseconds>(end1 - start1).count();
    
    // 测试版本2
    auto start2 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        auto a = it->second.getMember();
        a->method1();
        a->method2();
        a->method3();
    }
    auto end2 = std::chrono::high_resolution_clock::now();
    auto duration2 = std::chrono::duration_cast<std::chrono::milliseconds>(end2 - start2).count();
    
    std::cout << "版本1耗时: " << duration1 << " 毫秒" << std::endl;
    std::cout << "版本2耗时: " << duration2 << " 毫秒" << std::endl;
    std::cout << "性能提升: " << (100.0 * (duration1 - duration2) / duration1) << "%" << std::endl;
    
    return 0;
}
