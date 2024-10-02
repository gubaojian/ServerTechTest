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
#include <snappy.h>

#include "lib/yyjson/yyjson.h"

#include <folly/futures/Future.h>
#include <folly/executors/GlobalExecutor.h>
#include <folly/executors/CPUThreadPoolExecutor.h>




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
 多线程生成json及压缩速率约480- 500mb每秒。
 1024kb数据：
 
 yyjson zstd tojson used 3780ms
 yyjson tojson speed 2313.61mb
 yyjson tojson task queue size 412388

 task message done 4816ms
 task compress len 881 taskNum 2000000
 task message sleep end 10s
 yyjson tojson task queue size 0
 
 https://engineering.fb.com/2016/08/31/core-infra/smaller-and-faster-data-compression-with-zstandard/
 
 https://github.com/facebook/zstd
 
 **/

static std::atomic<int> taskNum = 0;

int event_bus_json_zstd_multi_thread_test_main(int argc, const char * argv[]) {
    
    auto start = std::chrono::high_resolution_clock::now();
    auto end = std::chrono::high_resolution_clock::now();
    auto used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::string msg = randString(1024);
    std::string authId = genUUID();
    std::string connId = genUUID();
    std::string appId = std::to_string(rand());
    std::string hwssId = genUUID();
    std::string msgJson;
  
    

    size_t length =10000*200;
    start = std::chrono::high_resolution_clock::now();
    size_t zstdLen = 0;
             
    for(int i=0; i<length; i++){
        folly::getGlobalCPUExecutor()->add([msg, authId, connId,
                                            appId, hwssId,
                                            length, start]{
            
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
                return;
            }
            std::shared_ptr<std::string> jsonStr = std::make_shared<std::string>(2048, '\n');
            size_t compressLen = ZSTD_compress(jsonStr->data(), 2048, json, len, 1);
            
            if (json) {
                free((void *)json);
            }
            // Free the doc
            yyjson_mut_doc_free(doc);
            // test for executor.
            //if (taskNum < 3) {
            //    sleep(10); //10s
            //}
            taskNum++;
            if(taskNum >= length) {
                auto end = std::chrono::high_resolution_clock::now();
                auto used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
                std::cout << std::endl;
                std::cout << "task message done " << used.count() << "ms "  << std::endl;
                std::cout << "task compress len " << compressLen  << " taskNum " << taskNum << std::endl;
                sleep(10); //10s
                std::cout << "task message sleep end 10s" << std::endl;
            }
        });
    }
    end = std::chrono::high_resolution_clock::now();
    used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "yyjson zstd tojson used " << used.count() << "ms" << std::endl;
    std::cout << "yyjson tojson speed " << (length*1213)/(1024*1024.0) << "mb" << std::endl;
    folly::CPUThreadPoolExecutor* executor = reinterpret_cast<folly::CPUThreadPoolExecutor*>(folly::getGlobalCPUExecutor().get());
    std::cout << "yyjson tojson task queue size " << executor->getTaskQueueSize()<< std::endl;
    executor->join();
    std::cout << "yyjson tojson task queue size " << executor->getTaskQueueSize()<< std::endl;
    
  
  
    
    return 0;
}
