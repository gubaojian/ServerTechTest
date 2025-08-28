#include <iostream>


#include <stdio.h>
#include <stdlib.h>
#include <thread>
#include <uv.h>

#include "concurrentqueue.h"
#include "FastSwapQueue.h"
#include "BatchPostMessage.h"


struct UVTask {
    std::function<void()> func;
};

class UVTaskPool {
public:
    UVTaskPool() {
        swapTasks[0] = std::make_shared<std::vector<std::shared_ptr<UVTask>>>();
        swapTasks[1] = std::make_shared<std::vector<std::shared_ptr<UVTask>>>();
        swapTasks[0]->reserve(4096);
        swapTasks[1]->reserve(4096);
        stopFlag = false;
        hasInitFlag = false;
        loop = nullptr;
        autoCatchException = false;
        tasks = swapTasks[swapTaskIndex];
        loopThread = std::make_shared<std::thread>([this]() {
            runLoop();
        });
    }

    ~UVTaskPool() {
        stop();
    }

public:
    void post(std::function<void()> &&func) {
        if (!stopFlag) {
            std::shared_ptr<UVTask> task = std::make_shared<UVTask>(std::move(func));
            std::lock_guard<std::mutex> lock(tasksMutex);
            tasks->emplace_back(std::move(task));
        }
        {
            std::lock_guard<std::mutex> lock(stopMutex);
            if (hasInitFlag && loop != nullptr && !stopFlag) {
                uv_async_send(&taskAsync);
            }
        }
    }

    void setAutoCatchException(bool catchException) {
        this->autoCatchException = catchException;
    }

    void stop() {
        bool needJoin = false;
        if (!stopFlag) {
            std::lock_guard<std::mutex> lock(stopMutex);
            if (loop != nullptr && !stopFlag) {
                uv_async_send(&stopAsync);
                needJoin = true;
            }
        }
        if (needJoin) {
            if (loopThread->joinable()) {
                loopThread->join();
            }
        }
    }

private:
    // only can be called in uv loop thread
    void executeTasksInLoop() {
        std::shared_ptr<std::vector<std::shared_ptr<UVTask>>> executeTasks;
        {
            std::lock_guard<std::mutex> lock(tasksMutex);
            executeTasks = swapTasks[swapTaskIndex];
            swapTaskIndex = (swapTaskIndex + 1) % 2;
            tasks = swapTasks[swapTaskIndex];
        }
        for (auto &task: *executeTasks) {
            if (autoCatchException) {
                try {
                    task->func();
                } catch (std::exception &e) {
                    std::cerr << "[UVTaskPool] run task error " << e.what() << std::endl;
                } catch (...) {
                    std::cerr << "[UVTaskPool] run task unknown exception" << std::endl;
                }
            } else {
                task->func();
            }
        }
        if (executeTasks->size() >= 512*1024) { // max 4 mb
            executeTasks->resize(512*1024);
            executeTasks->shrink_to_fit();

        }
        executeTasks->clear();
    }

    void runLoop() {
        loop = uv_loop_new();
        if (loop == nullptr) {
            std::cerr << "[UVTaskPool] uv_loop_new failed: out of memory" << std::endl;
            stopFlag = true;
            hasInitFlag = false;
            return;
        }
        uv_async_init(loop, &stopAsync, [](uv_async_t *handle) {
            auto *pool = (UVTaskPool *) handle->data;
            bool needClose = false;
            {
                std::lock_guard<std::mutex> lock(pool->stopMutex);
                if (!pool->stopFlag) {
                    pool->stopFlag = true;
                    pool->hasInitFlag = false;
                    needClose = true;
                }
            }
            if (needClose) {
                pool->executeTasksInLoop();
                uv_close((uv_handle_t*)&pool->taskAsync, [](uv_handle_t* handle) {
                });
                uv_close((uv_handle_t*)&pool->stopAsync, [](uv_handle_t* handle) {
                });
                uv_stop(pool->loop);
            }
        });
        uv_async_init(loop, &taskAsync, [](uv_async_t *handle) {
             auto *pool = (UVTaskPool *) handle->data;
             pool->executeTasksInLoop();
        });
        stopAsync.data = this;
        taskAsync.data = this;
        hasInitFlag = true;
        while (!stopFlag) {
            uv_run(loop, UV_RUN_DEFAULT);
        }
        stopFlag = true;
        hasInitFlag = false;
        uv_loop_delete(loop);
        loop = nullptr;
    }

private:
    std::shared_ptr<std::thread> loopThread;
    std::atomic<bool> hasInitFlag = false;
    uv_loop_t *loop = nullptr;
    uv_async_t taskAsync{};
    std::mutex tasksMutex;
    std::shared_ptr<std::vector<std::shared_ptr<UVTask>>> tasks;
    std::shared_ptr<std::vector<std::shared_ptr<UVTask>>> swapTasks[2];
    int64_t swapTaskIndex = 0;
    uv_async_t stopAsync{};
    std::atomic<bool> stopFlag;
    std::mutex stopMutex;
    bool autoCatchException;
};



