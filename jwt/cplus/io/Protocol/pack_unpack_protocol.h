//
// Created by efurture on 25-7-4.
//

#ifndef PACK_UNPACK_PROTOCOL_H
#define PACK_UNPACK_PROTOCOL_H
#include "yyjson.h"
#include <string>
#include <string_view>
#include <vector>
#include <map>
#include <set>
#include <iostream>
#include <queue>
#include <deque>
#include <memory>
#include <random>
#include <cmath>
#include <ctime>
#include <climits>
#include <cstdlib>
#include <iomanip>
#include <sstream>
#include <thread>
#include "turbob64.h"
#include "io_output_input.h"

namespace wsg {
    namespace gateway {
        //128*1024 is enough， websocket max message is 64*1024. 64kb binary base64 maxsize 86kb
        constexpr size_t poolBufferSize = 128 * 1024;
        thread_local std::shared_ptr<char[]> encodeBase64Buffer = std::make_shared<char[]>(poolBufferSize);
        thread_local std::shared_ptr<char[]> encodeJsonBuffer = std::make_shared<char[]>(poolBufferSize);

        thread_local std::shared_ptr<char[]> kvEncodeBuffer = std::make_shared<char[]>(poolBufferSize);


        thread_local std::shared_ptr<std::stack<std::shared_ptr<char[]> > > decodeBufferPool = std::make_shared<
            std::stack<std::shared_ptr<char[]> > >();


        constexpr std::string actionText = "text";
        constexpr std::string actionBinary = "binary";


        //2-8法则，关键的频繁的消息用binaryKV，
        //其它统一json，比如订阅，发布广播等。
        bool enableBinaryKV = true;


   class PackProtocol {
        public:
           std::shared_ptr<std::string> autoPackText(const std::string_view &message, const std::string &connId,
                                                      const std::string &wsgId) {
               if (enableBinaryKV) {
                   return binaryKVPackText(message, connId, wsgId);
               }
               return jsonPackText(message, connId, wsgId);
           }

           std::shared_ptr<std::string> autoPackBinary(const std::string_view &message, const std::string &connId,
                                                   const std::string &wsgId) {
               if (enableBinaryKV) {
                   return binaryKVPackBinary(message, connId, wsgId);
               }
               return jsonPackBinary(message, connId, wsgId);
           }

