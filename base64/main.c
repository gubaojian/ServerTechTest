#include <stdio.h>
#include <string.h>
#include <time.h>

#include "base64.h"



void print_normal_table() {
    static const unsigned char bin2ascii[65] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    unsigned char ascii2bin[128] = {
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xE0, 0xF0, 0xFF, 0xFF, 0xF1, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xE0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0x3E, 0xFF, 0xF2, 0xFF, 0x3F, // '+'→0x3E（62），'/'→0x3F（63）
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
    for (int i=0; i<64; i++) {
        unsigned char c = bin2ascii[i];
        ascii2bin[c] = (unsigned char)i;
    }
    for (int i=0; i<128; i+=8) {
        for (int j=0; j<8; j++) {
            printf("0x%02X, ", (unsigned int)ascii2bin[i +j]);
        }
        printf("\n");
    }
    printf("\n\n");
}

void print_url_safe_table() {
    static const unsigned char bin2ascii[65] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

    unsigned char ascii2bin[128] = {
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xE0, 0xF0, 0xFF, 0xFF, 0xF1, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xE0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0x3E, 0xFF, 0xF2, 0xFF, 0x3F, // '+'→0x3E（62），'/'→0x3F（63）
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
    //默认纯净表
    ascii2bin['+'] = (unsigned char)0xFF;
    ascii2bin['/'] = (unsigned char)0xFF;
    for (int i=0; i<64; i++) {
        unsigned char c = bin2ascii[i];
        ascii2bin[c] = (unsigned char)i;
    }
    for (int i=0; i<128; i+=8) {
        for (int j=0; j<8; j++) {
            printf("0x%02X, ", (unsigned int)ascii2bin[i +j]);
        }
        printf("\n");
    }
    printf("\n\n");
}

void print_base64_auto_to_std_table() {
    static const unsigned char valid_ascii[65] =
       "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    unsigned char ascii2bin[256] = {};
    memset(ascii2bin, 0, sizeof(ascii2bin));
    for (int i=0; i<65; i++) {
        unsigned char c = valid_ascii[i];
        ascii2bin[c] = c;
    }
    ascii2bin['-'] = '+';
    ascii2bin['_'] = '/';
    ascii2bin['='] = '=';
    printf("\n\n");
    for (int i=0; i<256; i+=8) {
        for (int j=0; j<8; j++) {
            printf("0x%02X, ", (unsigned int)ascii2bin[i +j]);
        }
        printf("\n");
    }
    printf("\n\n");
}

void testBase64() {
    char out[1024];
    char dout[1024];
    char* in = " +/=你好 base64 测试 ";
    size_t outlen;
    size_t dlen;
    {
        printf("base64 encode decode \n");
        size_t inlen = strlen(in);
        base64_encode(in, inlen, out, &outlen);
        printf("%s %ld %ld\n", out, outlen,  base64_encode_len(inlen));
        base64_decode(out, outlen, dout, &dlen);
        dout[dlen] = '\0';
        printf("%s %ld\n", dout, dlen);
    }

    {
        printf("base64 encode decode \n");
        size_t inlen = strlen(in);
        base64_encode_url_safe(in, inlen, out, &outlen);
        printf("%s %ld %ld\n", out, outlen,  base64_encode_len(inlen));
        base64_decode_url_safe(out, outlen, dout, &dlen);
        dout[dlen] = '\0';
        printf("%s %ld\n", dout, dlen);
    }

    {
        printf("base64 encode auto decode \n");
        size_t inlen = strlen(in);
        base64_encode_url_safe(in, inlen, out, &outlen);
        printf("%s %ld %ld\n", out, outlen,  base64_encode_len(inlen));
        base64_auto_decode(out, outlen, dout, &dlen);
        dout[dlen] = '\0';
        printf("%s %ld\n", dout, dlen);
    }

    {
        printf("base64 encode auto decode \n");
        size_t inlen = strlen(in);
        base64_encode(in, inlen, out, &outlen);
        printf("%s %ld %ld\n", out, outlen,  base64_encode_len(inlen));
        base64_auto_decode(out, outlen, dout, &dlen);
        dout[dlen] = '\0';
        printf("%s %ld\n", dout, dlen);
    }

    {
        printf("base64 encode auto decode 2 \n");
        char* out_test = "ICsvPeS9oOWlvSBiYXNlNjQg5rWL6K+VIA====";
        outlen = strlen(out_test);
        memcpy(out, out_test, outlen);
        base64_auto_decode(out, outlen, dout, &dlen);
        dout[dlen] = '\0';
        printf("%s %ld\n", dout, dlen);
    }

    {
        printf("base64 encode auto decode with to std \n");
        char* out_test = "I CsvPeS9oOW  lvSBiYXN   lNj\n Qg5rWL6K+VIA====";
        outlen = strlen(out_test);
        memcpy(out, out_test, outlen);
        base64_auto_decode(out, outlen, dout, &dlen);
        dout[dlen] = '\0';
        printf("%s %ld\n", dout, dlen);
    }

}

/**
 *
*  编码耗时：66.93 ms（平均单次：0.000067 ms）
*  解码耗时：77.70 ms（平均单次：0.000078 ms）
*  自动解码包含空格的解码耗时：164.25 ms（平均单次：0.000164 ms）
 */
void testBase64Perf() {
    char out[1024];
    char dout[1024];
    char* in = " +/=你好 base64 测试+/=你好 base64 测试+/=你好 base64 测试+/=你好 base64 测试+/=你好 base64 测试+/=你好 base64 测试v+/=你好 base64 测试+/=你好 base64 测试+/=你好 base64 测试+/=你好 base64 测试 ";
    size_t inlen = strlen(in);
    size_t outlen;
    size_t dlen;
    {
        clock_t start, end;
        start = clock();
        size_t LOOP_COUNT = 10000*100;
        for (size_t i=0; i<LOOP_COUNT; i++) {
            base64_encode(in, inlen, out, &outlen);
        }
        end = clock();
        double cost_ms = (double)(end - start) / CLOCKS_PER_SEC * 1000;
        printf("编码耗时：%.2f ms（平均单次：%.6f ms）\n",
           cost_ms, cost_ms / LOOP_COUNT);
    }

    {
        clock_t start, end;
        start = clock();
        size_t LOOP_COUNT = 10000*100;
        for (size_t i=0; i<10000*100; i++) {
            base64_auto_decode(out, outlen, dout, &dlen);
        }
        end = clock();
        double cost_ms = (double)(end - start) / CLOCKS_PER_SEC * 1000;
        printf("解码耗时：%.2f ms（平均单次：%.6f ms）\n",
           cost_ms, cost_ms / LOOP_COUNT);
    }

    {
        clock_t start, end;
        start = clock();
        size_t LOOP_COUNT = 10000*100;
        char* out_test = "ICsvPeS 9oOWlvSBiYXNlNjQg5r WL6K+VKy895L2g5aW9IGJhc2U2NCDmtYvor5UrLz3kvaDlpb0gYmFzZTY0IOa1i+ivlSsvPeS9oOWlvSBiYXNlNjQg5rWL6K+VKy895L2g5aW9IGJhc2U2NCDmtYvor5UrLz3kvaDlpb0gYmFzZTY0IOa1i+ivlXYrLz3kvaDlpb0gYmFzZTY0IOa1i+ivlSsvPeS9oOWlvSBiYXNlNjQg5rWL6K+VKy895L2g5aW9IGJhc2U2NCDmtYvor5UrLz3kvaDlpb0gYmFzZTY0IOa1i+ivlSA=";
        outlen = strlen(out_test);
        memcpy(out, out_test, outlen);

        for (size_t i=0; i<LOOP_COUNT; i++) {
            base64_auto_decode(out, outlen, dout, &dlen);
        }
        end = clock();
        double cost_ms = (double)(end - start) / CLOCKS_PER_SEC * 1000;
        printf("自动解码包含空格的解码耗时：%.2f ms（平均单次：%.6f ms）\n",
           cost_ms, cost_ms / LOOP_COUNT);
    }


}


int main(void) {
    testBase64();
    testBase64Perf();
    //print_normal_table();
    //print_url_safe_table();

    //print_base64_auto_to_std_table();

    return 0;
}