class UVTaskConcurrentPool {
public:
    UVTaskConcurrentPool() {
        tasks = std::make_shared<moodycamel::ConcurrentQueue<std::shared_ptr<UVTask>>>();
        stopFlag = false;
        hasInitFlag = false;
        loop = nullptr;
        autoCatchException = false;
        loopThread = std::make_shared<std::thread>([this]() {
            runLoop();
        });
    }

    ~UVTaskConcurrentPool() {
        stop();
    }

public:
    void post(std::function<void()> &&func) {
        if (!stopFlag) {
            std::shared_ptr<UVTask> task = std::make_shared<UVTask>(std::move(func));
            tasks->enqueue(std::move(task));
        }
        {
            std::lock_guard<std::mutex> lock(stopMutex);
            if (hasInitFlag && loop != nullptr && !stopFlag) {
                uv_async_send(&taskAsync);
            }
        }
    }

    void setAutoCatchException(bool catchException) {
        this->autoCatchException = catchException;
    }

    void stop() {
        bool needJoin = false;
        if (!stopFlag) {
            std::lock_guard<std::mutex> lock(stopMutex);
            if (loop != nullptr && !stopFlag) {
                uv_async_send(&stopAsync);
                needJoin = true;
            }
        }
        if (needJoin) {
            if (loopThread->joinable()) {
                loopThread->join();
            }
        }
    }

private:
    void executeTasksInLoop() {
        bool hasTask = false;
        do {
            std::shared_ptr<UVTask> task;
            hasTask = tasks->try_dequeue(task);
            if (hasTask) {
                if (autoCatchException) {
                    try {
                        task->func();
                    } catch (std::exception &e) {
                        std::cerr << "[UVTaskConcurrentPool] run task error " << e.what() << std::endl;
                    } catch (...) {
                        std::cerr << "[UVTaskConcurrentPool] run task unknown exception" << std::endl;
                    }
                } else {
                    task->func();
                }

            }
        } while (hasTask);
    }
    void runLoop() {
        loop = uv_loop_new();
        if (loop == nullptr) {
            std::cerr << "[UVTaskConcurrentPool] uv_loop_new failed: out of memory" << std::endl;
            stopFlag = true;
            hasInitFlag = false;
            return;
        }
        uv_async_init(loop, &stopAsync, [](uv_async_t *handle) {
            auto *pool = (UVTaskConcurrentPool *) handle->data;
            bool needClose = false;
            {
                std::lock_guard<std::mutex> lock(pool->stopMutex);
                if (!pool->stopFlag) {
                    pool->stopFlag = true;
                    pool->hasInitFlag = false;
                    needClose = true;
                }
            }
            if (needClose) {
                pool->executeTasksInLoop();
                uv_close((uv_handle_t*)&pool->taskAsync, [](uv_handle_t* handle) {
                });
                uv_close((uv_handle_t*)&pool->stopAsync, [](uv_handle_t* handle) {
                });
                uv_stop(pool->loop);
            }
        });
        uv_async_init(loop, &taskAsync, [](uv_async_t *handle) {
            auto *pool = (UVTaskConcurrentPool *) handle->data;
            pool->executeTasksInLoop();
        });
        stopAsync.data = this;
        taskAsync.data = this;
        hasInitFlag = true;
        while (!stopFlag) {
            uv_run(loop, UV_RUN_DEFAULT);
        }
        stopFlag = true;
        hasInitFlag = false;
        uv_loop_delete(loop);
        loop = nullptr;
    }

private:
    std::shared_ptr<std::thread> loopThread;
    std::atomic<bool> hasInitFlag = false;
    uv_loop_t *loop = nullptr;
    uv_async_t taskAsync{};
    std::shared_ptr<moodycamel::ConcurrentQueue<std::shared_ptr<UVTask>>> tasks;
    uv_async_t stopAsync{};
    std::atomic<bool> stopFlag;
    std::mutex stopMutex;
    bool autoCatchException;
};


