//
//  main.cpp
//  StringTest
//
//  Created by efurture on 2025/6/4.
//

#include <iostream>
#include <string>
#include <cstring>
#include <chrono>
#include <vector>

int main() {
    // 测试不同长度的字符串
    std::vector<size_t> sizes = {10, 100, 1000, 10000, 100000};
    
    for (size_t size : sizes) {
        std::string src(size, 'a');
        char* src_cstr = new char[size + 1];
        std::memcpy(src_cstr, src.c_str(), size + 1);
        
        // 测试 std::string 赋值
        auto start = std::chrono::high_resolution_clock::now();
        std::string dst_str;
        for (int i = 0; i < 1000*1000; ++i) {
            dst_str = src;
        }
        auto end = std::chrono::high_resolution_clock::now();
        auto str_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        
        // 测试 std::memcpy
        start = std::chrono::high_resolution_clock::now();
        char* dst_cstr = new char[size + 1];
        for (int i = 0; i < 1000*1000; ++i) {
            std::memcpy(dst_cstr, src_cstr, size + 1);
        }
        end = std::chrono::high_resolution_clock::now();
        auto memcpy_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        
        std::cout << "Size: " << size << " bytes\n";
        std::cout << "std::string assignment: " << str_time << " microseconds\n";
        std::cout << "std::memcpy: " << memcpy_time << " microseconds\n";
        std::cout << "Ratio (string/memcpy): " << static_cast<double>(str_time) / memcpy_time << "\n\n";
        
        delete[] src_cstr;
        delete[] dst_cstr;
    }
    
    return 0;
}
