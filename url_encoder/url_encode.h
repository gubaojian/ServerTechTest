//
// Created by baojian on 2025/8/18.
//

#ifndef URL_ENCODER_URL_ENCODE_H
#define URL_ENCODER_URL_ENCODE_H
#include <string.h>



inline size_t  max_url_encode_len(size_t length) {
    return length * 3 + 1; //length * 3 + 1, 1 for \n space
}

inline size_t  max_url_decode_len(size_t length) {
    return length + 1; // 1 for \n space
}

/**
 *  Escapes for URL the given unescaped string of given length.
 * @param src
 * @param length
 * @param dest
 * @return Returns the length of encode
 */
size_t  url_encode(const char *src, size_t length, char *dest);

/**
 * Unescapes the given URL escaped string of given length.
 * @param src
 * @param length
 * @param dest
 * @return  Returns the length of decode
 */
size_t  url_decode(const char *src, size_t length, char *dest);


#endif //URL_ENCODER_URL_ENCODE_H