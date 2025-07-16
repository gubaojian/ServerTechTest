#include <iostream>


#include <stdio.h>
#include <stdlib.h>
#include <thread>
#include <uv.h>


struct UVTask {
    std::function<void()> func;
};

class UVTaskPool {
public:
    UVTaskPool() {
        stopFlag = false;
        stopAsync.data = this;
        taskAsync.data = this;
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
            UVTask task(func);
            std::lock_guard<std::mutex> lock(tasksMutex);
            tasks->emplace_back(task);
        }
        if (loop != nullptr
            && !stopFlag) {
            bool needAsyncSend = hasWeakUpFlag.test_and_set(std::memory_order_acquire);
            if (!needAsyncSend) {
                uv_async_send(&taskAsync);
            }
        }
    }

    void stop() {
        if (loop != nullptr) {
            uv_async_send(&stopAsync);
            if (loopThread->joinable()) {
                loopThread->join();
            }
        }
    }

private:
    void runLoop() {
        loop = uv_loop_new();
        uv_async_init(loop, &stopAsync, [](uv_async_t *handle) {
            auto *pool = (UVTaskPool *) handle->data;
            if (!pool->stopFlag) {
                pool->stopFlag = true;
                uv_stop(pool->loop);
                uv_loop_close(pool->loop);
                free(pool->loop);
                pool->loop = nullptr;
            }
        });
        uv_async_init(loop, &taskAsync, [](uv_async_t *handle) {
            auto *pool = (UVTaskPool *) handle->data;
            pool->hasWeakUpFlag.clear(std::memory_order_release);
            std::shared_ptr<std::vector<UVTask> > executeTasks;
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

        while (!stopFlag) {
            uv_run(loop, UV_RUN_DEFAULT);
        }
        loop = nullptr;
    }

private:
    std::shared_ptr<std::thread> loopThread;
    uv_loop_t *loop = nullptr;
    uv_async_t taskAsync{};
    std::mutex tasksMutex;
    std::atomic_flag hasWeakUpFlag = ATOMIC_FLAG_INIT;
    std::shared_ptr<std::vector<UVTask> > tasks;
    std::shared_ptr<std::vector<UVTask> > swapTasks[2];
    int64_t swapTaskIndex = 0;
    uv_async_t stopAsync{};
    std::atomic<bool> stopFlag;
};


int64_t executeCount = 0;
#define TASK_COUNT (1000*1000*2)

int main(int argc, char *argv[]) {
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
    end_time = uv_hrtime();
    std::cout << "UVTaskPool pool used: " << ((end_time - start_time) / 1000000.0) << " ms" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    std::cout << "uv_queue_work pool done " << executeCount << std::endl;
    pool.stop();


    return 0;
}
