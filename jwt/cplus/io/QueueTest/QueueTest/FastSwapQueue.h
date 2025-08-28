//
// Created by baojian on 25-7-16.
//

#ifndef FASTSWAPQUEUE_H
#define FASTSWAPQUEUE_H
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>

struct FastSwapQueueTask {
    std::function<void()> func = nullptr;
};

class FastSwapQueue {
public:
    FastSwapQueue(bool multiConsumer = false) {
        this->multiConsumer = multiConsumer;
        swapTasks[0] = std::make_shared<std::vector<std::shared_ptr<FastSwapQueueTask>>>();
        swapTasks[1] = std::make_shared<std::vector<std::shared_ptr<FastSwapQueueTask>>>();
        swapTasks[0]->reserve(4096);
        swapTasks[1]->reserve(4096);
        tasks = swapTasks[swapTaskIndex];
    }
public:
    void setConsumeTasksFunc(std::function<void()> &&asyncFunc) {
        consumeFunc = std::move(asyncFunc);
    }
public:
    void post(std::function<void()> &&func) {
        {
            std::shared_ptr<FastSwapQueueTask> task = std::make_shared<FastSwapQueueTask>(std::move(func));
            std::lock_guard<std::mutex> lock(tasksMutex);
            tasks->emplace_back(std::move(task));
        }
        if (consumeFunc != nullptr) {
            bool needCallConsume = hasWeakUpFlag.test_and_set(std::memory_order_acquire);
            if (!needCallConsume) {
                consumeFunc();
            }
        }
    }
public:
    void consumeTasks() {
        auto tasks = getConsumeTasks();
        for (auto& task: *tasks) {
            task->func();
        }
        if (tasks->size() >= 512*1024) { // max 4 mb
            tasks->resize(512*1024);
            tasks->shrink_to_fit();
        }
        tasks->clear();
        if (multiConsumer) {
            std::lock_guard<std::mutex> lock(poolMutex);
            if (pools.size() < 1024) {
                pools.push(tasks);
            }
        }
    }
private:
    std::shared_ptr<std::vector<std::shared_ptr<FastSwapQueueTask> >> getConsumeTasks() {
        std::shared_ptr<std::vector<std::shared_ptr<FastSwapQueueTask>>> consumeTasks;
        {
            hasWeakUpFlag.clear(std::memory_order_release);
            std::lock_guard<std::mutex> lock(tasksMutex);
            consumeTasks = swapTasks[swapTaskIndex];
            if (multiConsumer) {
                std::lock_guard<std::mutex> lock(poolMutex);
                if (pools.empty()) {
                    swapTasks[swapTaskIndex] = std::make_shared<std::vector<std::shared_ptr<FastSwapQueueTask>>>();
                } else {
                    swapTasks[swapTaskIndex] = pools.front();
                    pools.pop();
                }
            }
            swapTaskIndex = (swapTaskIndex + 1) % 2;
            tasks = swapTasks[swapTaskIndex];
        }
        return consumeTasks;
    }
    
private:
    std::function<void()> consumeFunc = nullptr;
    std::mutex tasksMutex;
    std::atomic_flag hasWeakUpFlag = ATOMIC_FLAG_INIT;
    std::shared_ptr<std::vector<std::shared_ptr<FastSwapQueueTask>>> tasks;
    std::shared_ptr<std::vector<std::shared_ptr<FastSwapQueueTask>>> swapTasks[2];
    std::mutex poolMutex;
    std::queue<std::shared_ptr<std::vector<std::shared_ptr<FastSwapQueueTask>>>> pools;
    int32_t swapTaskIndex = 0;
    bool multiConsumer = false; //是否多线程消费
};


#endif //FASTSWAPQUEUE_H
