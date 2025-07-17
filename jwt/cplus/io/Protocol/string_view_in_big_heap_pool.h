//
// Created by baojian on 25-7-17.
//

#ifndef STRING_VIEW_IN_BIG_HEAP_POOL_H
#define STRING_VIEW_IN_BIG_HEAP_POOL_H
#include <string>
#include <memory>
#include <string_view>

class BigHeapStringViewPool;

class StringViewInBigHeapPool {
public:
    explicit StringViewInBigHeapPool(const std::string_view& messageView, BigHeapStringViewPool* holderPool = nullptr) {
        this->pool = holderPool;
        if (holderPool != nullptr) {
            this->messageView = messageView;
        } else {
            if (!messageView.empty()) {
                unPoolStringViewHolder = std::make_shared<std::string>(messageView);
                this->messageView = *unPoolStringViewHolder;
            }
        }
    }
    void releaseStringViewInHeapPool();

    bool isInHeapPool() const {
        return pool != nullptr && messageView.empty();
    }

public:
    std::string_view messageView;
    std::shared_ptr<std::string> unPoolStringViewHolder = nullptr; //only valid
    BigHeapStringViewPool* pool = nullptr;
};


#endif //string_view_in_big_heap_pool.h
