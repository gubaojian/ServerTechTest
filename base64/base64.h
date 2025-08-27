//
// Created by baojian on 2025/8/26.
//

#ifndef CDK_BASE64_H
#define CDK_BASE64_H
#include <stdbool.h>
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
 * Base64 解码 和 URL-Safe Base64 解码。不支持中间空格换行等情况
 */
int base64_decode_both_url_safe_and_std(const unsigned char *in, size_t inlen, unsigned char *out, size_t* outlen);

/**
 * 自动解码支持，支持带换行和空格的标准 Base64 解码 和 URL-Safe Base64 解码。
 * 合法情况和其它函数相当，非法情况性能比直接解析标准要慢。
 * 如果确定是标准合法的建议直接调用base64_decode_url_safe或者base64_decode
 * 返回0成功，非0失败
 */
int base64_auto_decode(const unsigned char *in, size_t inlen, unsigned char *out, size_t* outlen);

/**
 * 是否包含非法字符串，如空格换行
 */
bool base64_has_invalid_chars(const unsigned char *in, size_t inlen);

/**
 * 自动清理输入换行，空格等非法字符情况，转换url safe模式，转换为标准的base64。
 * out缓冲区传入的outlen要保留padding空间，传入0或空间不足，则不会自动padding
 * 返回0成功，非0失败
 */
int base64_convert_to_std(const unsigned char *in, size_t inlen,
                       unsigned char *out, size_t *outlen);


#endif //CDK_BASE64_H