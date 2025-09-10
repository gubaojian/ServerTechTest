#include <iostream>


#include "llhttp.h"


int perf_handle_on_message_complete(llhttp_t* parser) {
    //fprintf(stdout, "Message completed!\n");
    return 0;
}

void testPerf() {
     llhttp_settings_t settings;

    /*Initialize user callbacks and settings */
    llhttp_settings_init(&settings);

    /*Set user callback */
    settings.on_message_complete = perf_handle_on_message_complete;

    {
        const size_t DATA_SIZE = 32 * 1024;  // 32KB
        std::string file_content;
        file_content.reserve(DATA_SIZE);

        // 生成固定模式的32KB数据（重复"ABCDEFGHIJKLMNOPQRSTUVWXYZ012345"）
        const std::string base = "ABCDEFGHIJKLMNOPQRSTUVWXYZ012345";
        while (file_content.size() < DATA_SIZE) {
            size_t remaining = DATA_SIZE - file_content.size();
            file_content += base.substr(0, std::min(remaining, base.size()));
        }

        std::string boundary = "----FixedBoundary123456";
        std::string body =
            "--" + boundary + "\r\n"
            "Content-Disposition: form-data; name=\"file\"; filename=\"32kb_test.dat\"\r\n"
            "Content-Type: application/octet-stream\r\n"
            "\r\n"
            + file_content + "\r\n"
            "--" + boundary + "--\r\n";

        std::string request =
            "POST /upload HTTP/1.1\r\n"
            "Host: example.com\r\n"
            "Content-Type: multipart/form-data; boundary=" + boundary + "\r\n"
            "Content-Length: " + std::to_string(body.size()) + "\r\n"
            "\r\n" + body;

        clock_t start, end;
        start = clock();
        size_t LOOP_COUNT = 10000*100;
        for (size_t i=0; i<10000*100; i++) {
            llhttp_t parser;
            /*Initialize the parser in HTTP_BOTH mode, meaning that it will select between
            *HTTP_REQUEST and HTTP_RESPONSE parsing automatically while reading the first
            *input.
            */
            llhttp_init(&parser, HTTP_BOTH, &settings);

            enum llhttp_errno err = llhttp_execute(&parser, request.data(), request.size());
            if (err == HPE_OK) {

            } else {
                fprintf(stderr, "Parse error: %s %s\n", llhttp_errno_name(err), llhttp_get_error_reason(&parser));
            }
        }
        end = clock();
        double cost_ms = (double)(end - start) / CLOCKS_PER_SEC * 1000;
        printf("解码耗时：%.2f ms（平均单次：%.6f ms）\n",
           cost_ms, cost_ms / LOOP_COUNT);
    }
}

