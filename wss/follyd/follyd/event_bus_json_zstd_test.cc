//
//  map_find_test.cc
//  cplus
//
//  Created by efurture on 2024/9/26.
//

#include <string>
#include <iostream>
#include <uuid/uuid.h>
#include <chrono>
#include <map>
#include <set>
#include <cstdlib>
#include <unistd.h>
#include <thread>
#include <zstd.h>
#include <lz4.h>

#include "lib/yyjson/yyjson.h"


static std::string genUUID() {
    uuid_t uuid;
    char uuidStrBuffer[37];
    uuid_generate_random(uuid);
    uuid_unparse(uuid, uuidStrBuffer);
    std::string str(uuidStrBuffer, 36);
    return str;
}

static std::string randString(size_t len) {
    std::string str;
    char ch[1];
    for(int i=0; i<len; i++) {
        int value = rand()%36;
        ch[0] = '0';
        if (value < 10) {
            ch[0] += value;
        } else {
            value -=10;
            ch[0] = 'A' + value;
        }
        str.append(ch, 1);
    }
    return str;
}


/**
 
 zstd或者二进制完美避开utf-8验证的限制。 消息转换成二进制，完美避开。
 zstd速度并不快也就 100-120mb 每秒。内网通信，带宽不是问题。压缩必要性其实没那么大。
 
 512字节数据：
 
 yyjson zstd tojson used 12487ms
 yyjson tojson speed 1338.96mb
 yyjson tojson compress 1018.52mb
 yyjson tojson json len702
 yyjson tojson compress json len534
 yyjson lz4 tojson used 2527ms
 yyjson tojson speed 1338.96mb
 yyjson tojson compress 1335.14mb
 yyjson lz4 len 700
 
 1024kb数据：
 yyjson zstd tojson used 14885ms
 yyjson tojson speed 2313.61mb
 yyjson tojson compress 1676.56mb
 yyjson tojson compress json len1213 mb
 yyjson lz4 tojson used 3413ms
 yyjson tojson speed 2313.61mb
 yyjson tojson compress 2317.43mb
 yyjson lz4 len 1215
 
 https://engineering.fb.com/2016/08/31/core-infra/smaller-and-faster-data-compression-with-zstandard/
 
 https://github.com/facebook/zstd
 
 **/
int event_bus_json_zstd_test_main(int argc, const char * argv[]) {
    auto start = std::chrono::high_resolution_clock::now();
    auto end = std::chrono::high_resolution_clock::now();
    auto used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::string msg = randString(512);
    std::string authId = genUUID();
    std::string connId = genUUID();
    std::string appId = std::to_string(rand());
    std::string hwssId = genUUID();
    std::string msgJson;
  
    
    size_t length =10000*200;
    start = std::chrono::high_resolution_clock::now();
    char  zstdBuffer[2048];
    int64_t totalCompressLength = 0;
    size_t jsonLength = 0;
    size_t zstdLen = 0;
    for(int i=0; i<length; i++){
        yyjson_mut_doc *doc = yyjson_mut_doc_new(NULL);
        yyjson_mut_val *root = yyjson_mut_obj(doc);
        yyjson_mut_doc_set_root(doc, root);
        
        yyjson_mut_obj_add_str(doc, root, "action", "msg");
        yyjson_mut_obj_add_strn(doc, root, "msg", msg.data(), msg.size());
        yyjson_mut_obj_add_strn(doc, root, "authId", authId.data(), authId.length());
        yyjson_mut_obj_add_strn(doc, root, "connId", connId.data(), connId.length());
        yyjson_mut_obj_add_strn(doc, root, "appId", appId.data(), appId.length());
        yyjson_mut_obj_add_strn(doc, root, "hwssId", hwssId.data(), hwssId.length());
        size_t len;
        const char *json = yyjson_mut_write(doc, 0, &len);
        if (json == NULL) {
            break;
        }
        jsonLength = len;
        int bound = ZSTD_compressBound(len);
        size_t compressLen = ZSTD_compress(zstdBuffer, bound, json, len, 1);
        totalCompressLength += compressLen;
        zstdLen = compressLen;
        if (json) {
            free((void *)json);
        }
        // Free the doc
        yyjson_mut_doc_free(doc);
    }
    end = std::chrono::high_resolution_clock::now();
    used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "yyjson zstd tojson used " << used.count() << "ms" << std::endl;
    std::cout << "yyjson tojson speed " << (length*jsonLength)/(1024*1024.0) << "mb" << std::endl;
    std::cout << "yyjson tojson compress " << (totalCompressLength)/(1024*1024.0) << "mb" << std::endl;
    
    std::cout << "yyjson tojson json len " << jsonLength  << std::endl;
    std::cout << "yyjson tojson compress json len " << zstdLen  << std::endl;
    
    start = std::chrono::high_resolution_clock::now();
    char  lz4Buffer[2048];
    int64_t lz4totalCompressLength = 0;
    
    jsonLength = 0;
    int lz4len = 0;
    for(int i=0; i<length; i++){
        yyjson_mut_doc *doc = yyjson_mut_doc_new(NULL);
        yyjson_mut_val *root = yyjson_mut_obj(doc);
        yyjson_mut_doc_set_root(doc, root);
        
        yyjson_mut_obj_add_str(doc, root, "action", "msg");
        yyjson_mut_obj_add_strn(doc, root, "msg", msg.data(), msg.size());
        yyjson_mut_obj_add_strn(doc, root, "authId", authId.data(), authId.length());
        yyjson_mut_obj_add_strn(doc, root, "connId", connId.data(), connId.length());
        yyjson_mut_obj_add_strn(doc, root, "appId", appId.data(), appId.length());
        yyjson_mut_obj_add_strn(doc, root, "hwssId", hwssId.data(), hwssId.length());
        size_t len;
        const char *json = yyjson_mut_write(doc, 0, &len);
        if (json == NULL) {
            break;
        }
        jsonLength = len;
        int compressLen = LZ4_compress_default(json, lz4Buffer, (int)len, 2048);
        if (compressLen <= 0) {
            std::cout << "lz4 error" << std::endl;
            break;
        }
        lz4len = compressLen;
        lz4totalCompressLength += compressLen;
        if (json) {
            free((void *)json);
        }
        // Free the doc
        yyjson_mut_doc_free(doc);
    }
    end = std::chrono::high_resolution_clock::now();
    used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "yyjson lz4 tojson used " << used.count() << "ms" << std::endl;
    std::cout << "yyjson tojson speed " << (length*jsonLength)/(1024*1024.0) << "mb" << std::endl;
    std::cout << "yyjson tojson compress " << (lz4totalCompressLength)/(1024*1024.0) << "mb" << std::endl;
    
    std::cout << "yyjson lz4 len " << lz4len << std::endl;
    
    
    return 0;
}
