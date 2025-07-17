//
// Created by baojian on 25-7-17.
//

#ifndef STRING_VIEW_IN_BIG_HEAP_POOL_H
#define STRING_VIEW_IN_BIG_HEAP_POOL_H
#include <iostream>
#include <string>
#include <memory>
#include <string_view>
#include <__ostream/basic_ostream.h>

class BigHeapStringViewPool;

class StringViewInBigHeapPool {
public:
    explicit StringViewInBigHeapPool(const std::string_view& messageView, bool inPool = false) {
        this->inPool = inPool;
        if (inPool) {
            this->messageView = messageView;
        } else {
            if (!messageView.empty()) {
                unPoolStringViewHolder = std::make_shared<std::string>(messageView);
                this->messageView = *unPoolStringViewHolder;
            }
        }
    }


public:
    std::string_view messageView;
    std::shared_ptr<std::string> unPoolStringViewHolder = nullptr; //only valid
    bool inPool; //不要用指向指针，要用基本类型，防止移动构造不稳定。
};


#endif //string_view_in_big_heap_pool.h
