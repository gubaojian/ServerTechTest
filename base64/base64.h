//
// Created by baojian on 2025/8/26.
//

#ifndef CDK_BASE64_H
#define CDK_BASE64_H
#include <stddef.h>

static inline size_t base64_encode_len(size_t inlen) {
    return ((inlen + 2) / 3) * 4;
}


int base64_encode(const unsigned char *in, size_t inlen, unsigned char *out);
int base64_encode_url_safe(const unsigned char *in, size_t inlen, unsigned char *out);


int base64_decode(const unsigned char *in, size_t inlen, unsigned char *out);
int base64_decode_url_safe(const unsigned char *in, size_t inlen, unsigned char *out);

//自动解码, 支持标准base64 和 urlsafe 模式的base64
int base64_auto_decode(const unsigned char *in, size_t inlen, unsigned char *out);


#endif //CDK_BASE64_H