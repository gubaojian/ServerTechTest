//
// Created by baojian on 2025/8/26.
//

#include "base64.h"

#include <stddef.h>

typedef int cdk_bool;
#define cdk_true 1
#define cdk_false 0

static const unsigned char cdk_data_bin2ascii[65] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static const unsigned char cdk_url_safe_data_bin2ascii[65] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";




#define cdk_conv_bin2ascii(a, table)       ((table)[(a)&0x3f])

#define B64_EOLN                0xF0
#define B64_CR                  0xF1
#define B64_EOF                 0xF2
#define B64_WS                  0xE0
#define B64_ERROR               0xFF
#define B64_NOT_BASE64(a)       (((a)|0x13) == 0xF3)
#define B64_BASE64(a)           (!B64_NOT_BASE64(a))

static const unsigned char cdk_data_ascii2bin[128] = {
       0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
       0xFF, 0xE0, 0xF0, 0xFF, 0xFF, 0xF1, 0xFF, 0xFF,
       0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
       0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
       0xE0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
       0xFF, 0xFF, 0xFF, 0x3E, 0xFF, 0xF2, 0xFF, 0x3F,
       0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B,
       0x3C, 0x3D, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF,
       0xFF, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
       0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E,
       0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16,
       0x17, 0x18, 0x19, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
       0xFF, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20,
       0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
       0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30,
       0x31, 0x32, 0x33, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
};

static const unsigned char cdk_url_safe_data_ascii2bin[128] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xE0, 0xF0, 0xFF, 0xFF, 0xF1, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xE0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x3E, 0xFF, 0xFF,
    0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B,
    0x3C, 0x3D, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF,
    0xFF, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
    0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E,
    0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16,
    0x17, 0x18, 0x19, 0xFF, 0xFF, 0xFF, 0xFF, 0x3F,
    0xFF, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20,
    0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
    0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30,
    0x31, 0x32, 0x33, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
};

// 混合映射表：让URL-safe解码器同时兼容标准Base64（+/-）和URL-safe Base64（-/_）
static const unsigned char cdk_url_safe_combine_data_ascii2bin[128] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xE0, 0xF0, 0xFF, 0xFF, 0xF1, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xE0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0x3E, 0xFF, 0x3E, 0xFF, 0x3F,
    0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B,
    0x3C, 0x3D, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF,
    0xFF, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
    0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E,
    0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16,
    0x17, 0x18, 0x19, 0xFF, 0xFF, 0xFF, 0xFF, 0x3F,
    0xFF, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20,
    0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
    0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30,
    0x31, 0x32, 0x33, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
};

static unsigned char cdk_conv_ascii2bin(unsigned char a, const unsigned char *table)
{
    if (a & 0x80)
        return B64_ERROR;
    return table[a];
}

static int base64_encodeblock(unsigned char *out,
                               const unsigned char *in, int dlen, cdk_bool url_safe)
{
    int i, ret = 0;
    unsigned long l;
    const unsigned char *table;

    if (url_safe)
        table = cdk_url_safe_data_bin2ascii;
    else
        table = cdk_data_bin2ascii;

    for (i = dlen; i > 0; i -= 3) {
        if (i >= 3) {
            l = (((unsigned long)in[0]) << 16L) |
                (((unsigned long)in[1]) << 8L) | in[2];
            *(out++) = cdk_conv_bin2ascii(l >> 18L, table);
            *(out++) = cdk_conv_bin2ascii(l >> 12L, table);
            *(out++) = cdk_conv_bin2ascii(l >> 6L, table);
            *(out++) = cdk_conv_bin2ascii(l, table);
        } else {
            l = ((unsigned long)in[0]) << 16L;
            if (i == 2)
                l |= ((unsigned long)in[1] << 8L);

            *(out++) = cdk_conv_bin2ascii(l >> 18L, table);
            *(out++) = cdk_conv_bin2ascii(l >> 12L, table);
            *(out++) = (i == 1) ? '=' : cdk_conv_bin2ascii(l >> 6L, table);
            *(out++) = '=';
        }
        ret += 4;
        in += 3;
    }

    *out = '\0';
    return ret;
}

