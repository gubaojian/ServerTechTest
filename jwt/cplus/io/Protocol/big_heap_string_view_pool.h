//
// Created by baojian on 25-7-17.
//

#ifndef BIG_HEAP_STRING_VIEW_POOL_H
#define BIG_HEAP_STRING_VIEW_POOL_H
#include <cstdint>
#include <iostream>
#include <mutex>



/**
 *  BigHeapStringViewPool implement by sliding window
  * put many small message in big preallocate heap array.
  * reduce memory alloc. first alloc, first return as queue
 * only support one thread alloc, one thread consume and then return pool return.
 * suitable for many small message less than 1kb
 * must be lock free, if not it will slow than std malloc。
 */
class BigHeapStringViewPool {
public:
    explicit BigHeapStringViewPool(size_t size) {
        buffer = new char[size + 8*1024]; //add padding to avoid check array bounds
        this->poolSize = size;
        allocOffset = 0;
        returnOffset = poolSize;
    }
    ~BigHeapStringViewPool() {
        if (buffer != nullptr) {
            delete[] buffer;
            buffer = nullptr;
        }
    }
public:
    std::string_view allocateStringViewInPool(const std::string& message) {
        return allocateStringViewInPool(message.data(), message.size());
    }

    std::string_view allocateStringViewInPool(const std::string_view& message) {
        return allocateStringViewInPool(message.data(), message.size());
    }

    std::string_view allocateStringViewInPool(const char* data, size_t length) {
        if (length > 4*1024) { //for bug message just alloc directly
            return std::string_view{};
        }
        totalGet++;
        int64_t remainSize = returnOffset - allocOffset;
        if (remainSize > length) {
            int64_t offset = allocOffset%poolSize;
            char* from = buffer + offset;
            memcpy(from, data, length);
            allocOffset += length;
            cacheHit++;
            return std::string_view{from, length};
        }
        return std::string_view{};
    }

    void deallocateStringViewInPool(const std::string_view& message) {
        if (!message.empty()) {
            //加入debug选项。
            if (message.data() < buffer || message.data() >= (buffer + poolSize + 8*1024)) {
                std::cout << "deallocateStringViewInPool call release message not in pool" << std::endl;
                return;
            }
            returnOffset += message.size();
            int64_t remainSize = returnOffset - allocOffset;
            if (remainSize > poolSize) { //illegal return, ignore it
                std::cout << "deallocateStringViewInPool call release message illegal" << std::endl;
                returnOffset -= message.size();
            }
        }
    }

    int64_t remainSpace() const {
        return returnOffset - allocOffset;
    }

    int64_t getTotalGet() const {
        return totalGet;
    }

    int64_t getCacheHit() const {
        return cacheHit;
    }

private:
    char* buffer = nullptr;
    int64_t allocOffset;
    int64_t returnOffset; //none need atomic, later see update is acceptable
    int64_t totalGet = 0;
    int64_t cacheHit = 0;
    int64_t poolSize = 0;
};




#endif //BIG_HEAP_STRING_VIEW_POOL_H
