//
//  main.cpp
//  StringPoolTest
//
//  Created by baojian on 2025/6/18.
//

#include <iostream>
#include <map>
#include <memory>
#include <thread>

/**
  * make ptr used 31ms
  *  pool make ptr used 15ms
  * lock
  * make ptr used 37ms
  * pool make ptr used 26ms
  */
class StringPool {
    public:
        StringPool(){};
        StringPool(const StringPool&) = delete;
        StringPool& operator=(const StringPool&) = delete;
    
    public:
        std::shared_ptr<std::string> getString(const std::string& key) {
            std::lock_guard<std::mutex> lock(mutex);
            auto it = poolMaps.find(key);
            if (it != poolMaps.end()) {
                return it->second;
            }
            poolMaps[key] = std::make_shared<std::string>(key);
            return poolMaps.at(key);
        }
    
    private:
        std::unordered_map<std::string, std::shared_ptr<std::string>> poolMaps;
        std::mutex mutex;
};

int main(int argc, const char * argv[]) {
    auto start = std::chrono::high_resolution_clock::now();
    auto end = std::chrono::high_resolution_clock::now();
    auto used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    
    std::string wsgId = "wsg_1109988348253";
    std::shared_ptr<std::string> ptr;
    start = std::chrono::high_resolution_clock::now();
    for(int i=0; i<10000*200; i++) {
        ptr = std::make_shared<std::string>(wsgId);
    }
    end = std::chrono::high_resolution_clock::now();
    used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "make ptr used " << used.count() << "ms" << std::endl;
    
    StringPool pool;
    start = std::chrono::high_resolution_clock::now();
    for(int i=0; i<10000*200; i++) {
        ptr = pool.getString(wsgId);
    }
    end = std::chrono::high_resolution_clock::now();
    used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "pool make ptr used " << used.count() << "ms" << std::endl;
    
    
    return 0;
}
