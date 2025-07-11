//
//  main.cpp
//  uuidvsint64
//
//  Created by baojian on 2025/7/11.
//

#include <iostream>
#include <unordered_map>
#include <string>
#include <vector>
#include <random>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <cstdint>
#include <algorithm>

using namespace std;
using namespace std::chrono;

// 生成随机UUID字符串
string generate_uuid() {
    static random_device rd;
    static mt19937 gen(rd());
    static uniform_int_distribution<> dis(0, 15);
    static uniform_int_distribution<> dis2(8, 11);

    stringstream ss;
    int i;
    ss << hex;
    for (i = 0; i < 8; i++) {
        ss << dis(gen);
    }
    ss << '-';
    for (i = 0; i < 4; i++) {
        ss << dis(gen);
    }
    ss << '-';
    for (i = 0; i < 4; i++) {
        ss << dis(gen);
    }
    ss << '-';
    for (i = 0; i < 4; i++) {
        ss << dis2(gen);
    }
    ss << '-';
    for (i = 0; i < 12; i++) {
        ss << dis(gen);
    }
    return ss.str();
}

// 基准测试类
class Benchmark {
private:
    high_resolution_clock::time_point start_time;
    high_resolution_clock::time_point end_time;
    string test_name;

public:
    Benchmark(const string& name) : test_name(name) {
        start_time = high_resolution_clock::now();
    }

    ~Benchmark() {
        end_time = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(end_time - start_time).count();
        cout << left << setw(30) << test_name << ": " << duration << " ms" << endl;
    }
};

/**
 *
 *  int64校验和: 999000000
 *  int64在unordered_map中查找: 10 ms
 *  uuid string校验和: 999000000
 *  uuid string在unordered_map中查找: 33 ms
 */

int main() {
    const int DATA_SIZE = 1000;
    const int LOOKUP_TIMES = 2000000;

    // 准备测试数据
    vector<int64_t> int_keys;
    vector<string> uuid_keys;
    unordered_map<int64_t, int> int_unordered_map;
    unordered_map<string, int> uuid_unordered_map;

    // 生成随机int64键
    mt19937_64 rng(time(nullptr));
    for (int i = 0; i < DATA_SIZE; i++) {
        int64_t key = rng();
        int_keys.push_back(key);
        int_unordered_map[key] = i;
    }

    // 生成随机UUID键
    for (int i = 0; i < DATA_SIZE; i++) {
        string key = generate_uuid();
        uuid_keys.push_back(key);
        uuid_unordered_map[key] = i;
    }

    // 随机打乱查找顺序
    vector<int> lookup_indices;
    for (int i = 0; i < LOOKUP_TIMES; i++) {
        lookup_indices.push_back(i % DATA_SIZE);
    }
    shuffle(lookup_indices.begin(), lookup_indices.end(), mt19937(random_device()()));

    // 测试int64在unordered_map中的查找性能
    {
        Benchmark b("int64在unordered_map中查找");
        int sum = 0;
        for (int idx : lookup_indices) {
            auto it = int_unordered_map.find(int_keys[idx]);
            if (it != int_unordered_map.end()) {
                sum += it->second;
            }
        }
        cout << "int64校验和: " << sum << endl;
    }

    // 测试uuid string在unordered_map中的查找性能
    {
        Benchmark b("uuid string在unordered_map中查找");
        int sum = 0;
        for (int idx : lookup_indices) {
            auto it = uuid_unordered_map.find(uuid_keys[idx]);
            if (it != uuid_unordered_map.end()) {
                sum += it->second;
            }
        }
        cout << "uuid string校验和: " << sum << endl;
    }

    return 0;
}
