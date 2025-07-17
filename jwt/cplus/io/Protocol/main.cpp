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


class StringBlockSubView {
public:
    explicit StringBlockSubView(const std::shared_ptr<std::string>& message) {
        block = message;
        messageView = *message;
    }
    explicit StringBlockSubView(const std::shared_ptr<std::string>& block, const std::string_view& messageViewInBlock) {
        this->block = block;
        messageView = messageViewInBlock;
    }
    public:
      std::string_view messageView;
    private:
      std::shared_ptr<std::string> block;
};

class PoolStringView {
public:
    explicit PoolStringView(const std::string_view& messageView, bool fromPool = true) {
        this->fromPool = fromPool;
        if (fromPool) {
            this->messageView = messageView;
        } else {
            if (!messageView.empty()) {
                block = std::make_shared<std::string>(messageView);
                this->messageView = *block;
            }
        }
        if (messageView.empty()) {
            this->fromPool = false;
        }
    }
public:
    std::string_view messageView;
    bool fromPool;
    std::shared_ptr<std::string> block = nullptr; //only valid
};

 /**
 * one thread alloc, one thread consume and then return pool return
 */
class BigBlockStringPool {
public:
    explicit BigBlockStringPool(int size) {
        buffer = new char[size + 4*1024]; //add padding to avoid check array bounds
        this->pooSize = size;
        allocOffset = 0;
        returnOffset = pooSize;
    }
    ~BigBlockStringPool() {
        if (buffer != nullptr) {
            delete[] buffer;
            buffer = nullptr;
        }
    }
public:
    PoolStringView createPoolStringView(const std::string& message) {
        return createPoolStringView(message.data(), message.size());
    }
    PoolStringView createPoolStringView(const char* data, size_t length) {
        if (length > 2*1024) {
            return PoolStringView({data, length}, false);
        }
        std::lock_guard<std::mutex> lock(mutex);
        int remainSize = returnOffset - allocOffset;
        int offset = allocOffset%pooSize;
        if (remainSize > length) {
            char* from = buffer + offset;
            memcpy(from, data, length);
            allocOffset += length;
            return PoolStringView({from, length});
        } else {
            return PoolStringView({data, length}, false);
        }
    }
    void returnPoolStringView(const PoolStringView& message) {
        if (message.fromPool) {
            std::lock_guard<std::mutex> lock(mutex);
            returnOffset += message.messageView.size();
            int64_t remainSize = returnOffset - allocOffset;
            if (remainSize > pooSize) { //illegal return, ignore it
                returnOffset -= message.messageView.size();
            }
            if (returnOffset > pooSize*128) {
                returnOffset -= pooSize*64;
                allocOffset -= pooSize*64;
            }
        }
    }
private:
    char* buffer = nullptr;
    std::mutex mutex;
    std::atomic<int64_t> allocOffset;
    std::atomic<int64_t> returnOffset;
    int64_t pooSize = 0;
};

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
        StringBlockSubView(block, messageViewInBlock);
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

int64_t consumeCount2 = 0;
std::mutex mutex2;
void testProduceOneConsume2() {
    auto start = std::chrono::high_resolution_clock::now();
    auto end = std::chrono::high_resolution_clock::now();
    std::shared_ptr<std::queue<PoolStringView>> queue;
    BigBlockStringPool pool(64*1024*1024);
    BigBlockStringPool* poolRef = &pool;
    queue = std::make_shared<std::queue<PoolStringView>>();
    std::thread consumeThread([queue, poolRef] {
          while (consumeCount2 < 10000*200) {
              bool needBreak = false;
              for (int i=0; i<10000*200; i++)
              {
                  std::lock_guard<std::mutex> lock(mutex2);
                  PoolStringView view("");
                  for (int m=0; m<256; m++) {
                      if (!queue->empty()) {
                        view = queue->front();
                        queue->pop();
                     } else {
                         needBreak = true;
                     }
                      if (!view.messageView.empty()) {
                          poolRef->returnPoolStringView(view);
                         consumeCount2++;
                      }
                      if (needBreak) {
                        break;
                     }
                  }
              }

              std::this_thread::sleep_for(std::chrono::microseconds(50));
          }
    });
    std::string message(1024, 'a');

    start = std::chrono::high_resolution_clock::now();
    for(int i=0; i<10000; i++) {
        std::lock_guard<std::mutex> lock(mutex2);
        for (int j=0; j<200; j++) {
            queue->push(pool.createPoolStringView(message));
        }
    }
    consumeThread.join();
    end = std::chrono::high_resolution_clock::now();
    auto used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "BigBlockStringPool used " << used.count() << "ms" << std::endl;
}


int64_t consumeCount3 = 0;
std::mutex mutex3;
void testProduceOneConsume3() {
    auto start = std::chrono::high_resolution_clock::now();
    auto end = std::chrono::high_resolution_clock::now();
    std::shared_ptr<moodycamel::ConcurrentQueue<PoolStringView>> queue;
    BigBlockStringPool pool(64*1024*1024);
    BigBlockStringPool* poolRef = &pool;
    queue = std::make_shared<moodycamel::ConcurrentQueue<PoolStringView>>();
    std::thread consumeThread([queue, poolRef] {
          while (consumeCount3 < 10000*200) {
              {
                  bool found = false;
                 do {
                     PoolStringView poolStringView("");
                     found = queue->try_dequeue(poolStringView);
                     if (found) {
                         poolRef->returnPoolStringView(poolStringView);
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
        queue->enqueue(pool.createPoolStringView(message));
    }
    consumeThread.join();
    end = std::chrono::high_resolution_clock::now();
    auto used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "BigBlockStringPool with lock free queue used " << used.count() << "ms" << std::endl;
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


    testProduceOneConsume();

    testProduceOneConsume2();


    testProduceOneConsume4();

    testProduceOneConsume3();



    return 0;
    // TIP 请访问 <a href="https://www.jetbrains.com/help/clion/">jetbrains.com/help/clion/</a> 查看 CLion 帮助。此外，您还可以从主菜单中选择“帮助 | 学习 IDE 功能”，尝试 CLion 的交互式课次。
}