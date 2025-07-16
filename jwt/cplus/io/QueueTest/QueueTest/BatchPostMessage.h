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
using OnBatchMessageAction = std::function<void(std::shared_ptr<std::vector<T>>&)>;


template <typename T>
class BatchPostMessage {
    public :
    explicit BatchPostMessage(size_t flushSize, bool multiProduce = false)
     : batchFlushSize(flushSize)
     , messages(std::make_shared<std::vector<T>>(flushSize))
    {
        messages->reserve(flushSize);
    };

public:
    void add(T&& t) {
        bool needFlush = false;
        {
            std::lock_guard<std::mutex> lock(messageMutex);
            messages->emplace_back(std::move(t));
            needFlush = messages->size() >= batchFlushSize;
        }
        if (needFlush) {
            flushForce();
        }
    }

    void flush() {
        bool needFlush = false;
        {
            std::lock_guard<std::mutex> lock(messageMutex);
            needFlush = messages->size() > 0;
        }
        flushForce();
    }

    void setOnBatchMessageAction(OnBatchMessageAction<T> action) {
        _onBatchMessageAction = action;
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
        std::shared_ptr<std::vector<T>> batchMessages = messages;
        {
            std::lock_guard<std::mutex> messageLock(messageMutex);
            std::lock_guard<std::mutex> poolLock(poolMutex);
            if (messages->empty()) {
                return;
            }
            if (!pools.empty()) {
                messages = pools.front();
                pools.pop();
            } else {
                messages = std::make_shared<std::vector<T>>();
            }
        }
        if (_onBatchMessageAction != nullptr && batchMessages->size() > 0) {
            _onBatchMessageAction(batchMessages);
        }
    }

private:
    size_t batchFlushSize = 128;
    OnBatchMessageAction<T> _onBatchMessageAction = nullptr;
    std::mutex messageMutex;
    std::shared_ptr<std::vector<T>> messages;
    std::mutex poolMutex;
    std::queue<std::shared_ptr<std::vector<T>>> pools;
};

#endif /* BatchPost_hpp */