static int base64_decodeblock(unsigned char *out,
                               const unsigned char *in, int n,
                               int eof, size_t tableId)
{
    int i, ret = 0, a, b, c, d;
    unsigned long l;
    const unsigned char *table;

    if (eof < -1 || eof > 2)
        return -1;

    if (tableId == 1)
        table = cdk_url_safe_data_ascii2bin;
    else if (tableId == 2)
        table = cdk_url_safe_combine_data_ascii2bin;
    else
        table = cdk_data_ascii2bin;

    /* trim whitespace from the start of the line. */
    while ((n > 0) && (cdk_conv_ascii2bin(*in, table) == B64_WS)) {
        in++;
        n--;
    }

    /*
     * strip off stuff at the end of the line ascii2bin values B64_WS,
     * B64_EOLN, B64_EOLN and B64_EOF
     */
    while ((n > 3) && (B64_NOT_BASE64(cdk_conv_ascii2bin(in[n - 1], table))))
        n--;

    if (n % 4 != 0)
        return -1;
    if (n == 0)
        return 0;

    /* all 4-byte blocks except the last one do not have padding. */
    for (i = 0; i < n - 4; i += 4) {
        a = cdk_conv_ascii2bin(*(in++), table);
        b = cdk_conv_ascii2bin(*(in++), table);
        c = cdk_conv_ascii2bin(*(in++), table);
        d = cdk_conv_ascii2bin(*(in++), table);
        if ((a | b | c | d) & 0x80)
            return -1;
        l = ((((unsigned long)a) << 18L) |
             (((unsigned long)b) << 12L) |
             (((unsigned long)c) << 6L) | (((unsigned long)d)));
        *(out++) = (unsigned char)(l >> 16L) & 0xff;
        *(out++) = (unsigned char)(l >> 8L) & 0xff;
        *(out++) = (unsigned char)(l) & 0xff;
        ret += 3;
    }

    /* process the last block that may have padding. */
    a = cdk_conv_ascii2bin(*(in++), table);
    b = cdk_conv_ascii2bin(*(in++), table);
    c = cdk_conv_ascii2bin(*(in++), table);
    d = cdk_conv_ascii2bin(*(in++), table);
    if ((a | b | c | d) & 0x80)
        return -1;
    l = ((((unsigned long)a) << 18L) |
         (((unsigned long)b) << 12L) |
         (((unsigned long)c) << 6L) | (((unsigned long)d)));

    if (eof == -1)
        eof = (in[2] == '=') + (in[3] == '=');

    switch (eof) {
    case 2:
        *(out++) = (unsigned char)(l >> 16L) & 0xff;
        break;
    case 1:
        *(out++) = (unsigned char)(l >> 16L) & 0xff;
        *(out++) = (unsigned char)(l >> 8L) & 0xff;
        break;
    case 0:
        *(out++) = (unsigned char)(l >> 16L) & 0xff;
        *(out++) = (unsigned char)(l >> 8L) & 0xff;
        *(out++) = (unsigned char)(l) & 0xff;
        break;
    }
    ret += 3 - eof;
    return ret;
}


int base64_encode(const unsigned char *in, size_t inlen, unsigned char *out) {
    return base64_encodeblock(out, in, inlen, cdk_false);
}
int base64_encode_url_safe(const unsigned char *in, size_t inlen, unsigned char *out) {
    return base64_encodeblock(out, in, inlen, cdk_true);
}

int base64_decode(const unsigned char *in, size_t inlen, unsigned char *out) {
    return base64_decodeblock(out, in, inlen, -1, 0);
}

int base64_decode_url_safe(const unsigned char *in, size_t inlen, unsigned char *out) {
    return base64_decodeblock(out, in, inlen, -1, 1);
}

//自动解码, 支持标准base64 和 urlsafe 模式的base64
int base64_auto_decode(const unsigned char *in, size_t inlen, unsigned char *out) {
    return base64_decodeblock(out, in, inlen, -1, 2);
}