void testNormalUsage() {


    const size_t DATA_SIZE = 32 * 1024;  // 32KB
    std::string file_content;
    file_content.reserve(DATA_SIZE);

    // 生成固定模式的32KB数据（重复"ABCDEFGHIJKLMNOPQRSTUVWXYZ012345"）
    const std::string base = "ABCDEFGHIJKLMNOPQRSTUVWXYZ012345";
    while (file_content.size() < DATA_SIZE) {
        size_t remaining = DATA_SIZE - file_content.size();
        file_content += base.substr(0, std::min(remaining, base.size()));
    }

    std::string boundary = "----FixedBoundary123456";
    std::string body =
        "--" + boundary + "\r\n"
        "Content-Disposition: form-data; name=\"file\"; filename=\"32kb_test.dat\"\r\n"
        "Content-Type: application/octet-stream\r\n"
        "\r\n"
        + file_content + "\r\n"
        "--" + boundary + "--\r\n";

    std::string request =
        "POST /upload HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "Content-Type: multipart/form-data; boundary=" + boundary + "\r\n"
        "Content-Length: " + std::to_string(body.size()) + "\r\n"
        "\r\n" + body;

    llhttp_settings_t settings;

    /*Initialize user callbacks and settings */
    llhttp_settings_init(&settings);

    /*Set user callback */
    settings.on_message_complete = [](llhttp_t* parser) {
        printf("parser->data %p \n", parser->data);
        printf("parser->data %ld \n", parser->content_length);
        return 0;
    };

    settings.on_header_field = [](llhttp_t* parser, const char* at, size_t length) {
        char buf[1024];
        memcpy(buf, at, length);
        buf[length] = '\0';
        printf("parser->data header %s  %ld \n", buf, length);
        return 0;
    };
    settings.on_header_value = [](llhttp_t* parser, const char* at, size_t length) {
        char buf[1024];
        memcpy(buf, at, length);
        buf[length] = '\0';
        printf("parser->data value %s  %ld \n", buf, length);
        return 0;
    };
    settings.on_body = [](llhttp_t* parser, const char* at, size_t length) {
        char buf[1024];
        return 0;
    };
    llhttp_t parser;
    /*Initialize the parser in HTTP_BOTH mode, meaning that it will select between
    *HTTP_REQUEST and HTTP_RESPONSE parsing automatically while reading the first
    *input.
    */
    llhttp_init(&parser, HTTP_BOTH, &settings);

    enum llhttp_errno err = llhttp_execute(&parser, request.data(), request.size());
    if (err == HPE_OK) {
        printf("parser success ");
    } else {
        fprintf(stderr, "Parse error: %s %s\n", llhttp_errno_name(err), llhttp_get_error_reason(&parser));
    }
}

void testNormalPartParseUsage() {




    llhttp_settings_t settings;

    /*Initialize user callbacks and settings */
    llhttp_settings_init(&settings);

    /*Set user callback */
    settings.on_message_complete = [](llhttp_t* parser) {
        printf("parser->data %p \n", parser->data);
        printf("parser->data %ld \n", parser->content_length);
        return 0;
    };

    settings.on_header_field = [](llhttp_t* parser, const char* at, size_t length) {
        char buf[1024];
        memcpy(buf, at, length);
        buf[length] = '\0';
        printf("parser->data header %s  %ld \n", buf, length);
        return 0;
    };
    settings.on_header_value = [](llhttp_t* parser, const char* at, size_t length) {
        char buf[1024];
        memcpy(buf, at, length);
        buf[length] = '\0';
        printf("parser->data value %s  %ld \n", buf, length);
        return 0;
    };
    settings.on_body = [](llhttp_t* parser, const char* at, size_t length) {
        char buf[1024];
        return 0;
    };
    llhttp_t parser;
    /*Initialize the parser in HTTP_BOTH mode, meaning that it will select between
    *HTTP_REQUEST and HTTP_RESPONSE parsing automatically while reading the first
    *input.
    */
    llhttp_init(&parser, HTTP_BOTH, &settings);
    std::string part1 = "GET /api HTTP/1.1\r\nHost: e";  // 不完整的 Host 头部
    std::string part2 = "xample.com\r\n\r\n";              // 剩余数据

    // 3. 第一次解析：处理 part1，未完成
    auto f1 = llhttp_execute(&parser, part1.data(), part1.size());
    // 此时 parser 状态：处于解析 Host 头部值的中间状态

    // 4. 第二次解析：处理拼接后的剩余数据（part1 未解析部分 + part2）
    // 注意：需手动拼接未解析数据！llhttp 不会缓存，需调用者保存
    std::string remaining = part2;  // 提取 part1 未解析部分
    auto err = llhttp_execute(&parser, remaining.data(), remaining.size());

    if (err == HPE_OK) {
        printf("parser success ");
    } else {
        fprintf(stderr, "Parse error: %s %s\n", llhttp_errno_name(err), llhttp_get_error_reason(&parser));
    }
}

int main() {

  //testNormalUsage();

  testNormalPartParseUsage();


}