        private:
          std::shared_ptr<std::string> jsonPackText(const std::string_view &message, const std::string &connId,
                                                      const std::string &wsgId) {
                yyjson_alc alc;
                char *jsonBuf = (char *) encodeJsonBuffer.get();
                yyjson_alc_pool_init(&alc, jsonBuf, poolBufferSize);

                yyjson_mut_doc *doc = yyjson_mut_doc_new(&alc);
                yyjson_mut_val *jsonRoot = yyjson_mut_obj(doc);
                yyjson_mut_doc_set_root(doc, jsonRoot);
                yyjson_mut_obj_add_strn(doc, jsonRoot, "wsgId", wsgId.data(), wsgId.size());
                yyjson_mut_obj_add_strn(doc, jsonRoot, "connId", connId.data(),
                                        connId.size());
                yyjson_mut_obj_add_str(doc, jsonRoot, "action", "text");
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

            std::shared_ptr<std::string> jsonPackBinary(const std::string_view &message, const std::string &connId,
                                                        const std::string &wsgId) {
                unsigned char *in = (unsigned char *) message.data();
                unsigned char *out = (unsigned char *) encodeBase64Buffer.get();
                size_t base64Len = tb64enc(in, message.size(), out);


                yyjson_alc alc;
                char *jsonBuf = (char *) encodeJsonBuffer.get();
                yyjson_alc_pool_init(&alc, jsonBuf, poolBufferSize);


                yyjson_mut_doc *doc = yyjson_mut_doc_new(&alc);
                yyjson_mut_val *jsonRoot = yyjson_mut_obj(doc);
                yyjson_mut_doc_set_root(doc, jsonRoot);


                yyjson_mut_obj_add_strn(doc, jsonRoot, "wsgId", wsgId.data(), wsgId.size());
                yyjson_mut_obj_add_strn(doc, jsonRoot, "connId", connId.data(),
                                        connId.size());
                yyjson_mut_obj_add_str(doc, jsonRoot, "action", "binary");
                yyjson_mut_obj_add_strn(doc, jsonRoot, "msg", (const char *) out, base64Len);

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
       private:
           std::shared_ptr<std::string> binaryKVPackText(const std::string_view &message, const std::string &connId,
                                                          const std::string &wsgId) {
               char *buffer = (char *) encodeJsonBuffer.get();
               Output output(buffer, 0);

               //协议头部及版本号
               output.writeByte( 'b');
               output.writeByte(0);

               // 按kv顺序写入内容, 用一个字节代表key，这也方便扩展和调整顺序也不影响。
               // 注意k不要重复，unpack中增加相应处理方法。
               output.writeByte('w');
               output.writeBinary(wsgId);

               output.writeByte('c');
               output.writeBinary(connId);

               output.writeByte('a');
               output.writeBinary("t");

               output.writeByte('m');
               output.writeBinary(message);

               return std::make_shared<std::string>(buffer, output.getPosition());
           }

           std::shared_ptr<std::string> binaryKVPackBinary(const std::string_view &message, const std::string &connId,
                                                     const std::string &wsgId) {
               char *buffer = (char *) encodeJsonBuffer.get();
               Output output(buffer, 0);

               //协议头部及版本号
               output.writeByte( 'b');
               output.writeByte(0);

               // 按kv顺序写入内容, 用一个字节代表key，这也方便扩展和调整顺序也不影响。
               // 注意k不要重复，unpack中增加相应处理方法。
               output.writeByte('w');
               output.writeBinary(wsgId);

               output.writeByte('c');
               output.writeBinary(connId);

               output.writeByte('a');
               output.writeBinary("b");

               output.writeByte('m');
               output.writeBinary(message);

               return std::make_shared<std::string>(buffer, output.getPosition());
           }



        public:
            std::shared_ptr<std::string> jsonPackOnOpenEvent(const std::string &connId, const std::string &wsgId) {
                yyjson_alc alc;
                char *jsonBuf = (char *) encodeJsonBuffer.get();
                yyjson_alc_pool_init(&alc, jsonBuf, poolBufferSize);

                yyjson_mut_doc *doc = yyjson_mut_doc_new(&alc);
                yyjson_mut_val *jsonRoot = yyjson_mut_obj(doc);
                yyjson_mut_doc_set_root(doc, jsonRoot);
                yyjson_mut_obj_add_strn(doc, jsonRoot, "wsgId", wsgId.data(), wsgId.size());
                yyjson_mut_obj_add_strn(doc, jsonRoot, "connId", connId.data(),
                                        connId.size());
                yyjson_mut_obj_add_str(doc, jsonRoot, "action", "onopen");

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

            std::shared_ptr<std::string> jsonPackOnCloseEvent(const std::string &connId, const std::string &wsgId) {
                yyjson_alc alc;
                char *jsonBuf = (char *) encodeJsonBuffer.get();
                yyjson_alc_pool_init(&alc, jsonBuf, poolBufferSize);

                yyjson_mut_doc *doc = yyjson_mut_doc_new(&alc);
                yyjson_mut_val *jsonRoot = yyjson_mut_obj(doc);
                yyjson_mut_doc_set_root(doc, jsonRoot);
                yyjson_mut_obj_add_strn(doc, jsonRoot, "wsgId", wsgId.data(), wsgId.size());
                yyjson_mut_obj_add_strn(doc, jsonRoot, "connId", connId.data(),
                                        connId.size());
                yyjson_mut_obj_add_str(doc, jsonRoot, "action", "onclose");

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

        public:
            static std::shared_ptr<std::string> jsonPackCmdQueryConn(const std::string &connId,
                                                                     const std::string &wsgId) {
                yyjson_alc alc;
                char *jsonBuf = (char *) encodeJsonBuffer.get();
                yyjson_alc_pool_init(&alc, jsonBuf, poolBufferSize);

                yyjson_mut_doc *doc = yyjson_mut_doc_new(&alc);
                yyjson_mut_val *jsonRoot = yyjson_mut_obj(doc);
                yyjson_mut_doc_set_root(doc, jsonRoot);
                yyjson_mut_obj_add_strn(doc, jsonRoot, "wsgId", wsgId.data(), wsgId.size());
                yyjson_mut_obj_add_strn(doc, jsonRoot, "connId", connId.data(),
                                        connId.size());
                yyjson_mut_obj_add_str(doc, jsonRoot, "action", "cmdQueryConnInfo");

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


    class UnPackProtocol {
        public:
            explicit UnPackProtocol(const std::string_view &packMessage) {
                if (!packMessage.empty() && packMessage.at(0) == 'b') {
                    parseBinaryKVUnPack(packMessage);
                } else {
                    parseJsonUnPack(packMessage);
                }
            }

            ~UnPackProtocol() {
                if (innerDecodeJsonBuffer) {
                    if (decodeBufferPool->size() < 16) {
                        decodeBufferPool->emplace(innerDecodeJsonBuffer);
                    }
                    innerDecodeJsonBuffer = nullptr;
                }

                if (innerDecodeBase64Buffer) {
                    if (decodeBufferPool->size() < 16) {
                        decodeBufferPool->emplace(innerDecodeBase64Buffer);
                    }
                    innerDecodeBase64Buffer = nullptr;
                }
                if (doc != nullptr) {
                    yyjson_doc_free(doc);
                    doc = nullptr;
                }
            }

        private:
            void parseBinaryKVUnPack(const std::string_view &packMessage) {
                const char* buffer = packMessage.data();
                if (buffer[0] != 'b') {
                    return;
                }
                uint8_t version = buffer[1];
                Input  input(buffer, 2, packMessage.length());
                while (input.hasNext()) {
                    switch (input.readByte()) {
                        case 'w' :
                            wsgId = input.readBinary();
                            break;
                        case 'c' :
                            connId = input.readBinary();
                            break;
                        case 'a' :
                            {
                                auto v = input.readBinary();
                                //针对text和binary msg特殊优化。
                                if (v.size() == 1) {
                                    if (v[0] == 'b') {
                                        v = actionBinary;
                                    } else if (v[0] == 't') {
                                        v = actionText;
                                    }
                                }

                                action = v;
                            }
                            break;
                        case 'm' :
                            message = input.readBinary();
                            break;
                        default:
                            input.skipBinary();
                            break;
                    }
                }
            }
            void parseJsonUnPack(const std::string_view &packMessage) {
                if (decodeBufferPool->empty()) {
                    innerDecodeJsonBuffer = std::make_shared<char[]>(poolBufferSize);
                } else {
                    innerDecodeJsonBuffer = decodeBufferPool->top();
                    decodeBufferPool->pop();
                }
                yyjson_alc alc;
                char *jsonBuf = (char *) innerDecodeJsonBuffer.get();
                yyjson_alc_pool_init(&alc, jsonBuf, poolBufferSize);

                yyjson_read_flag flg = 0;
                flg &= ~YYJSON_READ_INSITU;
                doc = yyjson_read_opts((char*)(packMessage.data()),
                                       packMessage.size(), flg, &alc, NULL);

                if (doc != nullptr) {
                    yyjson_val *rootVal = yyjson_doc_get_root(doc);
                    yyjson_val *wsgIdVal = yyjson_obj_get(rootVal, "wsgId");
                    yyjson_val *connIdVal = yyjson_obj_get(rootVal, "connId");
                    yyjson_val *actionVal = yyjson_obj_get(rootVal, "action");
                    yyjson_val *msgVal = yyjson_obj_get(rootVal, "msg");

                    if (wsgIdVal != nullptr) {
                        wsgId = std::string_view(yyjson_get_str(wsgIdVal), yyjson_get_len(wsgIdVal));
                    }
                    if (connIdVal != nullptr) {
                        connId = std::string_view(yyjson_get_str(connIdVal), yyjson_get_len(connIdVal));
                    }
                    if (actionVal != nullptr) {
                        action = std::string_view(yyjson_get_str(actionVal), yyjson_get_len(actionVal));
                    }
                    if (action == "text") {
                        if (msgVal != nullptr) {
                            message = std::string_view(yyjson_get_str(msgVal), yyjson_get_len(msgVal));
                        }
                    } else if (action == "binary") {
                        if (msgVal != nullptr) {
                            if (decodeBufferPool->empty()) {
                                innerDecodeBase64Buffer = std::make_shared<char[]>(poolBufferSize);
                            } else {
                                innerDecodeBase64Buffer = decodeBufferPool->top();
                                decodeBufferPool->pop();
                            }
                            const size_t msgSize = yyjson_get_len(msgVal);
                            unsigned char *in = (unsigned char *) yyjson_get_str(msgVal);
                            unsigned char *out = (unsigned char *) innerDecodeBase64Buffer.get();
                            size_t base64Len = tb64dec(in, msgSize, out);
                            if (base64Len > 0) {
                                message = std::string_view((const char *) out, base64Len);
                            }
                        }
                    } else if (action == "publish") {
                        yyjson_val *topicVal = yyjson_obj_get(rootVal, "topic");
                        yyjson_val *msgTypeVal = yyjson_obj_get(rootVal, "msgType");
                        if (topicVal != nullptr) {
                            topic = std::string_view(yyjson_get_str(topicVal), yyjson_get_len(topicVal));
                        }
                        if (msgTypeVal != nullptr) {
                            publishMsgType = std::string_view(yyjson_get_str(msgTypeVal), yyjson_get_len(msgTypeVal));
                        }
                    }  else if (action == "subscribe") {
                        yyjson_val *topicVal = yyjson_obj_get(rootVal, "topic");
                        if (topicVal != nullptr) {
                            topic = std::string_view(yyjson_get_str(topicVal), yyjson_get_len(topicVal));
                        }
                    }  else if (action == "unsubscribe") {
                        yyjson_val *topicVal = yyjson_obj_get(rootVal, "topic");
                        if (topicVal != nullptr) {
                            topic = std::string_view(yyjson_get_str(topicVal), yyjson_get_len(topicVal));
                        }
                    } else {
                        if (msgVal != nullptr) {
                            message = std::string_view(yyjson_get_str(msgVal), yyjson_get_len(msgVal));
                        }
                    }
                }
            }


        public:
            std::string_view wsgId;
            std::string_view connId;
            std::string_view action;
            std::string_view message;
            std::string_view topic;
            std::string_view publishMsgType; //only valid for broadcast

        private:
            yyjson_doc *doc = nullptr;
            std::shared_ptr<char[]> innerDecodeJsonBuffer = nullptr;
            std::shared_ptr<char[]> innerDecodeBase64Buffer = nullptr;
        };
    }
}

#endif //PACK_UNPACK_PROTOCOL_H
