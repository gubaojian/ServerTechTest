//
//  BatchPostMessage.h
//  QueueTest
//
//  Created by baojian on 2025/7/16.
//

//
//  BatchPost.hpp
//  websocketpp
//
//  Created by baojian on 2025/6/11.
//

#ifndef BatchPostMessage_hpp
#define BatchPostMessage_hpp

#include <vector>
#include <memory>
#include <iostream>

#include <stdio.h>

template <typename T>
using OnBatchAction = std::function<void(const std::shared_ptr<std::vector<T>>&)>;


template <typename T>
class BatchQueue {
    public :
    explicit BatchQueue(size_t batchSize)
     : batchSize(batchSize)
     , elements(std::make_shared<std::vector<T>>(batchSize))
    {
        elements->reserve(batchSize);
    };

public:
    void add(T&& t) {
        bool needFlush = false;
        {
            std::lock_guard<std::mutex> lock(elementMutex);
            elements->emplace_back(std::move(t));
            needFlush = elements->size() >= batchSize;
        }
        if (needFlush) {
            flushForce();
        } else {
            if (flashTimerFunc != nullptr) {
                bool hasSetFlushTimer = hasSetFlushTimerFlag.test_and_set(std::memory_order_acquire);
                if (!hasSetFlushTimer) {
                    flashTimerFunc();
                }
            }
        }
    }

    void flush() {
        bool needFlush = false;
        {
            std::lock_guard<std::mutex> lock(elementMutex);
            needFlush = elements->size() > 0;
        }
        hasSetFlushTimerFlag.clear(std::memory_order_release);
        if (needFlush) {
            flushForce();
        }
    }

    void setOnBatchAction(OnBatchAction<T> action) {
        _onBatchAction = action;
    }
    
    void setAutoFlashTimerFunc(std::function<void()> && setFlashTimerFunc) {
        flashTimerFunc = std::move(setFlashTimerFunc);
    }
    void recycle(const std::shared_ptr<std::vector<T>>& elements) {
        elements->clear();
        {
            std::lock_guard<std::mutex> lock(poolMutex);
            if (pools.size() < 1024) {
                pools.push(elements);
            }
        }
    }

private:
    void flushForce() {
        hasSetFlushTimerFlag.clear(std::memory_order_release);
        std::shared_ptr<std::vector<T>> batchElements = nullptr;
        {
            std::lock_guard<std::mutex> messageLock(elementMutex);
            if (elements->empty()) {
                return;
            }
            batchElements = elements;
            {
                std::lock_guard<std::mutex> poolLock(poolMutex);
                if (!pools.empty()) {
                    elements = pools.front();
                    pools.pop();
                } else {
                    elements = std::make_shared<std::vector<T>>();
                }
            }
        }
        if (_onBatchAction != nullptr && batchElements->size() > 0) {
            _onBatchAction(batchElements);
        }
    }

private:
    size_t batchSize = 128;
    OnBatchAction<T> _onBatchAction = nullptr;
    std::mutex elementMutex;
    std::shared_ptr<std::vector<T>> elements;
    std::mutex poolMutex;
    std::queue<std::shared_ptr<std::vector<T>>> pools;
    std::atomic_flag hasSetFlushTimerFlag = ATOMIC_FLAG_INIT;
    std::function<void()> flashTimerFunc = nullptr;
};

#endif /* BatchPost_hpp */


