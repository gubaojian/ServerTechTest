//
// Created by baojian on 25-6-16.
//

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <uv.h>
#include "udp_server.h"

#include <iostream>
#include <__ostream/basic_ostream.h>

#define DEFAULT_PORT 8080
#define DEFAULT_BACKLOG 128

uv_loop_t *loop;
uv_udp_t recv_socket;

// 内存分配回调
void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
    buf->base = (char*) malloc(suggested_size);
    buf->len = suggested_size;
    std::cout << "alloc_buffer " << suggested_size << std::endl;
}

void send_compete(uv_udp_send_t* req, int status) {
    if (status) {
        fprintf(stderr, "Send error: %s\n", uv_strerror(status));
    }
    std::cout << "reply send_compete free req " << req->data << std::endl;
    free(req->data);
    free(req);
}

// 接收数据回调
void on_recv(uv_udp_t *req, ssize_t nread, const uv_buf_t *buf,
             const struct sockaddr *addr, unsigned flags) {
    if (nread < 0) {
        printf("Read error %s\n", uv_strerror(nread));
        uv_close((uv_handle_t*) req, NULL);
        free(buf->base);
        return;
    }

    if (nread == 0) {
        // 特殊处理：空读取事件
        printf("Empty read event (nread == 0)\n");
        std::cout << "Empty read event " << buf->len << std::endl;
        free(buf->base);
        return;
    }

    std::cout << "received data from client"
    << nread << "family " << addr->sa_family
    << "queue size " << req->send_queue_size
    << "queue count " << req->send_queue_count
    << std::endl;

    if (addr) {
        std::string_view message = std::string_view(buf->base, nread);
        std::string reply = std::string("Message received");
        char sender[17] = {0};
        uv_ip4_name((const struct sockaddr_in*)addr, sender, 16);
        printf("Recv from %s: %.*s\n", sender, (int)nread, buf->base);
        std::cout << "sended to complete" << message << std::endl;

        // 回复客户端
        uv_udp_send_t *send_req = (uv_udp_send_t*) malloc(sizeof(uv_udp_send_t));
        send_req->data = std::malloc(reply.size());
        std::memcpy(send_req->data, reply.data(), reply.size());
        uv_buf_t send_buf = uv_buf_init((char*)send_req->data, reply.size());
        uv_udp_send(send_req, req, &send_buf, 1, addr, &send_compete);
    }

    free(buf->base);
}

// 组播示例：加入组播组
void join_multicast_group(uv_udp_t *handle) {
    // 加入 239.0.0.1 组播组
    uv_udp_set_membership(handle, "239.0.0.1", "0.0.0.0", UV_JOIN_GROUP);
    // 设置组播 TTL
    uv_udp_set_multicast_ttl(handle, 16);
}

void server_main() {
    loop = uv_default_loop();

    // 初始化 UDP 套接字
    uv_udp_init(loop, &recv_socket);

    // 绑定地址
    struct sockaddr_in addr;
    uv_ip4_addr("0.0.0.0", DEFAULT_PORT, &addr);
    uv_udp_bind(&recv_socket, (const struct sockaddr*)&addr, UV_UDP_REUSEADDR);

    // 加入组播组（可选）
    join_multicast_group(&recv_socket);

    // 开始接收数据
    uv_udp_recv_start(&recv_socket, alloc_buffer, on_recv);

    printf("UDP server listening on port %d\n", DEFAULT_PORT);
    uv_run(loop, UV_RUN_DEFAULT);
}

