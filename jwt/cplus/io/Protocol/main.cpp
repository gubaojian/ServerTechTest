#include <iostream>
#include <iostream>
#include <map>
#include <memory>
#include <thread>

#include <yyjson.h>
#include <turbob64.h>

#include "pack_unpack_protocol.h"
#include "BS_thread_pool.hpp"
#include "concurrentqueue.h"
#include "big_heap_string_view_pool.h"
#include "string_view_in_block.h"
#include "lib/concurrentqueue/concurrentqueue.h"


namespace test {

        constexpr size_t poolBufferSize = 256*1024;
        thread_local std::shared_ptr<char[]> encodeBase64Buffer = std::make_shared<char[]>(poolBufferSize);
        thread_local std::shared_ptr<char[]> encodeJsonBuffer =std::make_shared<char[]>(poolBufferSize);


        thread_local std::shared_ptr<std::stack<std::shared_ptr<char[]>>> decodeBufferPool = std::make_shared<std::stack<std::shared_ptr<char[]>>>();


        class JsonProtocolPacker {
            public:
                std::shared_ptr<std::string> packText(const std::string_view& message, const std::string& connId, const std::string& wsgId) {

                    yyjson_alc alc;
                    char* jsonBuf = (char*)encodeJsonBuffer.get();
                    yyjson_alc_pool_init(&alc, jsonBuf, poolBufferSize);

                    yyjson_mut_doc *doc = yyjson_mut_doc_new(&alc);
                    yyjson_mut_val *jsonRoot = yyjson_mut_obj(doc);
                    yyjson_mut_doc_set_root(doc, jsonRoot);
                    yyjson_mut_obj_add_strn(doc, jsonRoot, "wsgId", wsgId.data(), wsgId.size());
                    yyjson_mut_obj_add_strn(doc, jsonRoot, "connId", connId.data(),
                                        connId.size());
                    yyjson_mut_obj_add_str(doc, jsonRoot, "action", "textMsg");
                    yyjson_mut_obj_add_strn(doc, jsonRoot, "msg", message.data(),
                                        message.size());

                    size_t len;
                    char *packJson = yyjson_mut_write_opts(doc, 0, &alc, &len, nullptr);
                    std::shared_ptr<std::string> packMessage;
                    if (packJson) {
                        packMessage = std::make_shared<std::string>(packJson, len);
                        alc.free(alc.ctx, packJson);
                    }
                    yyjson_mut_doc_free(doc);
                    return packMessage;
                }

                std::shared_ptr<std::string> packBinary(const std::string_view& message, const std::string& connId, const std::string& wsgId) {

                    unsigned char *in = (unsigned char *)message.data();
                    unsigned char *out = (unsigned char *)encodeBase64Buffer.get();
                    size_t base64Len = tb64enc(in, message.size(), out);


                    yyjson_alc alc;
                    char* jsonBuf = (char*)encodeJsonBuffer.get();
                    yyjson_alc_pool_init(&alc, jsonBuf, poolBufferSize);

                    yyjson_mut_doc *doc = yyjson_mut_doc_new(&alc);
                    yyjson_mut_val *jsonRoot = yyjson_mut_obj(doc);
                    yyjson_mut_doc_set_root(doc, jsonRoot);


                    yyjson_mut_obj_add_strn(doc, jsonRoot, "wsgId", wsgId.data(), wsgId.size());
                    yyjson_mut_obj_add_strn(doc, jsonRoot, "connId", connId.data(),
                                        connId.size());
                    yyjson_mut_obj_add_str(doc, jsonRoot, "action", "binaryMsg");
                    yyjson_mut_obj_add_strn(doc, jsonRoot, "msg", (const char*)out, base64Len);

                    size_t len;
                    char *packJson = yyjson_mut_write_opts(doc, 0, &alc, &len, nullptr);
                    std::shared_ptr<std::string> packMessage;
                    if (packJson) {
                        packMessage = std::make_shared<std::string>(packJson, len);
                        alc.free(alc.ctx, packJson);
                    }

                    yyjson_mut_doc_free(doc);
                    return packMessage;
                }
        };


