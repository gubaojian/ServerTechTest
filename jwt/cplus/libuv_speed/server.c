#include <stdio.h>
#include <stdlib.h>
#include <uv.h>

#define DEFAULT_PORT 5001
#define DEFAULT_BACKLOG 128
#define BUFFER_SIZE 65536

uv_loop_t *loop;
uv_tcp_t server;
uv_timer_t timer;

size_t total_bytes = 0;
double start_time = 0;
int is_testing = 0;

// 客户端连接处理
void on_new_connection(uv_stream_t *server, int status);
// 分配内存缓冲区
void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf);
// 读取数据回调
void read_cb(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf);
// 关闭客户端连接
void close_client(uv_handle_t *handle);
// 定时器回调，每秒输出带宽
void timer_cb(uv_timer_t *handle);

int main() {
    loop = uv_default_loop();

    // 初始化TCP服务器
    uv_tcp_init(loop, &server);

    // 绑定地址和端口
    struct sockaddr_in addr;
    uv_ip4_addr("0.0.0.0", DEFAULT_PORT, &addr);

    // 绑定并监听
    uv_tcp_bind(&server, (const struct sockaddr *)&addr, 0);
    int r = uv_listen((uv_stream_t *)&server, DEFAULT_BACKLOG, on_new_connection);
    if (r) {
        fprintf(stderr, "Listen error %s\n", uv_strerror(r));
        return 1;
    }

    // 初始化定时器
    uv_timer_init(loop, &timer);
    uv_timer_start(&timer, timer_cb, 1000, 1000);

    printf("Server listening on port %d\n", DEFAULT_PORT);
    return uv_run(loop, UV_RUN_DEFAULT);
}

void on_new_connection(uv_stream_t *server, int status) {
    if (status < 0) {
        fprintf(stderr, "New connection error %s\n", uv_strerror(status));
        return;
    }

    uv_tcp_t *client = (uv_tcp_t *)malloc(sizeof(uv_tcp_t));
    uv_tcp_init(loop, client);

    if (uv_accept(server, (uv_stream_t *)client) == 0) {
        printf("New client connected\n");
        is_testing = 1;
        total_bytes = 0;
        start_time = uv_now(loop) / 1000.0;
        uv_read_start((uv_stream_t *)client, alloc_buffer, read_cb);
    } else {
        uv_close((uv_handle_t *)client, close_client);
    }
}

void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
    buf->base = (char *)malloc(BUFFER_SIZE);
    buf->len = BUFFER_SIZE;
}

void read_cb(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf) {
    if (nread > 0) {
        total_bytes += nread;
        return;
    }

    if (nread < 0) {
        if (nread != UV_EOF)
            fprintf(stderr, "Read error %s\n", uv_err_name(nread));
        uv_close((uv_handle_t *)client, close_client);
    }

    free(buf->base);
}

void close_client(uv_handle_t *handle) {
    is_testing = 0;
    double duration = (uv_now(loop) / 1000.0) - start_time;
    if (duration > 0) {
        double mbits = (total_bytes * 8.0) / (1024.0 * 1024.0);
        double bandwidth = mbits / duration;
        printf("Test completed - Bandwidth: %.2f Mbps\n", bandwidth);
    }
    free(handle);
}

void timer_cb(uv_timer_t *handle) {
    if (!is_testing) return;

    double current_time = uv_now(loop) / 1000.0;
    double duration = current_time - start_time;

    if (duration > 0) {
        double mbits = (total_bytes * 8.0) / (1024.0 * 1024.0);
        double bandwidth = mbits / duration;
        printf("Current bandwidth: %.2f Mbps\n", bandwidth);
        printf("Current bandwidth speed: %.2f Mb\n", bandwidth/8);
    }
}
