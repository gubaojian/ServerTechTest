//
// Created by baojian on 2025/8/18.
//

#include "url_encode.h"

#include <stdio.h>

/**
 * https://github.com/curl/curl/blob/master/lib/escape.c
 * https://github.com/ada-url/ada/tree/main
 * https://github.com/cpp-netlib/cpp-netlib
 * url 参考curl及java代码实现。独立类，零依赖，方便外部工程使用
 */
#define INNER_ISLOWHEXALHA(x) (((x) >= 'a') && ((x) <= 'f'))
#define INNER_ISUPHEXALHA(x) (((x) >= 'A') && ((x) <= 'F'))

#define INNER_ISLOWCNTRL(x) ((unsigned char)(x) <= 0x1f)
#define INNER_IS7F(x) ((x) == 0x7f)

#define INNER_ISLOWPRINT(x) (((x) >= 9) && ((x) <= 0x0d))

#define INNER_ISPRINT(x)  (INNER_ISLOWPRINT(x) || (((x) >= ' ') && ((x) <= 0x7e)))
#define INNER_ISGRAPH(x)  (INNER_ISLOWPRINT(x) || (((x) > ' ') && ((x) <= 0x7e)))
#define INNER_ISCNTRL(x) (INNER_ISLOWCNTRL(x) || INNER_IS7F(x))
#define INNER_ISALPHA(x) (INNER_ISLOWER(x) || INNER_ISUPPER(x))
#define INNER_ISXDIGIT(x) (INNER_ISDIGIT(x) || INNER_ISLOWHEXALHA(x) || INNER_ISUPHEXALHA(x))
#define INNER_ISODIGIT(x) (((x) >= '0') && ((x) <= '7'))
#define INNER_ISALNUM(x)  (INNER_ISDIGIT(x) || INNER_ISLOWER(x) || INNER_ISUPPER(x))
#define INNER_ISUPPER(x)  (((x) >= 'A') && ((x) <= 'Z'))
#define INNER_ISLOWER(x)  (((x) >= 'a') && ((x) <= 'z'))
#define INNER_ISDIGIT(x)  (((x) >= '0') && ((x) <= '9'))
#define INNER_ISBLANK(x)  (((x) == ' ') || ((x) == '\t'))
#define INNER_ISSPACE(x)  (INNER_ISBLANK(x) || (((x) >= 0xa) && ((x) <= 0x0d)))
#define INNER_ISURLPUNTCS(x) (((x) == '-') || ((x) == '.') || ((x) == '_') || \
((x) == '~'))
#define INNER_ISUNRESERVED(x) (INNER_ISALNUM(x) || INNER_ISURLPUNTCS(x))
#define INNER_ISNEWLINE(x) (((x) == '\n') || (x) == '\r')

/* Lower-case digits.  */
const unsigned char inner_hex_ldigits[] = "0123456789abcdef";

/* Upper-case digits.  */
const unsigned char inner_hex_udigits[] = "0123456789ABCDEF";

/* We use 16 for the zero index (and the necessary bitwise AND in the loop)
   to be able to have a non-zero value there to make valid_digit() able to
   use the info */
const unsigned char inner_hex_hexasciitable[] = {
    16, 1, 2, 3, 4, 5, 6, 7, 8, 9, /* 0x30: 0 - 9 */
    0, 0, 0, 0, 0, 0, 0,
    10, 11, 12, 13, 14, 15,        /* 0x41: A - F */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    10, 11, 12, 13, 14, 15         /* 0x61: a - f */
  };

inline static void inner_val_to_hex(unsigned char *dest, /* must fit two bytes */
unsigned char val)
{
    dest[0] = inner_hex_udigits[val >> 4];
    dest[1] = inner_hex_udigits[val & 0x0F];
}

#define inner_hex_to_val(x) (unsigned char)(inner_hex_hexasciitable[(x) - '0'] & 0x0f)

size_t  inner_url_encode(const unsigned char *src, size_t length, unsigned char *dest) {
    size_t outlen = 0;
    while(length--) {
        unsigned char in = (unsigned char)*src++;
        if(INNER_ISUNRESERVED(in)) {
            *dest++ = in;
            outlen++;
        }
        else {
            /* encode it */
            *dest++ = '%';
            inner_val_to_hex(dest, in);
            dest += 2;
            outlen += 3;
        }
    }
    *dest = '\0';
    return outlen;
}

size_t inner_url_decode(const unsigned char *src, size_t length, unsigned char *dest) {
    size_t outlen = 0;
    while(length) {
        unsigned char in = (unsigned char)*src;
        if(('%' == in) && (length > 2) &&
            INNER_ISXDIGIT(src[1]) && INNER_ISXDIGIT(src[2])) {
            /* this is two hexadecimal digits following a '%' */
            in = (unsigned char)((inner_hex_to_val(src[1]) << 4) |
                                 inner_hex_to_val(src[2]));
            src += 3;
            length -= 3;
       } else {
            src++;
            length--;
        }
        *dest++ = in;
        outlen++;
    }
    *dest = '\0';
    return outlen;
}

size_t  url_encode(const char *src, size_t length, char *dest) {
    if (src == NULL || dest == NULL) {
        return 0;
    }
    if (length == 0) {
        length = strlen(src);
    }
    if (length <= 0) {
        *dest = '\0';
        return 0;
    }
    return inner_url_encode((const unsigned char *)src, length, (unsigned char *)dest);
}

size_t  url_decode(const char *src, size_t length, char *dest) {
    if (src == NULL || dest == NULL) {
        return 0;
    }
    if (length == 0) {
        length = strlen(src);
    }
    if (length <= 0) {
        *dest = '\0';
        return 0;
    }
    return inner_url_decode((const unsigned char *)src, length, (unsigned char *)dest);
}