        //采用共享内存，用过后立即释放，不要同时初始化很多例子。
        class JsonProtocolUnPacker {
            public:
               JsonProtocolUnPacker(std::string_view packMessage) {
                   if (decodeBufferPool->empty()) {
                       innerDecodeJsonBuffer = std::make_shared<char[]>(poolBufferSize);
                   } else {
                       innerDecodeJsonBuffer = decodeBufferPool->top();
                       decodeBufferPool->pop();
                   }
                   yyjson_alc alc;
                   char* jsonBuf = (char*)innerDecodeJsonBuffer.get();
                   yyjson_alc_pool_init(&alc, jsonBuf, poolBufferSize);

                   yyjson_read_flag flg = 0;
                   flg &= ~YYJSON_READ_INSITU;
                   doc = yyjson_read_opts((char*)(packMessage.data()),
                                          packMessage.size(), flg, &alc, NULL);
                   if (doc != NULL) {
                       yyjson_val *rootVal = yyjson_doc_get_root(doc);
                       yyjson_val *wsgIdVal = yyjson_obj_get(rootVal, "wsgId");
                       yyjson_val *connIdVal = yyjson_obj_get(rootVal, "connId");
                       yyjson_val *actionVal = yyjson_obj_get(rootVal, "action");
                       yyjson_val *msgVal = yyjson_obj_get(rootVal, "msg");
                       if (wsgIdVal == nullptr
                            || connIdVal == nullptr
                            || actionVal == nullptr
                            || msgVal == nullptr) {
                            return;
                        }
                        wsgId = std::string_view(yyjson_get_str(wsgIdVal), yyjson_get_len(wsgIdVal));
                        connId = std::string_view(yyjson_get_str(connIdVal), yyjson_get_len(connIdVal));
                        action = std::string_view(yyjson_get_str(actionVal), yyjson_get_len(actionVal));
                        if (action == "binaryMsg") {
                            if (decodeBufferPool->empty()) {
                                innerDecodeBase64Buffer = std::make_shared<char[]>(poolBufferSize);
                            } else {
                                innerDecodeBase64Buffer = decodeBufferPool->top();
                                decodeBufferPool->pop();
                            }
                            const size_t msgSize = yyjson_get_len(msgVal);
                            unsigned char *in = (unsigned char *)yyjson_get_str(msgVal);
                            unsigned char *out = (unsigned char *)innerDecodeBase64Buffer.get();
                            size_t base64Len = tb64dec(in, msgSize, out);
                            if (base64Len > 0) {
                                message = std::string_view((const char*)out, base64Len);
                            }
                        } else {
                           message = std::string_view(yyjson_get_str(msgVal), yyjson_get_len(msgVal));
                        }

                   }
               }

               ~JsonProtocolUnPacker() {
                   if (innerDecodeJsonBuffer) {
                       decodeBufferPool->emplace(innerDecodeJsonBuffer);
                       innerDecodeJsonBuffer = nullptr;
                   }

                   if (innerDecodeBase64Buffer) {
                       decodeBufferPool->emplace(innerDecodeBase64Buffer);
                       innerDecodeBase64Buffer = nullptr;
                   }
                   if (doc != nullptr) {
                       yyjson_doc_free(doc);
                       doc = nullptr;
                   }
               }
            public:
                std::string_view wsgId;
                std::string_view connId;
                std::string_view action;
                std::string_view message;
            private:
               yyjson_doc *doc = nullptr;
               std::shared_ptr<char[]> innerDecodeJsonBuffer = nullptr;
               std::shared_ptr<char[]> innerDecodeBase64Buffer = nullptr;
         };
}


