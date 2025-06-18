//
// Created by baojian on 25-6-16.
//

#include "udp_client.h"

#include <chrono>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <thread>
#include <uv.h>

#define DEFAULT_PORT 8080
#define DEFAULT_TARGET "127.0.0.1"

namespace udp_client {
    uv_loop_t *loop;
    uv_udp_t send_socket;
    struct sockaddr_in recv_addr;
    uv_udp_t recv_socket;

    // 发送完成回调
    void on_send(uv_udp_send_t *req, int status) {
        if (status) {
            fprintf(stderr, "Send error: %s\n", uv_strerror(status));
        }
        free(req->data);
        free(req);
    }

    // 接收服务器响应
    void alloc_recv_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
        buf->base = (char*) malloc(suggested_size);
        buf->len = suggested_size;
    }

    void on_recv_response(uv_udp_t *req, ssize_t nread, const uv_buf_t *buf,
                          const struct sockaddr *addr, unsigned flags) {
        if (nread > 0) {
            char sender[17] = {0};
            uv_ip4_name((const struct sockaddr_in*)addr, sender, 16);
            printf("Response from %s: %.*s\n", sender, (int)nread, buf->base);
        }
        if (buf->base) free(buf->base);
    }

    // 发送消息到服务器
    void send_message(const char *message, const char *target_ip, int port) {
        struct sockaddr_in dest;
        uv_ip4_addr(target_ip, port, &dest);

        uv_udp_send_t *req = (uv_udp_send_t*) malloc(sizeof(uv_udp_send_t));
        req->data = (char*) malloc(strlen(message) + 1);
        std::memcpy(req->data, message, strlen(message));
        uv_buf_t buf = uv_buf_init((char*)req->data, strlen(message));

        uv_udp_send(req, &send_socket, &buf, 1, (const struct sockaddr*)&dest, on_send);
    }

    // 发送组播消息
    void send_multicast(const char *message) {
        struct sockaddr_in dest;
        uv_ip4_addr("239.0.0.1", DEFAULT_PORT, &dest);

        uv_udp_send_t *req = (uv_udp_send_t*) malloc(sizeof(uv_udp_send_t));
        req->data = (char*) malloc(strlen(message) + 1);
        std::memcpy(req->data, message, strlen(message));
        uv_buf_t buf = uv_buf_init((char*)req->data, strlen(message));

        // 设置发送套接字为组播模式
        uv_udp_set_multicast_ttl(&send_socket, 2);
        uv_udp_send(req, &send_socket, &buf, 1, (const struct sockaddr*)&dest, on_send);
    }

    void client_main() {
        loop = uv_default_loop();

        // 初始化发送套接字
        uv_udp_init(loop, &send_socket);

        // 初始化接收套接字（用于接收服务器响应）
        uv_udp_init(loop, &recv_socket);

        uv_ip4_addr("0.0.0.0", 0, &recv_addr); // 随机端口
        uv_udp_bind(&recv_socket, (const struct sockaddr*)&recv_addr, UV_UDP_REUSEADDR);
        uv_udp_recv_start(&recv_socket, alloc_recv_buffer, on_recv_response);

        std::cout << "Hello World!" << recv_addr.sin_port << std::endl;

        // 发送普通 UDP 消息
        send_message("Hello, UDP Server!", DEFAULT_TARGET, DEFAULT_PORT);

        // 发送组播消息
        //send_multicast("Hello, Multicast World!");

        send_message("Hello, nest test message", DEFAULT_TARGET, DEFAULT_PORT);
        
        send_message("Hello, nest test message", DEFAULT_TARGET, DEFAULT_PORT);
        
        send_message("Hello, nest test message", DEFAULT_TARGET, DEFAULT_PORT);
        
        send_message("Hello, nest test message", DEFAULT_TARGET, DEFAULT_PORT);
        
        send_message("Hello, nest test message", DEFAULT_TARGET, DEFAULT_PORT);

        std::cout << "例子有问题，没等待上次发送完成就继续发送。Hello World!" << recv_addr.sin_port << std::endl;
        
      

        uv_run(loop, UV_RUN_DEFAULT);

        std::cout << "client exit" << std::endl;
    }
}
















