#include <iostream>
#include <vector>
#include <unordered_map>

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

struct UVTimerTask {
    std::function<void()> func;
    int64_t timerId{};
    uv_timer_t* uvTimer{};
    size_t delayMS{};  //first fire call time
    size_t intervalMS{}; //repeat time, 0 means no repeat
    void* pool{};
};

class UVTaskPool {
public:
    UVTaskPool() {
        swapTasks[0] = std::make_shared<std::vector<std::shared_ptr<UVTask>>>();
        swapTasks[1] = std::make_shared<std::vector<std::shared_ptr<UVTask>>>();
        swapTasks[0]->reserve(4096);
        swapTasks[1]->reserve(4096);
        timerTasksMapInLoop = std::make_shared<std::unordered_map<int64_t, std::shared_ptr<UVTimerTask>>>();
        loopStopFlag = false;
        hasLoopInitFlag = false;
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
        if (!loopStopFlag) {
            std::shared_ptr<UVTask> task = std::make_shared<UVTask>(std::move(func));
            std::lock_guard<std::mutex> lock(tasksMutex);
            tasks->emplace_back(std::move(task));
        }
        {
            std::lock_guard<std::mutex> lock(stopMutex);
            if (hasLoopInitFlag && loop != nullptr && !loopStopFlag) {
                uv_async_send(&taskAsync);
            }
        }
    }

    std::shared_ptr<UVTimerTask> createTimer(std::function<void()> &&func, size_t delayMS, size_t intervalMS) {
        std::shared_ptr<UVTimerTask> timerTask = std::make_shared<UVTimerTask>();
        timerTask->func = std::move(func);
        timerTask->timerId = autoGenTimerId.fetch_add(1, std::memory_order_relaxed);
        timerTask->delayMS = delayMS;
        timerTask->intervalMS = intervalMS;
        timerTask->uvTimer = nullptr;
        timerTask->pool = this;
        post([this, timerTask]() {
           this->createTimerInLoop(timerTask);
        });
        return timerTask;
    }

    void cancelTimer(const std::shared_ptr<UVTimerTask>& timerTask) {
        post([this, timerTask]() {
             this->cancelTimerInLoop(timerTask->timerId);
        });
    }

    void cancelTimerById(const int64_t timerId) {
        post([this, timerId]() {
             this->cancelTimerInLoop(timerId);
        });
    }

    void setAutoCatchException(bool catchException) {
        this->autoCatchException = catchException;
    }

