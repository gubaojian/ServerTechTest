#include <iostream>


#include <stdio.h>
#include <stdlib.h>
#include <thread>
#include <uv.h>


#define TASK_COUNT 1000000  // 测试100万次调用

uv_loop_t *loop;
uv_mutex_t mutex;

double total_call_time = 0;  // 总调用时间 (ms)
double min_call_time = 9999; // 最小调用时间
double max_call_time = 0;    // 最大调用时间

// 空的工作回调（只用于测试提交耗时）
void empty_work_cb(uv_work_t *req) {
    // 不执行任何操作
}

// 空的完成回调
void empty_after_work_cb(uv_work_t *req, int status) {
    // 不执行任何操作
    free(req);
}

struct UVTask {
    std::function<void()> func;
};

class UVTaskPool {
 public:
       UVTaskPool() {
           loop = uv_loop_new();
           uv_async_init(loop, &stop_async, [](uv_async_t* handle) {
                auto* pool = static_cast<UVTaskPool*>(handle->data);
                pool->stop_flag = true;
                uv_stop(pool->loop);
                std::cout << "uv_async_init end: " << std::endl;
           });
           stop_async.data = this;
           loopThread = std::make_shared<std::thread>([this]() {
               runLoop();
           });
       }
       ~UVTaskPool() {
          stop();
       }
public:
    void post(std::function<void()>&& func) const {
        uv_work_t* req = (uv_work_t*)malloc(sizeof(uv_work_t));
        req->data = new UVTask(std::move(func));
        uv_queue_work(loop, req, [](uv_work_t* req) {
            const UVTask* task = (UVTask*)req->data;
            task->func();
        }, [](uv_work_t *req, int status) {
            const UVTask* task = (UVTask*)req->data;
             //req->data = nullptr;
             free(req);
             delete task;
        });
    }

    void stop() {
       // uv_async_send(&stop_async);
        // 等待线程结束
        if (loopThread->joinable()) {
            loopThread->join();
        }
        uv_close((uv_handle_t*)&stop_async, nullptr);
    }
private:
    void runLoop() const {
        uv_timer_t timer;
        uv_timer_init(loop, &timer);

        // 设置定时器：立即启动，每 1000ms 触发一次
        uv_timer_start(&timer,[](uv_timer_t* handle) {

        }, 0, 1000);
        uv_run(loop, UV_RUN_DEFAULT);
        uv_timer_stop(&timer);
        uv_loop_close(loop);
        free(loop);
    }
  private:
    std::shared_ptr<std::thread> loopThread;
    uv_loop_t *loop = nullptr;
    uv_async_t stop_async;
    std::atomic<bool> stop_flag{false};
};

void testNormal() {
    std::cout << "uv_queue_work start_time: " << std::endl;
    loop = uv_loop_new();
    std::cout << "uv_queue_work start_time: 33"  << std::endl;

    uint64_t start_time, end_time;

    // 预热：执行少量调用以让系统进入稳定状态
    for (int i = 0; i < 1000; i++) {
        uv_work_t* req = (uv_work_t*)malloc(sizeof(uv_work_t));
        uv_queue_work(loop, req, empty_work_cb, empty_after_work_cb);
    }
    std::cout << "uv_queue_work start_time: " << start_time << " ms" << std::endl;

    uv_run(loop, UV_RUN_DEFAULT);
    std::cout << "uv_queue_work start_time: " << start_time << " ms" << std::endl;
    start_time = uv_hrtime();
    // 开始正式测试
    for (int i = 0; i < TASK_COUNT; i++) {
        uv_work_t* req = (uv_work_t*)malloc(sizeof(uv_work_t));
        uv_queue_work(loop, req, empty_work_cb, empty_after_work_cb);
    }
    end_time = uv_hrtime();

    total_call_time = (end_time - start_time)/1000000.0;;

    // 执行所有已提交的任务
    uv_run(loop, UV_RUN_DEFAULT);


    // 计算平均值
    double avg_call_time = total_call_time / TASK_COUNT;

    // 输出结果
    printf("uv_queue_work performance test:\n");
    printf("  Total calls: %d\n", TASK_COUNT);
    printf("  Total time: %.6f ms\n", total_call_time);
    printf("  Average call time: %.9f ms (%.3f μs)\n",
           avg_call_time, avg_call_time * 1000);
    printf("  Min call time: %.9f ms (%.3f μs)\n",
           min_call_time, min_call_time * 1000);
    printf("  Max call time: %.9f ms (%.3f μs)\n",
           max_call_time, max_call_time * 1000);

    // 清理资源
    uv_loop_close(loop);
    free(loop);
}

int main(int argc, char *argv[]) {

    UVTaskPool pool;
    testNormal();

    uint64_t start_time, end_time;
    start_time = uv_hrtime();
    // 开始正式测试
    for (int i = 0; i < TASK_COUNT; i++) {
       pool.post([] {

       });
    }
    end_time = uv_hrtime();
    std::cout << "uv_queue_work pool: " << ((end_time - start_time)/1000000.0) << " ms" << std::endl;

    pool.stop();
    return 0;
}