int64_t executeCount = 0;
#define TASK_COUNT (1000*1000*2)

void testSwapQueue() {
    UVTaskPool pool;

    std::cout << "UVTaskPool pool start " << std::endl;
    uint64_t start_time, end_time;
    start_time = uv_hrtime();
    executeCount = 0;
    // 开始正式测试
    for (int i = 0; i < TASK_COUNT; i++) {
        pool.post([] {
            executeCount++;
        });
    }
    pool.stop();
    end_time = uv_hrtime();
    std::cout << "UVTaskPool pool used: " << ((end_time - start_time) / 1000000.0) << " ms" << std::endl;
    //std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    std::cout << "uv_queue_work pool done " << executeCount << std::endl;
}

void testSwapQueueTwoThread() {
    UVTaskPool pool;

    std::cout << "UVTaskPool two producer start " << std::endl;
    uint64_t start_time, end_time;
    start_time = uv_hrtime();
    executeCount = 0;
    std::thread twoThread([&] {
        for (int i = 0; i < TASK_COUNT/2; i++) {
            pool.post([] {
                executeCount++;
            });
        }
    });
    // 开始正式测试
    for (int i = 0; i < TASK_COUNT/2; i++) {
        pool.post([] {
            executeCount++;
        });
    }
    twoThread.join();
    pool.stop();
    end_time = uv_hrtime();
    std::cout << "UVTaskPool two producer used: " << ((end_time - start_time) / 1000000.0) << " ms" << std::endl;
    //std::this_thread::sleep_for(std::chrono::milliseconds(500));
    std::cout << "UVTaskPool two producer done " << executeCount << std::endl;
}

void testConcurrentQueue() {
    UVTaskConcurrentPool pool;

    std::cout << "UVTaskConcurrentPool pool start " << std::endl;
    uint64_t start_time, end_time;
    start_time = uv_hrtime();
    executeCount = 0;
    // 开始正式测试
    for (int i = 0; i < TASK_COUNT; i++) {
        pool.post([] {
            executeCount++;
        });
    }
    pool.stop();
    end_time = uv_hrtime();
    std::cout << "UVTaskConcurrentPool pool used: " << ((end_time - start_time) / 1000000.0) << " ms" << std::endl;
    //std::this_thread::sleep_for(std::chrono::milliseconds(500));
    std::cout << "UVTaskConcurrentPool pool done " << executeCount << std::endl;

}


void testConcurrentQueueTwo() {
    UVTaskConcurrentPool pool;

    std::cout << "UVTaskConcurrentPool two thread pool start " << std::endl;
    uint64_t start_time, end_time;
    start_time = uv_hrtime();
    executeCount = 0;
    // 开始正式测试
    std::thread twoThread([&] {
        for (int i = 0; i < TASK_COUNT/2; i++) {
            pool.post([] {
                executeCount++;
            });
        }
    });
    // 开始正式测试
    for (int i = 0; i < TASK_COUNT/2; i++) {
        pool.post([] {
            executeCount++;
        });
    }
    twoThread.join();
    pool.stop();
    end_time = uv_hrtime();
    std::cout << "UVTaskConcurrentPool two thread pool used: " << ((end_time - start_time) / 1000000.0) << " ms" << std::endl;
    std::cout << "UVTaskConcurrentPool two thread pool done " << executeCount << std::endl;

}


void test() {
    testSwapQueue();
    testConcurrentQueue();
    testSwapQueueTwoThread();
    testConcurrentQueueTwo();
}

size_t test_for_stable_count = 1024*1024*10*2;

void testForStable() {
    for (size_t i=0; i<test_for_stable_count; i++) {
        test();
    }
    std::thread test2([] {
        for (size_t i=0; i<test_for_stable_count; i++) {
            test();
        }
    });
    std::thread test3([] {
        for (size_t i=0; i<test_for_stable_count; i++) {
            test();
        }
    });

    test2.join();
    test3.join();
}
/**

UVTaskPool pool start
UVTaskPool pool used: 32.6434 ms
UVTaskConcurrentPool pool used: 130.991 ms
UVTaskConcurrentPool pool used: 72.3749 ms //去掉唤醒

lsof -p 8427 | wc -l
     853
ps aux | grep tasktest

 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char *argv[]) {

    test();
    testForStable();

    return 0;
}
