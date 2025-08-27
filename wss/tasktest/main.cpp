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
        stopFlag = false;
        hasInitFlag = false;
        loop = nullptr;
        swapTasks[0] = std::make_shared<std::vector<UVTask> >();
        swapTasks[1] = std::make_shared<std::vector<UVTask> >();
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
        {
            UVTask task(std::move(func));
            std::lock_guard<std::mutex> lock(tasksMutex);
            tasks->emplace_back(task);
        }
        {
            std::lock_guard<std::mutex> lock(stopMutex);
            if (hasInitFlag && loop != nullptr && !stopFlag) {
                uv_async_send(&taskAsync);
            }
        }
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
    void runLoop() {
        loop = uv_loop_new();
        if (loop == nullptr) {
            std::cerr << "[UVTaskPool] uv_loop_new failed: out of memory" << std::endl;
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
                uv_stop(pool->loop);
                uv_close((uv_handle_t*)&pool->stopAsync, [](uv_handle_t* handle) {
                    auto *pool = (UVTaskPool *) handle->data;
                    uv_close((uv_handle_t*)&pool->taskAsync, [](uv_handle_t* handle) {
                          auto *pool = (UVTaskPool *) handle->data;
                           uv_loop_close(pool->loop);
                           free(pool->loop);
                           pool->loop = nullptr;
                    });
               });
            }
        });
        uv_async_init(loop, &taskAsync, [](uv_async_t *handle) {
            auto *pool = (UVTaskPool *) handle->data;
            std::shared_ptr<std::vector<UVTask>> executeTasks;
            {
                std::lock_guard<std::mutex> lock(pool->tasksMutex);
                executeTasks = pool->swapTasks[pool->swapTaskIndex];
                pool->swapTaskIndex = (pool->swapTaskIndex + 1) % 2;
                pool->tasks = pool->swapTasks[pool->swapTaskIndex];
            }
            for (auto &task: *executeTasks) {
                task.func();
            }
            executeTasks->clear();
        });
        stopAsync.data = this;
        taskAsync.data = this;
        hasInitFlag = true;
        while (!stopFlag) {
            uv_run(loop, UV_RUN_DEFAULT);
        }
        stopFlag = true;
        hasInitFlag = false;
    }

private:
    std::shared_ptr<std::thread> loopThread;
    std::atomic<bool> hasInitFlag = false;
    uv_loop_t *loop = nullptr;
    uv_async_t taskAsync{};
    std::mutex tasksMutex;
    std::shared_ptr<std::vector<UVTask>> tasks;
    std::shared_ptr<std::vector<UVTask>> swapTasks[2];
    int64_t swapTaskIndex = 0;
    uv_async_t stopAsync{};
    std::atomic<bool> stopFlag;
    std::mutex stopMutex;
};



class UVTaskConcurrentPool {
public:
    UVTaskConcurrentPool() {
        stopFlag = false;
        hasInitFlag = false;
        loop = nullptr;
        tasks = std::make_shared<moodycamel::ConcurrentQueue<UVTask> >();
        loopThread = std::make_shared<std::thread>([this]() {
            runLoop();
        });
    }

    ~UVTaskConcurrentPool() {
        stop();
    }

public:
    void post(std::function<void()> &&func) {
        {
            UVTask task(std::move(func));
            tasks->enqueue(task);
        }
        {
            std::lock_guard<std::mutex> lock(stopMutex);
            if (hasInitFlag && loop != nullptr && !stopFlag) {
                uv_async_send(&taskAsync);
            }
        }
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
    void runLoop() {
        loop = uv_loop_new();
        if (loop == nullptr) {
            std::cerr << "[UVTaskConcurrentPool] uv_loop_new failed: out of memory" << std::endl;
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
                uv_stop(pool->loop);
                uv_close((uv_handle_t*)&pool->stopAsync, [](uv_handle_t* handle) {
                    auto *pool = (UVTaskConcurrentPool *) handle->data;
                    uv_close((uv_handle_t*)&pool->taskAsync, [](uv_handle_t* handle) {
                           auto *pool = (UVTaskConcurrentPool *) handle->data;
                           uv_loop_close(pool->loop);
                           free(pool->loop);
                           pool->loop = nullptr;
                    });
                });
            }
        });
        uv_async_init(loop, &taskAsync, [](uv_async_t *handle) {
            auto *pool = (UVTaskConcurrentPool *) handle->data;
            bool hasTask = false;
            do {
                 UVTask task;
                 hasTask = pool->tasks->try_dequeue(task);
                 if (hasTask) {
                     task.func();
                 }
            } while (hasTask);
        });
        stopAsync.data = this;
        taskAsync.data = this;
        hasInitFlag = true;
        while (!stopFlag) {
            uv_run(loop, UV_RUN_DEFAULT);
        }
        stopFlag = true;
        hasInitFlag = false;
    }

private:
    std::shared_ptr<std::thread> loopThread;
    std::atomic<bool> hasInitFlag = false;
    uv_loop_t *loop = nullptr;
    uv_async_t taskAsync{};
    std::shared_ptr<moodycamel::ConcurrentQueue<UVTask> > tasks;
    uv_async_t stopAsync{};
    std::atomic<bool> stopFlag;
    std::mutex stopMutex;
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

size_t test_for_stable_count = 1024;

void testForStable() {
    for (int i=0; i<test_for_stable_count; i++) {
        test();
    }
    std::thread test2([] {
        for (int i=0; i<test_for_stable_count; i++) {
            test();
        }
    });
    std::thread test3([] {
        for (int i=0; i<test_for_stable_count; i++) {
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
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char *argv[]) {

    test();
    testForStable();

    return 0;
}