void testPacker() {
    auto start = std::chrono::high_resolution_clock::now();
    auto end = std::chrono::high_resolution_clock::now();
    auto used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    test::JsonProtocolPacker packer;
    std::string message(1024, 'a');
    std::string wsgId = "wsgId3332";
    std::string connId = "2235823572375_3332_38888";
    start = std::chrono::high_resolution_clock::now();
    for(int i=0; i<10000*200; i++) {
        packer.packBinary(message, connId, wsgId);
    }
    end = std::chrono::high_resolution_clock::now();
    used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "pack binary used" << used.count() << "ms" << std::endl;


    start = std::chrono::high_resolution_clock::now();
    for(int i=0; i<10000*200; i++) {
        packer.packText(message, connId, wsgId);
    }
    end = std::chrono::high_resolution_clock::now();
    used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "pack text used" << used.count() << "ms" << std::endl;


    auto packBinary = packer.packBinary(message, connId, wsgId);
    std::cout << "length " << packBinary->size() << " "  << *packBinary << std::endl;
    auto packText = packer.packText(message, connId, wsgId);
    std::cout << "length " << packText->size() << " " << *packText << std::endl;


    start = std::chrono::high_resolution_clock::now();
    for(int i=0; i<10000*200; i++) {
        test::JsonProtocolUnPacker unPacker(*packText);
    }
    end = std::chrono::high_resolution_clock::now();
    used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "unpack text used" << used.count() << "ms" << std::endl;

    start = std::chrono::high_resolution_clock::now();
    for(int i=0; i<10000*200; i++) {
        test::JsonProtocolUnPacker unPacker(*packBinary);
    }
    end = std::chrono::high_resolution_clock::now();
    used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "unpack binary used" << used.count() << "ms" << std::endl;

    test::JsonProtocolUnPacker unPackerText(*packText);
    test::JsonProtocolUnPacker unPackerBinary(*packBinary);

    std::cout << "unpack " << unPackerText.message << std::endl;
    std::cout << "unpack " << unPackerBinary.message << std::endl;
}

void testWsgPacker() {
     auto start = std::chrono::high_resolution_clock::now();
    auto end = std::chrono::high_resolution_clock::now();
    auto used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    wsg::gateway::PackProtocol packer;
    std::string message(1024, 'a');
    std::string wsgId = "wsgId3332";
    std::string connId = "2235823572375_3332_38888";
    start = std::chrono::high_resolution_clock::now();
    for(int i=0; i<10000*200; i++) {
        packer.autoPackBinary(message, connId, wsgId);
    }
    end = std::chrono::high_resolution_clock::now();
    used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "wsg pack binary used" << used.count() << "ms" << std::endl;


    start = std::chrono::high_resolution_clock::now();
    for(int i=0; i<10000*200; i++) {
        packer.autoPackText(message, connId, wsgId);
    }
    end = std::chrono::high_resolution_clock::now();
    used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "wsg pack text used" << used.count() << "ms" << std::endl;


    auto packBinary = packer.autoPackBinary(message, connId, wsgId);
    std::cout << "length " << packBinary->size() << " "  << *packBinary << std::endl;
    auto packText = packer.autoPackText(message, connId, wsgId);
    std::cout << "length " << packText->size() << " " << *packText << std::endl;

    std::cout << "wsg unpack text length " << packText->size()  << std::endl;
    start = std::chrono::high_resolution_clock::now();
    for(int i=0; i<10000*200; i++) {
        wsg::gateway::UnPackProtocol unPacker(*packText);
    }
    end = std::chrono::high_resolution_clock::now();
    used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "wsg unpack text used" << used.count() << "ms" << std::endl;

    std::cout << "wsg unpack binary length " << packBinary->size()  << std::endl;
    start = std::chrono::high_resolution_clock::now();
    for(int i=0; i<10000*200; i++) {
        wsg::gateway::UnPackProtocol unPacker(*packBinary);
    }
    end = std::chrono::high_resolution_clock::now();
    used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "wsg unpack binary used" << used.count() << "ms" << std::endl;

    wsg::gateway::UnPackProtocol unPackerText(*packText);
    wsg::gateway::UnPackProtocol unPackerBinary(*packBinary);

    std::cout << "wsg unpack " << unPackerText.message << std::endl;
    std::cout << "wsg unpack " << unPackerBinary.message << std::endl;
}

void learn_pool() {
    auto start = std::chrono::high_resolution_clock::now();
    auto end = std::chrono::high_resolution_clock::now();
    auto used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    BS::thread_pool pool(1); //默认构造函数有问题

    auto message = std::make_shared<std::string>(1024, 'a');

    start = std::chrono::high_resolution_clock::now();
    for(int i=0; i<10000*200; i++) {

        pool.submit_task([message] {
           wsg::gateway::PackProtocol packer;
           std::string wsgId = "wsgId3332";
          std::string connId = "2235823572375_3332_38888";
           packer.autoPackBinary(*message, connId, wsgId);
       });
    }
    end = std::chrono::high_resolution_clock::now();
    used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "pool submit task used " << used.count() << " ms " << std::endl;


}







