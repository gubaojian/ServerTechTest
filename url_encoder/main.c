#include <stdio.h>
#include <time.h>

#include "url_encode.h"

int main(void) {
    char encode_dest[1024];
    char decode_dest[1024];
    printf("encode and decode demo \n");
    {
        char* in  = "Hello, World!";
        int elen = url_encode(in,strlen(in), encode_dest);
        printf("%s = %s\n", in, encode_dest);
        int dlen = url_decode(encode_dest, elen, decode_dest);
        printf("%s = %s\n",  encode_dest, decode_dest);
    }

    {
        char* in  = "中国";
        int elen = url_encode(in,strlen(in), encode_dest);
        printf("%s = %s\n", in, encode_dest);
        int dlen = url_decode(encode_dest, elen, decode_dest);
        printf("%s = %s\n",  encode_dest, decode_dest);
    }

    {
        char* in  = "test=";
        int elen = url_encode(in,strlen(in), encode_dest);
        printf("%s = %s\n", in, encode_dest);
        int dlen = url_decode(encode_dest, elen, decode_dest);
        printf("%s = %s\n",  encode_dest, decode_dest);
    }

    {
        char* in  = "illegal";
        int elen = url_encode(in,strlen(in), encode_dest);
        printf("%s = %s\n", in, encode_dest);
        char* illegal = "%E4%B8%AD%E5%不合法9B%BD";
        size_t illegal_len = strlen(illegal);
        memcpy(encode_dest, illegal, illegal_len);
        encode_dest[illegal_len] = '\0';
        int dlen = url_decode(encode_dest, illegal_len, decode_dest);
        printf("%s = %s\n",  encode_dest, decode_dest);
    }

    {
        char* in  = "illegal";
        int elen = url_encode(in,strlen(in), encode_dest);
        printf("%s = %s\n", in, encode_dest);
        char* illegal = "%E4%B8%AD%E5%9B%BD不合法";
        size_t illegal_len = strlen(illegal);
        memcpy(encode_dest, illegal, illegal_len);
        encode_dest[illegal_len] = '\0';
        int dlen = url_decode(encode_dest, illegal_len, decode_dest);
        printf("%s = %s\n",  encode_dest, decode_dest);
    }

    {
        char* in  = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789测试性能编码解码あいうえおαβγδε一二三四五六七八九十abcdefghijklmnopqrstuvwxyzABCDEFG";
        int elen = url_encode(in,strlen(in), encode_dest);
        printf("%s = %s\n", in, encode_dest);
        int dlen = url_decode(encode_dest, elen, decode_dest);
        printf("%s = %s\n",  encode_dest, decode_dest);
        {
            clock_t start, end;
            start = clock();
            size_t LOOP_COUNT = 10000*100;
            for (size_t i=0; i<LOOP_COUNT; i++) {
                url_encode(in,strlen(in), encode_dest);
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
                url_decode(encode_dest, elen, decode_dest);
            }
            end = clock();
            double cost_ms = (double)(end - start) / CLOCKS_PER_SEC * 1000;
            printf("解码耗时：%.2f ms（平均单次：%.6f ms）\n",
               cost_ms, cost_ms / LOOP_COUNT);
        }
    }




    return 0;
}