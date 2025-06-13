//
//  BatchPost.hpp
//  websocketpp
//
//  Created by baojian on 2025/6/11.
//

#ifndef BatchPost_hpp
#define BatchPost_hpp

#include <vector>
#include <memory>
#include <iostream>

#include <stdio.h>

template <typename T>
using OnBatchMessaage = std::function<void(std::shared_ptr<std::vector<T>>)>;


template <typename T>
class BatchPostMessage {
    public :
    explicit BatchPostMessage(size_t size)
     : batchSize(size)
     , messages(std::make_shared<std::vector<T>>(size))
    {
        
    };
    
public:
    void add(const T& t) {
        messages->push_back(t);
        flushIfOverBatchLimit();
    }
    
    void flush() {
        if (messages->size() > 0) {
            if (_onBatchMessaage != nullptr) {
                _onBatchMessaage(messages);
            }
            messages = std::make_shared<std::vector<T>>();
        }
    }
    
    void setOnBatchMessaage(OnBatchMessaage<T> action) {
        _onBatchMessaage = action;
    }
    
private:
    void flushIfOverBatchLimit() {
        if (messages->size() >= batchSize) {
            if (_onBatchMessaage != nullptr) {
                _onBatchMessaage(messages);
            }
            messages = std::make_shared<std::vector<T>>();
        }
    }
    
private:
    size_t batchSize = 128;
    OnBatchMessaage<T> _onBatchMessaage = nullptr;
    std::shared_ptr<std::vector<T>> messages;
};

#endif /* BatchPost_hpp */