void testStringBlockSubView() {
     auto start = std::chrono::high_resolution_clock::now();
    auto end = std::chrono::high_resolution_clock::now();
    auto used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    wsg::gateway::PackProtocol packer;
    std::string message(1024, 'a');
    std::shared_ptr<std::string> block = std::make_shared<std::string>(message);
    start = std::chrono::high_resolution_clock::now();
    for(int i=0; i<10000*200; i++) {
        std::string_view messageViewInBlock = std::string_view(block->data() + 8,  512);
        std::make_shared<std::string>(messageViewInBlock);
    }
    end = std::chrono::high_resolution_clock::now();
    used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "make string shared ptr " << used.count() << "ms" << std::endl;


    start = std::chrono::high_resolution_clock::now();
    for(int i=0; i<10000*200; i++) {
        std::string_view messageViewInBlock = std::string_view(block->data() + 8,  512);
        StringViewInBlock(block, messageViewInBlock);
    }
    end = std::chrono::high_resolution_clock::now();
    used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "StringBlockSubView used " << used.count() << "ms" << std::endl;

}

int64_t consumeCount = 0;
std::mutex mutex;
void testProduceOneConsume() {
    auto start = std::chrono::high_resolution_clock::now();
    auto end = std::chrono::high_resolution_clock::now();
    std::shared_ptr<std::queue<std::shared_ptr<std::string>>> queue;
    queue = std::make_shared<std::queue<std::shared_ptr<std::string>>>();
    std::thread consumeThread([queue] {
          while (consumeCount < 10000*200) {
              {
                  std::lock_guard<std::mutex> lock(mutex);
                  while (!queue->empty()) {
                     queue->pop();
                     consumeCount++;
                 }
              }

              std::this_thread::sleep_for(std::chrono::microseconds(50));
          }
    });
    std::string message(1024, 'a');
    start = std::chrono::high_resolution_clock::now();
    for(int i=0; i<10000*200; i++) {
        std::lock_guard<std::mutex> lock(mutex);
        queue->push(std::make_shared<std::string>(message));
    }
    consumeThread.join();
    end = std::chrono::high_resolution_clock::now();
    auto used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "std::make_shared<std::string> used " << used.count() << "ms" << std::endl;
}

struct Message {
    std::string_view messageView;
    std::shared_ptr<std::string> message;
    bool messageViewFromHeap;
};

int64_t consumeCount2 = 0;
std::mutex mutex2;
void testProduceOneConsume2() {
    auto start = std::chrono::high_resolution_clock::now();
    auto end = std::chrono::high_resolution_clock::now();
    std::shared_ptr<std::queue<Message>> queue;
    BigHeapStringViewPool pool(64*1024*1024);
    BigHeapStringViewPool* poolRef = &pool;
    queue = std::make_shared<std::queue<Message>>();
    std::thread consumeThread([queue, poolRef] {
          while (consumeCount2 < 10000*200) {
              {
                  std::lock_guard<std::mutex> lock(mutex2);
                  while (!queue->empty()) {
                      Message msg = queue->front();
                      if (msg.messageViewFromHeap) {
                          poolRef->deallocateStringViewInPool(msg.messageView);
                      }
                      consumeCount2++;
                      queue->pop();
                  }
              }

              std::this_thread::sleep_for(std::chrono::microseconds(50));
          }
    });
    std::string message(1024, 'a');

    start = std::chrono::high_resolution_clock::now();
    for(int i=0; i<10000*200; i++) {
        std::lock_guard<std::mutex> lock(mutex2);
        Message msg;
        msg.messageView = pool.allocateStringViewInPool(message);
        msg.messageViewFromHeap = !msg.messageView.empty();
        if (msg.messageViewFromHeap) {
            msg.message = std::make_shared<std::string>(message);
        }
        queue->push(msg);
    }
    consumeThread.join();
    end = std::chrono::high_resolution_clock::now();
    auto used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "BigHeapStringViewPool used " << used.count() << "ms" << std::endl;
    std::cout << "pool.createPoolStringView  " << pool.getTotalGet() << " hit " << pool.getCacheHit() << std::endl;

}


