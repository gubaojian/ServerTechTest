//
// Created by baojian on 25-7-17.
//
#include "string_view_in_big_heap_pool.h"

#include "big_heap_string_view_pool.h"

void StringViewInBigHeapPool::releaseStringViewInHeapPool() {
    if (pool != nullptr && !messageView.empty()) {
        pool->releaseStringViewInPool(*this);
        pool = nullptr;
    }
}

