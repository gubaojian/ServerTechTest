#include <stdio.h>
#include <string.h>
#include <time.h>

#include "fast_hex.h"

int main(void) {
    char encode_dest[1024];
    char decode_dest[1024];
    {
        char* in  = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789测试性能编码解abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789测试性能编码解abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789测试性能编码解abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789测试性能编码解abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789测试性能编码解码あいうえおαβγδε一二三四五六七八九十abcdefghijklmnopqrstuvwxyzABCDEFG";
        size_t len = strlen(in);
        encodeHex(encode_dest, in, len);
        encode_dest[len*2] = '\n';
        printf("%s  = %s\n", in, encode_dest);
        decodeHexLUT4(decode_dest, encode_dest, len);
        decode_dest[len] = '\n';
        printf("%s = %s\n",  encode_dest, decode_dest);
        {
            clock_t start, end;
            start = clock();
            size_t LOOP_COUNT = 10000*100;
            for (size_t i=0; i<LOOP_COUNT; i++) {
                encodeHex(encode_dest, in, len);
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
                decodeHexLUT4(decode_dest, encode_dest, len);
            }
            end = clock();
            double cost_ms = (double)(end - start) / CLOCKS_PER_SEC * 1000;
            printf("解码耗时：%.2f ms（平均单次：%.6f ms）\n",
               cost_ms, cost_ms / LOOP_COUNT);
        }
    }
    return 0;
}
