#include <stdio.h>
#include <string.h>
#include <time.h>

#include "urlparse.h"
#include "url_encode.h"
#include "apr_pools.h"
#include "apr_time.h"

void test_urlparse() {
    urlparse_url parse_url;
    char* url = "http://example.com?search=java%2520%2526%2520uri&page=1";
    size_t url_len = strlen(url);
    {
        clock_t start, end;
        start = clock();
        size_t LOOP_COUNT = 10000*100;
        for (size_t i=0; i<LOOP_COUNT; i++) {
            urlparse_parse_url(url, url_len, 0, &parse_url);
        }
        end = clock();
        double cost_ms = (double)(end - start) / CLOCKS_PER_SEC * 1000;
        printf("urlparse_parse_url耗时：%.2f ms（平均单次：%.6f ms）\n",
           cost_ms, cost_ms / LOOP_COUNT);
    }
    {
        clock_t start, end;
        start = clock();
        apr_initialize();
        apr_pool_t* pool;
        apr_pool_create(&pool, NULL);
        size_t LOOP_COUNT = 10000*100;
        for (size_t i=0; i<LOOP_COUNT; i++) {
            urlparse_parse_url(url, url_len, 0, &parse_url);
        }
        end = clock();
        double cost_ms = (double)(end - start) / CLOCKS_PER_SEC * 1000;
        printf("urlparse_parse_url耗时：%.2f ms（平均单次：%.6f ms）\n",
           cost_ms, cost_ms / LOOP_COUNT);
    }
    {
        char buf[1024];
        clock_t start, end;
        start = clock();
        size_t LOOP_COUNT = 10000*100;
        for (size_t i=0; i<LOOP_COUNT; i++) {
            url_decode(url, url_len, buf);
        }
        end = clock();
        double cost_ms = (double)(end - start) / CLOCKS_PER_SEC * 1000;
        printf("url_decode 耗时：%.2f ms（平均单次：%.6f ms）\n",
           cost_ms, cost_ms / LOOP_COUNT);
    }
}

int main(void) {
    urlparse_url parse_url;
    char* url = "http://www.google.com:8080/page.htm?id=3333";
    size_t url_len = strlen(url);
    urlparse_parse_url(url, url_len, 0, &parse_url);

    test_urlparse();

    printf("scheme %d\n", parse_url.field_data[URLPARSE_SCHEMA].len);
    printf("scheme %d\n", parse_url.field_data[URLPARSE_HOST].len);
    printf("scheme %d\n", parse_url.field_data[URLPARSE_PORT].len);
    printf("scheme %d\n", parse_url.field_data[URLPARSE_PATH].len);
    printf("scheme %d\n", parse_url.field_data[URLPARSE_QUERY].len);
    printf("scheme %d\n", parse_url.field_data[URLPARSE_FRAGMENT].len);
    printf("Hello, World!\n");
    return 0;
}
