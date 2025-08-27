//
// Created by baojian on 2025/8/26.
//

#include "base64.h"

#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>


static const unsigned char cdk_data_bin2ascii[65] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static const unsigned char cdk_url_safe_data_bin2ascii[65] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";




#define cdk_conv_bin2ascii(a, table)       ((table)[(a)&0x3f])

#define CDK_B64_EOLN                0xF0
#define CDK_B64_CR                  0xF1
#define CDK_B64_EOF                 0xF2
#define CDK_B64_WS                  0xE0
#define CDK_B64_ERROR               0xFF
#define CDK_B64_NOT_BASE64(a)       (((a)|0x13) == 0xF3)
#define CDK_B64_BASE64(a)           (!CDK_B64_NOT_BASE64(a))



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

// 混合映射表：兼容 标准Base64（+/-）和URL-safe Base64（-/_）
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


static const unsigned char cdk_base64_auto_to_std[256] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x2B, 0x00, 0x2B, 0x00, 0x2F,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    0x38, 0x39, 0x00, 0x00, 0x00, 0x3D, 0x00, 0x00,
    0x00, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
    0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
    0x58, 0x59, 0x5A, 0x00, 0x00, 0x00, 0x00, 0x2F,
    0x00, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
    0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
    0x78, 0x79, 0x7A, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static unsigned char cdk_conv_ascii2bin(unsigned char a, const unsigned char *table)
{
    if (a & 0x80)
        return CDK_B64_ERROR;
    return table[a];
}

static int base64_encodeblock(unsigned char *out,
                               const unsigned char *in, int dlen, bool url_safe)
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

    return ret;
}