    void stop() {
        bool needJoin = false;
        if (!loopStopFlag) {
            //等待线程启动完成初始化。
            while (!hasLoopInitFlag && !loopStopFlag) {
                std::this_thread::sleep_for(std::chrono::milliseconds(30));
            }
            std::lock_guard<std::mutex> lock(stopMutex);
            if (loop != nullptr && !loopStopFlag) {
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

    // InLoop function can only can be called in uv loop thread
private:
    void createTimerInLoop(const std::shared_ptr<UVTimerTask>& timerTask) const {
        auto *uvTimer = new uv_timer_t();
        timerTask->uvTimer = uvTimer;
        timerTask->uvTimer->data = timerTask.get();
        uv_timer_init(loop, timerTask->uvTimer);
        (*timerTasksMapInLoop)[timerTask->timerId] = timerTask;
        uv_timer_start(uvTimer, [](uv_timer_t* handle) {
            auto* task = (UVTimerTask*)handle->data;
            auto *pool = (UVTaskPool *) task->pool;
            pool->onTimerInLoop(task);

        }, timerTask->delayMS, timerTask->intervalMS);
    }

    void cancelTimerInLoop(const int64_t timerId) const {
        auto findId = timerTasksMapInLoop->find(timerId);
        if (findId == timerTasksMapInLoop->end()) {
            return;
        }
        const std::shared_ptr<UVTimerTask>& timerTask = findId->second;
        if (timerTask->uvTimer != nullptr) {
            uv_close((uv_handle_t*)timerTask->uvTimer, [](uv_handle_t* handle) {
                delete handle;
            });
            timerTask->uvTimer = nullptr;
        }
        timerTasksMapInLoop->erase(findId);
    }

    void cleanAllTimersInLoop() {
        std::for_each(timerTasksMapInLoop->begin(), timerTasksMapInLoop->end(),[](auto& mapIt) {
            const std::shared_ptr<UVTimerTask>& timerTask = mapIt.second;
            if (timerTask->uvTimer != nullptr) {
             uv_close((uv_handle_t*)timerTask->uvTimer, [](uv_handle_t* handle) {
                 delete handle;
             });
             timerTask->uvTimer = nullptr;
           }
        });
        timerTasksMapInLoop->clear();
    }

    void onTimerInLoop(const UVTimerTask* task) const {
        if (autoCatchException) {
            try {
                task->func();
            } catch (std::exception &e) {
                std::cerr << "[UVTaskPool] run timer task error " << e.what() << std::endl;
            } catch (...) {
                std::cerr << "[UVTaskPool] run timer task unknown exception" << std::endl;
            }
        } else {
            task->func();
        }
        if (task->intervalMS <= 0) {
            cancelTimerInLoop(task->timerId);
        }
    }
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
            loopStopFlag = true;
            hasLoopInitFlag = false;
            return;
        }
        uv_async_init(loop, &stopAsync, [](uv_async_t *handle) {
            auto *pool = (UVTaskPool *) handle->data;
            bool needClose = false;
            {
                std::lock_guard<std::mutex> lock(pool->stopMutex);
                if (!pool->loopStopFlag) {
                    pool->loopStopFlag = true;
                    pool->hasLoopInitFlag = false;
                    needClose = true;
                }
            }
            if (needClose) {
                pool->executeTasksInLoop();
                pool->cleanAllTimersInLoop();
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
        hasLoopInitFlag = true;
        while (!loopStopFlag) {
            uv_run(loop, UV_RUN_DEFAULT);
        }
        loopStopFlag = true;
        hasLoopInitFlag = false;
        uv_loop_delete(loop);
        loop = nullptr;
    }

private:
    std::shared_ptr<std::thread> loopThread;
    uv_loop_t *loop = nullptr;
    std::atomic<bool> loopStopFlag;
    std::atomic<bool> hasLoopInitFlag = false;
    uv_async_t taskAsync{};
    std::mutex tasksMutex;
    uv_async_t stopAsync{};
    std::mutex stopMutex;
    std::shared_ptr<std::vector<std::shared_ptr<UVTask>>> tasks;
    std::shared_ptr<std::vector<std::shared_ptr<UVTask>>> swapTasks[2];
    int64_t swapTaskIndex = 0;
    bool autoCatchException;
    std::atomic<int64_t> autoGenTimerId = ATOMIC_VAR_INIT(0);
    //only be used in loop thread
    std::shared_ptr<std::unordered_map<int64_t, std::shared_ptr<UVTimerTask>>> timerTasksMapInLoop;
};



class UVTaskConcurrentPool {
public:
    UVTaskConcurrentPool() {
        tasks = std::make_shared<moodycamel::ConcurrentQueue<std::shared_ptr<UVTask>>>();
        loopStopFlag = false;
        hasLoopInitFlag = false;
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
        if (!loopStopFlag) {
            std::shared_ptr<UVTask> task = std::make_shared<UVTask>(std::move(func));
            tasks->enqueue(std::move(task));
        }
        {
            std::lock_guard<std::mutex> lock(stopMutex);
            if (hasLoopInitFlag && loop != nullptr && !loopStopFlag) {
                uv_async_send(&taskAsync);
            }
        }
    }

    void setAutoCatchException(bool catchException) {
        this->autoCatchException = catchException;
    }

    void stop() {
        bool needJoin = false;
        if (!loopStopFlag) {
            while (!hasLoopInitFlag && !loopStopFlag) {
                std::this_thread::sleep_for(std::chrono::milliseconds(30));
            }
            std::lock_guard<std::mutex> lock(stopMutex);
            if (loop != nullptr && !loopStopFlag) {
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
    void executeTasksInLoop() const {
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
            loopStopFlag = true;
            hasLoopInitFlag = false;
            return;
        }
        uv_async_init(loop, &stopAsync, [](uv_async_t *handle) {
            auto *pool = (UVTaskConcurrentPool *) handle->data;
            bool needClose = false;
            {
                std::lock_guard<std::mutex> lock(pool->stopMutex);
                if (!pool->loopStopFlag) {
                    pool->loopStopFlag = true;
                    pool->hasLoopInitFlag = false;
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
        hasLoopInitFlag = true;
        while (!loopStopFlag) {
            uv_run(loop, UV_RUN_DEFAULT);
        }
        loopStopFlag = true;
        hasLoopInitFlag = false;
        uv_loop_delete(loop);
        loop = nullptr;
    }

private:
    std::shared_ptr<std::thread> loopThread;
    std::atomic<bool> hasLoopInitFlag = false;
    uv_loop_t *loop = nullptr;
    uv_async_t taskAsync{};
    std::shared_ptr<moodycamel::ConcurrentQueue<std::shared_ptr<UVTask>>> tasks;
    uv_async_t stopAsync{};
    std::atomic<bool> loopStopFlag;
    std::mutex stopMutex;
    bool autoCatchException;
};


int64_t executeCount = 0;
#define TASK_COUNT (1000*1000*2)

#define TIMER_TASK_COUNT (1000*10)

void testSwapQueue() {
    {
        UVTaskPool pool;
        // 2. 创建延迟 200ms 的定时器（触发时 pool 已销毁）
        pool.post([] {
            std::cout << "UVTaskPool pool create then stop now " << std::endl;
        });
    }
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
    {
        UVTaskConcurrentPool pool;
        // 2. 创建延迟 200ms 的定时器（触发时 pool 已销毁）
        pool.post([] {
            std::cout << "UVTaskConcurrentPool pool create then stop now " << std::endl;
        });
    }
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


void testUVTaskPoolTimer() {
    {
        UVTaskPool pool;
        // 2. 创建延迟 200ms 的定时器（触发时 pool 已销毁）
        pool.createTimer([] {
            std::cout << "定时器触发" << std::endl;
        }, 200, 0);
    }
    UVTaskPool pool;

    std::cout << "UVTaskPool Timer pool start " << std::endl;
    uint64_t start_time, end_time;
    start_time = uv_hrtime();
    executeCount = 0;
    // 开始正式测试
    for (int i = 0; i < TIMER_TASK_COUNT; i++) {
        pool.createTimer([] {
            executeCount++;
        }, 30, 0);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    std::cout << "UVTaskPool timer call stop " << std::endl;
    pool.stop();
    end_time = uv_hrtime();
    std::cout << "UVTaskPool timer pool used: " << ((end_time - start_time) / 1000000.0) << " ms" << std::endl;
    std::cout << "UVTaskPool timer pool done " << executeCount << std::endl;
}


void testIntervalUVTaskPoolTimer() {
    UVTaskPool pool;

    std::cout << "UVTaskPool Interval Timer pool start " << std::endl;
    uint64_t start_time, end_time;
    start_time = uv_hrtime();
    executeCount = 0;
    // 开始正式测试
    for (int i = 0; i < 4096*128; i++) {
        pool.createTimer([] {
            executeCount++;
        }, 30, 30);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    std::cout << "UVTaskPool Interval Timer call stop " << std::endl;
    pool.stop();
    end_time = uv_hrtime();
    std::cout << "UVTaskPool Interval Timer pool used: " << ((end_time - start_time) / 1000000.0) << " ms" << std::endl;
    std::cout << "UVTaskPool Interval Timer pool done " << executeCount << std::endl;
}


void test() {
    testSwapQueue();
    testConcurrentQueue();
    testSwapQueueTwoThread();
    testConcurrentQueueTwo();
    testUVTaskPoolTimer();
    testIntervalUVTaskPoolTimer();
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
