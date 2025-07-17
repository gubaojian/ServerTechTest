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
  * make ptr used 30ms
 * pool make ptr used 9ms
 * make p3 ptr used 1ms
 * pool p4 make ptr used 8ms
  * 针对短内存字符串
  */
class SmallStringPool {
    public:
        SmallStringPool(int maxSize_):maxSize(maxSize_){
            stringTable = new std::shared_ptr<std::string>[maxSize];
        };
    
        ~SmallStringPool() {
            if (stringTable != nullptr) {
                delete [] stringTable;
                stringTable = nullptr;
            }
        }
        SmallStringPool(const SmallStringPool&) = delete;
        SmallStringPool& operator=(const SmallStringPool&) = delete;
    
    public:
        std::shared_ptr<std::string> getString(const std::string& key) {
            std::string_view view(key.data(), key.size());
            return getString(view);
        }
    
        std::shared_ptr<std::string> getString(const char* data, size_t length) {
            std::string_view view(data, length);
            return getString(view);
        }
       
        std::shared_ptr<std::string> getString(std::string_view& view) {
            size_t index = hash(view) % maxSize;
            std::shared_ptr<std::string> it = stringTable[index];
            if (it) {
                return it;
            }
            it = std::make_shared<std::string>(view);
            stringTable[index] = it;
            return it;
        }
      
    
    private:
        size_t maxSize;
        std::shared_ptr<std::string>* stringTable;
        std::hash<std::string_view> hash;
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
    
    SmallStringPool pool(1024);
    start = std::chrono::high_resolution_clock::now();
    for(int i=0; i<10000*200; i++) {
        ptr = pool.getString(wsgId.data(), wsgId.size());
    }
    end = std::chrono::high_resolution_clock::now();
    used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "pool make ptr used " << used.count() << "ms" << std::endl;
    
    
    std::string p3;
    start = std::chrono::high_resolution_clock::now();
    for(int i=0; i<10000*200; i++) {
        p3 = std::string(wsgId);
    }
    end = std::chrono::high_resolution_clock::now();
    used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "make p3 ptr used " << used.count() << "ms" << std::endl;
    
    std::string* p4 = nullptr;
    start = std::chrono::high_resolution_clock::now();
    for(int i=0; i<10000*200; i++) {
        p4 = pool.getString(wsgId.data(), wsgId.size()).get();
    }
    end = std::chrono::high_resolution_clock::now();
    used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "pool p4 make ptr used " << used.count() << "ms" << std::endl;
    
    
    return 0;
}