static int base64_decodeblock(unsigned char *out,
                               const unsigned char *in, size_t n, size_t tableId)
{
    int i, ret = 0, a, b, c, d;
    unsigned long l;
    const unsigned char *table;



    if (tableId == 1)
        table = cdk_url_safe_data_ascii2bin;
    else if (tableId == 2)
        table = cdk_url_safe_combine_data_ascii2bin;
    else
        table = cdk_data_ascii2bin;

    /* trim whitespace from the start of the line. */
    while ((n > 0) && (cdk_conv_ascii2bin(*in, table) == CDK_B64_WS)) {
        in++;
        n--;
    }

    /*
     * strip off stuff at the end of the line ascii2bin values B64_WS,
     * B64_EOLN, B64_EOLN and B64_EOF
     */
    while ((n > 0) && (CDK_B64_NOT_BASE64(cdk_conv_ascii2bin(in[n - 1], table))))
        n--;

    size_t remain = n % 4;

#ifdef CDK_B64_DECODE_NO_AUTO_PADDING
    if (remain != 0)
        return -1;
#endif

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

    /* process the last block with auto padding. */
    int  autoPadding = 0;
    if (remain == 0) {
        a = cdk_conv_ascii2bin(*(in++), table);
        b = cdk_conv_ascii2bin(*(in++), table);
        c = cdk_conv_ascii2bin(*(in++), table);
        d = cdk_conv_ascii2bin(*(in++), table);
        autoPadding = 0;
    } else if (remain == 1) {
        a = cdk_conv_ascii2bin(*(in++), table);
        b = cdk_conv_ascii2bin('=', table);
        c = cdk_conv_ascii2bin('=', table);
        d = cdk_conv_ascii2bin('=', table);
        autoPadding = 3;
    } else if (remain == 2) {
        a = cdk_conv_ascii2bin(*(in++), table);
        b = cdk_conv_ascii2bin(*(in++), table);
        c = cdk_conv_ascii2bin('=', table);
        d = cdk_conv_ascii2bin('=', table);
        autoPadding = 2;
    } else if (remain == 3) {
        a = cdk_conv_ascii2bin(*(in++), table);
        b = cdk_conv_ascii2bin(*(in++), table);
        c = cdk_conv_ascii2bin(*(in++), table);
        d = cdk_conv_ascii2bin('=', table);
        autoPadding = 1;
    }
    if ((a | b | c | d) & 0x80)
        return -1;
    l = ((((unsigned long)a) << 18L) |
         (((unsigned long)b) << 12L) |
         (((unsigned long)c) << 6L) | (((unsigned long)d)));

    if (autoPadding > 2)
        return -1;

    switch (autoPadding) {
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
    ret += 3 - autoPadding;
    return ret;
}

int base64_encode(const unsigned char *in, size_t inlen, unsigned char *out, size_t* outlen) {
    if (outlen == NULL) {
        return -1;
    }
    if (in == NULL || out == NULL) {
        *outlen = 0;
        return -1;
    }
    int ret = base64_encodeblock(out, in, inlen, false);
    if (ret >= 0) {
        *outlen = ret;
        return 0;
    }
    *outlen = 0;
    return ret;
}
int base64_encode_url_safe(const unsigned char *in, size_t inlen, unsigned char *out, size_t* outlen) {
    if (outlen == NULL) {
        return -1;
    }
    if (in == NULL || out == NULL) {
        *outlen = 0;
        return -1;
    }
    int ret = base64_encodeblock(out, in, inlen, true);
    if (ret >= 0) {
        *outlen = ret;
        return 0;
    }
    *outlen = 0;
    return ret;
}

int base64_decode(const unsigned char *in, size_t inlen, unsigned char *out, size_t* outlen) {
    if (outlen == NULL) {
        return -1;
    }
    if (in == NULL || out == NULL) {
        *outlen = 0;
        return -1;
    }
    int ret = base64_decodeblock(out, in, inlen,  0);
    if (ret >= 0) {
        *outlen = ret;
        return 0;
    }
    *outlen = 0;
    return ret;
}

int base64_decode_url_safe(const unsigned char *in, size_t inlen, unsigned char *out, size_t* outlen) {
    if (outlen == NULL) {
        return -1;
    }
    if (in == NULL || out == NULL) {
        *outlen = 0;
        return -1;
    }

    int ret = base64_decodeblock(out, in, inlen,  1);
    if (ret >= 0) {
        *outlen = ret;
        return 0;
    }
    *outlen = 0;
    return ret;
}

int base64_decode_both_url_safe_and_std(const unsigned char *in, size_t inlen, unsigned char *out, size_t* outlen) {
    if (outlen == NULL) {
        return -1;
    }
    if (in == NULL || out == NULL) {
        *outlen = 0;
        return -1;
    }

    int ret = base64_decodeblock(out, in, inlen,  2);
    if (ret >= 0) {
        *outlen = ret;
        return 0;
    }
    *outlen = 0;
    return ret;
}

//自动解码, 支持标准base64 和 urlsafe 模式的base64
int base64_auto_decode(const unsigned char *in, size_t inlen, unsigned char *out, size_t* outlen) {
    if (outlen == NULL) {
        return -1;
    }
    if (in == NULL || out == NULL) {
        *outlen = 0;
        return -1;
    }
    if (inlen == 0) {
        *outlen = 0;
        return -1;
    }
    //most cast is valid base64 format
    int ret = base64_decodeblock(out, in, inlen, 2);
    if (ret >= 0) {
        *outlen = ret;
        return 0;
    }
    //decode failed bad case which is very rare
    size_t stdInlen = sizeof(unsigned char) * inlen + 4; //add space for padding
    unsigned char* stdIn = (unsigned char*)malloc(stdInlen);
    if (stdIn == NULL) {
        *outlen = 0;
        return -2;
    }

    ret = base64_convert_to_std(in, inlen, stdIn, &stdInlen);
    if (ret == 0 && stdInlen > 0) {
        ret = base64_decodeblock(out, stdIn, stdInlen, 2);
        if (ret >= 0) {
            *outlen = ret;
            free(stdIn);
            return 0;
        }
    }
    free(stdIn);
    *outlen = 0;
    return ret;
}


bool base64_has_invalid_chars(const unsigned char *in, size_t inlen) {
    if (in == NULL) {
        return true;
    }
    while (inlen >= 4) {
        const unsigned char a = cdk_base64_auto_to_std[*in++];
        const unsigned char b = cdk_base64_auto_to_std[*in++];
        const unsigned char c = cdk_base64_auto_to_std[*in++];
        const unsigned char d = cdk_base64_auto_to_std[*in++];
        inlen-=4;
        if (!(a && b && c && d)) { //most case fast path
            return true;
        }
    }
    while (inlen > 0) {
        unsigned char a = cdk_base64_auto_to_std[*in++];
        if (!a) {
            return true;
        }
    }
    return false;
}

int base64_convert_to_std(const unsigned char *in, size_t inlen,
                       unsigned char *out, size_t *outlen) {
    if (outlen == NULL) {
        return -1;
    }
    if (in == NULL || out == NULL) {
        *outlen = 0;
        return -1;
    }
    size_t stdLen = 0;
    while (inlen >= 4) {
        const unsigned char a = cdk_base64_auto_to_std[*in++];
        const unsigned char b = cdk_base64_auto_to_std[*in++];
        const unsigned char c = cdk_base64_auto_to_std[*in++];
        const unsigned char d = cdk_base64_auto_to_std[*in++];
        inlen-=4;
        if (a && b && c && d) { //most case fast path
            *out++ = a;
            *out++ = b;
            *out++ = c;
            *out++ = d;
            stdLen+=4;
            continue;
        }
        if (a) {
            *out++ = a;
            stdLen++;
        }
        if (b) {
            *out++ = b;
            stdLen++;
        }
        if (c) {
            *out++ = c;
            stdLen++;
        }
        if (d) {
            *out++ = d;
            stdLen++;
        }
    }
    while (inlen > 0) {
        unsigned char a = cdk_base64_auto_to_std[*in++];
        if (a) {
            *out++ = a;
            stdLen++;
        }
        inlen--;
    }

    size_t paddingSize = (4 - (stdLen % 4)) % 4;
    //缓冲区大小位设置，或者空间小，则不进行自动padding
    if (*outlen <= 0 || (*outlen < (stdLen + paddingSize))) {
        *outlen = stdLen;
        return 0;
    }

    if (paddingSize == 3) {
        *outlen = 0;
        return -2;
    } else if (paddingSize == 2) {
        *out++ = '=';
        *out++ = '=';
        stdLen+=2;
    } else if (paddingSize == 1) {
        *out++ = '=';
        stdLen += 1;
    }
    *outlen = stdLen;
    return 0;
}
