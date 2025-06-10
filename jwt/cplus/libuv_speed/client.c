#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>

#define DEFAULT_PORT 5001
#define DEFAULT_IP "127.0.0.1"
#define TEST_DURATION 10  // 测试时长(秒)
#define BUFFER_SIZE 65536

uv_loop_t *loop;
uv_tcp_t client;
uv_timer_t timer;
uv_connect_t connect_req;

char buffer[BUFFER_SIZE];
size_t total_bytes = 0;
int is_running = 0;

// 连接回调
void on_connect(uv_connect_t *req, int status);
// 写入回调
void on_write(uv_write_t *req, int status);
// 定时器回调
void on_timer(uv_timer_t *handle);
// 关闭客户端
void close_client();

int main(int argc, char **argv) {
    loop = uv_default_loop();
    uv_tcp_init(loop, &client);

    // 初始化测试数据
    memset(buffer, 'A', BUFFER_SIZE);

    struct sockaddr_in dest;
    uv_ip4_addr(DEFAULT_IP, DEFAULT_PORT, &dest);

    printf("Connecting to server %s:%d\n", DEFAULT_IP, DEFAULT_PORT);
    uv_tcp_connect(&connect_req, &client, (const struct sockaddr *)&dest, on_connect);

    // 设置定时器，控制测试时长
    uv_timer_init(loop, &timer);

    return uv_run(loop, UV_RUN_DEFAULT);
}

void on_connect(uv_connect_t *req, int status) {
    if (status < 0) {
        fprintf(stderr, "Connect error %s\n", uv_strerror(status));
        return;
    }

    printf("Connected to server. Starting test for %d seconds...\n", TEST_DURATION);
    is_running = 1;
    total_bytes = 0;

    // 启动定时器
    uv_timer_start(&timer, on_timer, TEST_DURATION * 1000, 0);

    // 开始发送数据
    uv_write_t *write_req = (uv_write_t *)malloc(sizeof(uv_write_t));
    uv_buf_t buf = uv_buf_init(buffer, BUFFER_SIZE);
    uv_write(write_req, (uv_stream_t *)&client, &buf, 1, on_write);
}

void on_write(uv_write_t *req, int status) {
    free(req);

    if (status < 0) {
        fprintf(stderr, "Write error %s\n", uv_strerror(status));
        close_client();
        return;
    }

    total_bytes += BUFFER_SIZE;

    if (is_running) {
        // 继续发送数据
        uv_write_t *new_req = (uv_write_t *)malloc(sizeof(uv_write_t));
        uv_buf_t buf = uv_buf_init(buffer, BUFFER_SIZE);
        uv_write(new_req, (uv_stream_t *)&client, &buf, 1, on_write);
    }
}

void on_timer(uv_timer_t *handle) {
    is_running = 0;
    
    double duration = TEST_DURATION;
    double mbits = (total_bytes * 8.0) / (1024.0 * 1024.0);
    double bandwidth = mbits / duration;

    printf("Test completed - Sent: %.2f MB, Bandwidth: %.2f Mbps\n", 
           total_bytes / (1024.0 * 1024.0), bandwidth);

    close_client();
}

void close_client() {
    uv_close((uv_handle_t *)&client, NULL);
    uv_close((uv_handle_t *)&timer, NULL);
}
