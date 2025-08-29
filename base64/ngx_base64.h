//
// Created by baojian on 2025/8/29.
//

#ifndef BASE64_NGX_BASE64_H
#define BASE64_NGX_BASE64_H
#include <stdio.h>

typedef unsigned char u_char;

typedef struct {
    size_t      len;
    u_char     *data;
} ngx_str_t;

typedef int ngx_int_t;
typedef unsigned int ngx_uint_t;



#define  NGX_OK 0
#define  NGX_ERROR -1

void ngx_encode_base64(ngx_str_t *dst, ngx_str_t *src);
void ngx_encode_base64url(ngx_str_t *dst, ngx_str_t *src);
ngx_int_t ngx_decode_base64(ngx_str_t *dst, ngx_str_t *src);
ngx_int_t ngx_decode_base64url(ngx_str_t *dst, ngx_str_t *src);

#endif //BASE64_NGX_BASE64_H