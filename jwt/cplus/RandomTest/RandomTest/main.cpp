//
//  main.cpp
//  RandomTest
//
//  Created by efurture on 2025/6/5.
//

#include <iostream>
#include <random>
#include <chrono>
#include <cstdlib>
/**
 * std::random_device: 1120 ms
 * std::mt19937: 96 ms
 * std::random_device: 113 ms
 * std::mt19937: 9 ms
 */
void test2() {
    const int N = 1000000;
        uint64_t sum = 0;

        // 1. 直接使用 std::random_device（最慢）
        auto start = std::chrono::high_resolution_clock::now();
        {
            std::random_device rd;
            for (int i = 0; i < N; ++i) {
                sum += rd();
            }
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::cout << "std::random_device: "
                  << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
                  << " ms" << std::endl;

        // 2. 使用 std::mt19937（快得多）
        start = std::chrono::high_resolution_clock::now();
        {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis;
            for (int i = 0; i < N; ++i) {
                sum += dis(gen);
            }
        }
        end = std::chrono::high_resolution_clock::now();
        std::cout << "std::mt19937: "
                  << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
                  << " ms" << std::endl;
}

int main() {
    const int N = 1000000;
    
    // 测试arc4random
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N; ++i) {
        arc4random();
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "arc4random: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
              << " ms" << std::endl;
    
    // 测试std::mt19937
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, UINT32_MAX);
    
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N; ++i) {
        dis(gen);
    }
    end = std::chrono::high_resolution_clock::now();
    std::cout << "std::mt19937: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
              << " ms" << std::endl;
    
    test2();
    
    return 0;
}
