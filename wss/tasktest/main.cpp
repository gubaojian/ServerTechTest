#include <iostream>


#include <stdio.h>
#include <stdlib.h>
#include <thread>
#include <uv.h>


#define TASK_COUNT 1000000  // 测试100万次调用


struct UVTask {
    std::function<void()> func;
};

class UVTaskPool {
 public:
       UVTaskPool() {
           loop = uv_loop_new();
           loopThread = std::make_shared<std::thread>([this]() {
               loop = uv_loop_new();
               runLoop();
           });
       }
       ~UVTaskPool() {
          stop();
       }
public:
    void post(std::function<void()>&& func) {
        auto* req = (uv_work_t*)malloc(sizeof(uv_work_t));
        uv_queue_work(loop, req, [](uv_work_t* req) {
        }, [](uv_work_t *req, int status) {
            // req->data = nullptr;
             free(req);
        });
    }

    void stop() {
        // 等待线程结束
        if (loopThread->joinable()) {
            loopThread->join();
        }
    }
private:
    void runLoop(){
        while (!stop_flag) {
            std::cout << "uv_loop default start " << std::endl;
            uv_run(loop, UV_RUN_DEFAULT);
            std::cout << "uv_loop default stop" << std::endl;
        }
        std::cout << "uv_loop all stop" << std::endl;
        //uv_loop_close(loop);
        //free(loop);
    }
  private:
    std::shared_ptr<std::thread> loopThread;
    uv_loop_t *loop = nullptr;
    uv_async_t stop_async;
    std::atomic<bool> stop_flag;
};



int main(int argc, char *argv[]) {

    UVTaskPool pool;


    uint64_t start_time, end_time;
    start_time = uv_hrtime();
    std::cout << "uv_queue_work pool: " << std::endl;
    // 开始正式测试
    for (int i = 0; i < TASK_COUNT; i++) {
        std::cout << "uv_queue_work pool: " << i << std::endl;
       pool.post([] {

       });
    }
    end_time = uv_hrtime();
    std::cout << "uv_queue_work pool: " << ((end_time - start_time)/1000000.0) << " ms" << std::endl;

    pool.stop();

    std::this_thread::sleep_for(std::chrono::milliseconds(10000));

    return 0;
}