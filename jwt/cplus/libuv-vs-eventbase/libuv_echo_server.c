#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>

#define DEFAULT_PORT 7000
#define DEFAULT_BACKLOG 128

uv_loop_t *loop;
uv_tcp_t server;
size_t connection_count = 0;
size_t message_count = 0;
uv_timer_t stats_timer;

typedef struct {
    uv_write_t req;
    uv_buf_t buf;
} write_req_t;

void free_write_req(write_req_t *req) {
    free(req->buf.base);
    free(req);
}

void on_close(uv_handle_t *handle) {
    free(handle);
}

void after_write(uv_write_t *req, int status) {
    write_req_t *wr = (write_req_t*) req;
    if (status) {
        fprintf(stderr, "Write error %s\n", uv_strerror(status));
    }
    free_write_req(wr);
}

void on_read(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf) {
    if (nread > 0) {
        message_count += nread;
        
        write_req_t *req = (write_req_t*) malloc(sizeof(write_req_t));
        req->buf = uv_buf_init(buf->base, nread);
        
        uv_write((uv_write_t*) req, client, &req->buf, 1, after_write);
        return;
    }
    
    if (nread < 0) {
        if (nread != UV_EOF)
            fprintf(stderr, "Read error %s\n", uv_err_name(nread));
        uv_close((uv_handle_t*) client, on_close);
    }
    
    free(buf->base);
}

void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
    buf->base = (char*) malloc(suggested_size);
    buf->len = suggested_size;
}

void on_new_connection(uv_stream_t *server, int status) {
    if (status < 0) {
        fprintf(stderr, "New connection error %s\n", uv_strerror(status));
        return;
    }
    
    uv_tcp_t *client = (uv_tcp_t*) malloc(sizeof(uv_tcp_t));
    uv_tcp_init(loop, client);
    
    if (uv_accept(server, (uv_stream_t*) client) == 0) {
        connection_count++;
        uv_read_start((uv_stream_t*) client, alloc_buffer, on_read);
    } else {
        uv_close((uv_handle_t*) client, on_close);
    }
}

void print_stats(uv_timer_t *handle) {
    static size_t last_messages = 0;
    static size_t last_connections = 0;
    
    size_t msg_diff = message_count - last_messages;
    size_t conn_diff = connection_count - last_connections;
    
    printf("Connections: %zu (new: %zu), Messages: %zu bytes (new: %zu bytes)\n",
           connection_count, conn_diff, message_count, msg_diff);
    
    last_messages = message_count;
    last_connections = connection_count;
}

int main() {
    loop = uv_default_loop();
    
    uv_tcp_init(loop, &server);
    
    struct sockaddr_in addr;
    uv_ip4_addr("0.0.0.0", DEFAULT_PORT, &addr);
    
    uv_tcp_bind(&server, (const struct sockaddr*)&addr, 0);
    int r = uv_listen((uv_stream_t*) &server, DEFAULT_BACKLOG, on_new_connection);
    if (r) {
        fprintf(stderr, "Listen error %s\n", uv_strerror(r));
        return 1;
    }
    
    // 初始化统计定时器
    uv_timer_init(loop, &stats_timer);
    uv_timer_start(&stats_timer, print_stats, 1000, 1000);
    
    printf("Server started on port %d\n", DEFAULT_PORT);
    return uv_run(loop, UV_RUN_DEFAULT);
}
    