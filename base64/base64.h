//
// Created by baojian on 2025/8/26.
//

#ifndef CDK_BASE64_H
#define CDK_BASE64_H
#include <stddef.h>

static inline size_t base64_encode_len(size_t inlen) {
    return ((inlen + 2) / 3) * 4;
}

static inline size_t base64_decode_max_len(size_t inlen) {
    return ((inlen + 3) / 4) * 3;
}


/**
 * 标准 Base64 编码， 返回0成功，非0失败
 */
int base64_encode(const unsigned char *in, size_t inlen, unsigned char *out, size_t* outlen);
/**
 * URL-Safe Base64 编码， 返回0成功，非0失败
 */
int base64_encode_url_safe(const unsigned char *in, size_t inlen, unsigned char *out, size_t* outlen);


/**
 * 标准 Base64 解码， 返回0成功，非0失败
 */
int base64_decode(const unsigned char *in, size_t inlen, unsigned char *out, size_t* outlen);

/**
 * URL-Safe Base64 解码， 返回0成功，非0失败
 */
int base64_decode_url_safe(const unsigned char *in, size_t inlen, unsigned char *out, size_t* outlen);

/**
 * 自动解码支持 标准 Base64 解码 和 URL-Safe Base64 解码， 返回0成功，非0失败
 */
int base64_auto_decode(const unsigned char *in, size_t inlen, unsigned char *out, size_t* outlen);


#endif //CDK_BASE64_H