int64_t consumeCount3 = 0;
void testProduceOneConsume3() {
    auto start = std::chrono::high_resolution_clock::now();
    auto end = std::chrono::high_resolution_clock::now();
    std::shared_ptr<moodycamel::ConcurrentQueue<Message>> queue;
    BigHeapStringViewPool pool(64*1024*1024);
    BigHeapStringViewPool* poolRef = &pool;
    queue = std::make_shared<moodycamel::ConcurrentQueue<Message>>();
    std::thread consumeThread([queue, poolRef] {
          while (consumeCount3 < 10000*200) {
              {
                  bool found = false;
                 do {
                     Message msg;
                     found = queue->try_dequeue(msg);
                     if (found) {
                         if (msg.messageViewFromHeap) {
                             poolRef->deallocateStringViewInPool(msg.messageView);
                         }
                         consumeCount3++;
                     }
                 } while (found);

              }
              std::this_thread::sleep_for(std::chrono::microseconds(50));
          }
    });
    std::string message(1024, 'a');

    start = std::chrono::high_resolution_clock::now();
    for(int i=0; i<10000*200; i++) {
        Message msg;
        msg.messageView = pool.allocateStringViewInPool(message);
        msg.messageViewFromHeap = !msg.messageView.empty();
        if (!msg.messageViewFromHeap) {
            msg.message = std::make_shared<std::string>(message);
        }
        queue->enqueue(msg);
    }
    auto end2 = std::chrono::high_resolution_clock::now();
    auto used2 = std::chrono::duration_cast<std::chrono::milliseconds>(end2 - start);
    std::cout << "BigBlockStringPool with lock free queue submit " << used2.count() << "ms" << std::endl;
    std::cout << "pool.createPoolStringView  " << pool.getTotalGet() << " hit " << pool.getCacheHit() << std::endl;
    consumeThread.join();
    end = std::chrono::high_resolution_clock::now();
    auto used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "BigBlockStringPool with lock free queue used " << used.count() << "ms" << std::endl;


}


int64_t consumeCount5 = 0;
void testProduceOneConsume5() {
    auto start = std::chrono::high_resolution_clock::now();
    auto end = std::chrono::high_resolution_clock::now();
    std::shared_ptr<moodycamel::ConcurrentQueue<Message>> queue;
    BigHeapStringViewPool pool(64*1024*1024);
    BigHeapStringViewPool* poolRef = &pool;
    queue = std::make_shared<moodycamel::ConcurrentQueue<Message>>();
    std::thread consumeThread([queue, poolRef] {
          while (consumeCount5 < 10000*200) {
              {
                  bool found = false;
                 do {
                     Message msg;
                     found = queue->try_dequeue(msg);
                     if (found) {
                         if (msg.messageViewFromHeap) {
                             poolRef->deallocateStringViewInPool(msg.messageView);
                         }
                         consumeCount5++;
                     }
                 } while (found);

              }
              std::this_thread::sleep_for(std::chrono::microseconds(50));
          }
    });
    std::string message(512, 'a');

    start = std::chrono::high_resolution_clock::now();
    for(int i=0; i<10000*200; i++) {
        Message msg;
        msg.messageView = pool.allocateStringViewInPool(message);
        msg.messageViewFromHeap = !msg.messageView.empty();
        if (!msg.messageViewFromHeap) {
            msg.message = std::make_shared<std::string>(message);
        }
        queue->enqueue(msg);
    }
    auto end2 = std::chrono::high_resolution_clock::now();
    auto used2 = std::chrono::duration_cast<std::chrono::milliseconds>(end2 - start);
    std::cout << "BigBlockStringPool with lock free queue submit " << used2.count() << "ms" << std::endl;
    std::cout << "pool.createPoolStringView  " << pool.getTotalGet() << " hit " << pool.getCacheHit() << std::endl;
    consumeThread.join();
    end = std::chrono::high_resolution_clock::now();
    auto used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "BigBlockStringPool with lock free queue 512 byte  used " << used.count() << "ms" << std::endl;
}


