//
// Created by baojian on 25-7-17.
//

#ifndef BIG_HEAP_STRING_VIEW_POOL_H
#define BIG_HEAP_STRING_VIEW_POOL_H
#include <cstdint>
#include <mutex>
#include "string_view_in_big_heap_pool.h"

/**
 *  BigHeapStringViewPool implement by sliding window
  * put many small message in big preallocate heap array. reduce memory alloc.
 * one thread alloc, one thread consume and then return pool return
 */
class BigHeapStringViewPool {
public:
    explicit BigHeapStringViewPool(size_t size) {
        buffer = new char[size + 4*1024]; //add padding to avoid check array bounds
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
    StringViewInBigHeapPool createStringViewInPool(const std::string& message) {
        return createStringViewInPool(message.data(), message.size());
    }

    StringViewInBigHeapPool createStringViewInPool(const char* data, size_t length) {
        if (length > 2*1024) {
            return StringViewInBigHeapPool({data, length}, nullptr);
        }
        std::lock_guard<std::mutex> lock(mutex);
        int64_t remainSize = returnOffset - allocOffset;
        int64_t offset = allocOffset%poolSize;
        if (remainSize > length) {
            char* from = buffer + offset;
            memcpy(from, data, length);
            allocOffset += length;
            return StringViewInBigHeapPool({from, length}, this);
        } else {
            return StringViewInBigHeapPool({data, length}, nullptr);
        }
    }
    void releaseStringViewInPool(const StringViewInBigHeapPool& message) {
        if (message.pool == this && !message.messageView.empty()) {
            std::lock_guard<std::mutex> lock(mutex);
            returnOffset += message.messageView.size();
            int64_t remainSize = returnOffset - allocOffset;
            if (remainSize > poolSize) { //illegal return, ignore it
                returnOffset -= message.messageView.size();
            }
            if (returnOffset > poolSize*1024) {
                returnOffset -= poolSize*512;
                allocOffset -= poolSize*512;
            }
        }
    }
    size_t remainSpace() const {
        return returnOffset - allocOffset;
    }
private:
    char* buffer = nullptr;
    std::mutex mutex;
    int64_t allocOffset;
    int64_t returnOffset;
    int64_t poolSize = 0;
};




#endif //SLIDING_WINDOW_STRING_VIEW_POOL_H
