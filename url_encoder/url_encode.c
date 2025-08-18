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
#define URL_INNER_ISLOWHEXALHA(x) (((x) >= 'a') && ((x) <= 'f'))
#define URL_INNER_ISUPHEXALHA(x) (((x) >= 'A') && ((x) <= 'F'))

#define URL_INNER_ISLOWCNTRL(x) ((unsigned char)(x) <= 0x1f)
#define URL_INNER_IS7F(x) ((x) == 0x7f)

#define URL_INNER_ISLOWPRINT(x) (((x) >= 9) && ((x) <= 0x0d))

#define URL_INNER_ISPRINT(x)  (URL_INNER_ISLOWPRINT(x) || (((x) >= ' ') && ((x) <= 0x7e)))
#define URL_INNER_ISGRAPH(x)  (URL_INNER_ISLOWPRINT(x) || (((x) > ' ') && ((x) <= 0x7e)))
#define URL_INNER_ISCNTRL(x) (URL_INNER_ISLOWCNTRL(x) || URL_INNER_IS7F(x))
#define URL_INNER_ISALPHA(x) (URL_INNER_ISLOWER(x) || URL_INNER_ISUPPER(x))
#define URL_INNER_ISXDIGIT(x) (URL_INNER_ISDIGIT(x) || URL_INNER_ISLOWHEXALHA(x) || URL_INNER_ISUPHEXALHA(x))
#define URL_INNER_ISODIGIT(x) (((x) >= '0') && ((x) <= '7'))
#define URL_INNER_ISALNUM(x)  (URL_INNER_ISDIGIT(x) || URL_INNER_ISLOWER(x) || URL_INNER_ISUPPER(x))
#define URL_INNER_ISUPPER(x)  (((x) >= 'A') && ((x) <= 'Z'))
#define URL_INNER_ISLOWER(x)  (((x) >= 'a') && ((x) <= 'z'))
#define URL_INNER_ISDIGIT(x)  (((x) >= '0') && ((x) <= '9'))
#define URL_INNER_ISBLANK(x)  (((x) == ' ') || ((x) == '\t'))
#define URL_INNER_ISSPACE(x)  (URL_INNER_ISBLANK(x) || (((x) >= 0xa) && ((x) <= 0x0d)))
#define URL_INNER_ISURLPUNTCS(x) (((x) == '-') || ((x) == '.') || ((x) == '_') || \
((x) == '~'))
#define URL_INNER_ISUNRESERVED(x) (URL_INNER_ISALNUM(x) || URL_INNER_ISURLPUNTCS(x))
#define URL_INNER_ISNEWLINE(x) (((x) == '\n') || (x) == '\r')

/* Lower-case digits.  */
const unsigned char url_inner_hex_ldigits[] = "0123456789abcdef";

/* Upper-case digits.  */
const unsigned char url_inner_hex_udigits[] = "0123456789ABCDEF";

/* We use 16 for the zero index (and the necessary bitwise AND in the loop)
   to be able to have a non-zero value there to make valid_digit() able to
   use the info */
const unsigned char url_inner_hex_hexasciitable[] = {
    16, 1, 2, 3, 4, 5, 6, 7, 8, 9, /* 0x30: 0 - 9 */
    0, 0, 0, 0, 0, 0, 0,
    10, 11, 12, 13, 14, 15,        /* 0x41: A - F */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    10, 11, 12, 13, 14, 15         /* 0x61: a - f */
  };

inline static void url_inner_val_to_hex(unsigned char *dest, /* must fit two bytes */
unsigned char val)
{
    dest[0] = url_inner_hex_udigits[val >> 4];
    dest[1] = url_inner_hex_udigits[val & 0x0F];
}

#define url_inner_hex_to_val(x) (unsigned char)(url_inner_hex_hexasciitable[(x) - '0'] & 0x0f)

size_t  inner_url_encode(const unsigned char *src, size_t length, unsigned char *dest) {
    size_t outlen = 0;
    while(length--) {
        unsigned char in = (unsigned char)*src++;
        if(URL_INNER_ISUNRESERVED(in)) {
            *dest++ = in;
            outlen++;
        }
        else {
            /* encode it */
            *dest++ = '%';
            url_inner_val_to_hex(dest, in);
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
            URL_INNER_ISXDIGIT(src[1]) && URL_INNER_ISXDIGIT(src[2])) {
            /* this is two hexadecimal digits following a '%' */
            in = (unsigned char)((url_inner_hex_to_val(src[1]) << 4) |
                                 url_inner_hex_to_val(src[2]));
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