int64_t consumeCount4 = 0;
void testProduceOneConsume4() {
    auto start = std::chrono::high_resolution_clock::now();
    auto end = std::chrono::high_resolution_clock::now();
    std::shared_ptr<moodycamel::ConcurrentQueue<std::shared_ptr<std::string>>> queue;
    queue = std::make_shared<moodycamel::ConcurrentQueue<std::shared_ptr<std::string>>>();
    std::thread consumeThread([queue] {
          while (consumeCount4 < 10000*200) {
              {
                  bool found = false;
                 do {
                     std::shared_ptr<std::string> message;
                     found = queue->try_dequeue(message);
                     if (found) {
                         consumeCount4++;
                     }
                 } while (found);
              }

              std::this_thread::sleep_for(std::chrono::microseconds(50));
          }
    });
    std::string message(1024, 'a');
    start = std::chrono::high_resolution_clock::now();
    for(int i=0; i<10000*200; i++) {
         queue->enqueue(std::make_shared<std::string>(message));
    }
    consumeThread.join();
    end = std::chrono::high_resolution_clock::now();
    auto used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "std::make_shared<std::string> lock free queue used " << used.count() << "ms" << std::endl;
}

int64_t consumeCount6 = 0;
void testProduceOneConsume6() {
    auto start = std::chrono::high_resolution_clock::now();
    auto end = std::chrono::high_resolution_clock::now();
    std::shared_ptr<moodycamel::ConcurrentQueue<std::shared_ptr<std::string>>> queue;
    queue = std::make_shared<moodycamel::ConcurrentQueue<std::shared_ptr<std::string>>>();
    std::thread consumeThread([queue] {
          while (consumeCount6 < 10000*200) {
              {
                  bool found = false;
                 do {
                     std::shared_ptr<std::string> message;
                     found = queue->try_dequeue(message);
                     if (found) {
                         consumeCount6++;
                     }
                 } while (found);
              }

              std::this_thread::sleep_for(std::chrono::microseconds(50));
          }
    });
    std::string message(512, 'a');
    start = std::chrono::high_resolution_clock::now();
    for(int i=0; i<10000*200; i++) {
        queue->enqueue(std::make_shared<std::string>(message));
    }
    consumeThread.join();
    end = std::chrono::high_resolution_clock::now();
    auto used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "std::make_shared<std::string> lock free queue 512 byte used " << used.count() << "ms" << std::endl;
}


void testPoolOnly() {
    auto start = std::chrono::high_resolution_clock::now();
    auto end = std::chrono::high_resolution_clock::now();
    std::string message(1024, 'a');

    std::vector<std::shared_ptr<std::string>> messages;
    start = std::chrono::high_resolution_clock::now();
    for(int i=0; i<10000*200; i++) {
        messages.push_back(std::make_shared<std::string>(message));
    }
    end = std::chrono::high_resolution_clock::now();
    auto used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "std::make_shared<std::string> used " << used.count() << "ms" << std::endl;

    /**
    BigHeapStringViewPool pool(32*1024*1024);
    StringViewInBigHeap poolStringView("");
    std::vector<StringViewInBigHeap> messages2;
    start = std::chrono::high_resolution_clock::now();
    for(int i=0; i<10000*200; i++) {
        poolStringView = pool.createStringViewInPool(message);
        messages2.push_back(poolStringView);
        pool.releaseStringViewInPool(poolStringView);
    }
    end = std::chrono::high_resolution_clock::now();
     used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "pool.createPoolStringView used " << used.count() << "ms" << std::endl;
    */
    //std::cout << "pool.createPoolStringView  " << pool.getTotalGet() << " hit " << pool.getCacheHit() << std::endl;
}



// TIP 要<b>Run</b>代码，请按 <shortcut actionId="Run"/> 或点击装订区域中的 <icon src="AllIcons.Actions.Execute"/> 图标。
int main() {

    //testPacker();
    std::cout << "wsg enableBinaryKV ----------------- "<< std::endl;
    //testWsgPacker();
    std::cout << "wsg disable enableBinaryKV ----------------- "  << std::endl;
    //wsg::gateway::enableBinaryKV = false;
    //testWsgPacker();

    //learn_pool();

    //testStringBlockSubView();


    //testProduceOneConsume();

    //testProduceOneConsume2();


    testProduceOneConsume4();

    testProduceOneConsume3();

    testProduceOneConsume6();

    testProduceOneConsume5();

   // testPoolOnly();

    testStringBlockSubView();


    return 0;
    // TIP 请访问 <a href="https://www.jetbrains.com/help/clion/">jetbrains.com/help/clion/</a> 查看 CLion 帮助。此外，您还可以从主菜单中选择“帮助 | 学习 IDE 功能”，尝试 CLion 的交互式课次。
}