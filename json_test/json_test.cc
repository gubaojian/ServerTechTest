
#include <sstream>
#include <sys/_types/_int64_t.h>
#include <sys/_types/_null.h>
#define RAPIDJSON_HAS_STDSTRING 1
#include "rapidjson-master/include/rapidjson/stringbuffer.h"
#include "yyjson-master/src/yyjson.h"
#include "rapidjson-master/include/rapidjson/rapidjson.h"  // rapidjson's DOM-style API
#include "rapidjson-master/include/rapidjson/prettywriter.h" 
#include "rapidjson-master/include/rapidjson/document.h"
#include "message.pb.h"
#include "wson.h"
#include <cstddef>
#include <iostream>
#include <ostream>
#include <string>
#include <sys/_types/_size_t.h>
#include <utility>
#include <vector>
#include <map>

/***
 data size 160mb
 
yy serialized used 651.924ms
yyjson parse used 347.792ms

rapidjson json used 1188.3ms
rapidjson parse used 719.355ms

 normal cson used 210.685ms
 normal cson parse used 137.191ms

normal pb used 519.65ms
normal pb parse used 423.637ms

*/

 using namespace rapidjson;

namespace json {
     namespace yy {

        std::string toJsonMsg(const std::string& msg) {
            yyjson_mut_doc *doc = yyjson_mut_doc_new(NULL);
            yyjson_mut_val *root = yyjson_mut_obj(doc);
            yyjson_mut_doc_set_root(doc, root);
            std::string connId = "connect-id-111119";
            std::string authId = "auth-id-111119-4rwqrwqrrr";
            const std::string action = "auth";
            yyjson_mut_obj_add_strn(doc, root, "conn-id", connId.c_str(), connId.size());
            yyjson_mut_obj_add_strn(doc, root, "auth-id", authId.c_str(), authId.size());
            yyjson_mut_obj_add_strn(doc, root, "action", action.c_str(), action.length());
            yyjson_mut_obj_add_strn(doc, root, "msg", msg.c_str(), msg.length());
            size_t len = 0;
            yyjson_write_err err;
            // To string, minified
            const char *json = yyjson_mut_write_opts(doc, 0, NULL, &len, &err);
            if (err.code) {
                    printf("write error (%u): %s\n", err.code, err.msg);
            }
            std::string result(json, len);
            if (json) {
                free((void *)json);
            }
            // Free the doc
            yyjson_mut_doc_free(doc);
            return std::move(result);
        }


        void parse(const std::string& msg) {
            yyjson_doc *doc = yyjson_read(msg.data(), msg.length(), 0);
            yyjson_val *root = yyjson_doc_get_root(doc);
            // Free the doc
            yyjson_doc_free(doc);
        }
     }


     namespace rapid {
        std::string toJsonMsg(const std::string& msg) {
            std::string connId = "connect-id-111119";
            std::string authId = "auth-id-111119-4rwqrwqrrr";
            const std::string action = "auth";
            rapidjson::Document document; 
            document.SetObject();
            document.AddMember("conn-id", rapidjson::StringRef(connId.c_str(), connId.length()), document.GetAllocator());
            document.AddMember("auth-id", rapidjson::StringRef(authId.c_str(), authId.length()), document.GetAllocator());
            document.AddMember("action", rapidjson::StringRef(action.c_str(), action.length()), document.GetAllocator());
            document.AddMember("msg", rapidjson::StringRef(msg.c_str(), msg.length()), document.GetAllocator());
            rapidjson::StringBuffer sb;
            rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
            document.Accept(writer);  
            return std::move(std::string(sb.GetString(), sb.GetSize()));
        }

        void parse(const std::string& msg) {
            rapidjson::Document document; 
            document.Parse(msg.data(), msg.length());
        }

     }

     namespace pb {

       std::string toJsonMsg(const std::string& msg) {
            std::string connId = "connect-id-111119";
            std::string authId = "auth-id-111119-4rwqrwqrrr";
            const std::string action = "auth";
            HMessage pmsg;
            pmsg.set_action(action);
            pmsg.set_authid(authId);
            pmsg.set_connid(connId);
            pmsg.set_msg(msg);
            std::string result;
            pmsg.SerializePartialToString(&result);
            return std::move(result);
        }

        void parse(const std::string& msg) {
            HMessage pmsg;
            pmsg.ParseFromString(msg);
        }
     }

     std::string toPson(const std::string& msg) {
        std::string connId = "connect-id-111119";
        std::string authId = "auth-id-111119-4rwqrwqrrr";
        const std::string action = "auth";
        std::ostringstream ss;
        ss << connId;
        ss << authId;
        ss << action;
        ss << msg;
        return std::move(ss.str());
     }

      std::string toCson(const std::string& msg) {
        std::string connId = "connect-id-111119";
        std::string authId = "auth-id-111119-4rwqrwqrrr";
        const std::string action = "auth";
        wson_buffer* buffer =  wson_buffer_new();
        wson_push_type_string(buffer, connId.data(), connId.length());
        wson_push_type_string(buffer, authId.data(), authId.length());
        wson_push_type_string(buffer, action.data(), action.length());
        wson_push_type_string(buffer, msg.data(), msg.length());
        std::string data((char*)buffer->data, buffer->position);
        wson_buffer_free(buffer);
        return std::move(data);
     }

    void parse(const std::string& msg) {
        wson_buffer* buffer =  wson_buffer_from((char*)msg.data(), msg.length());
        
        wson_next_type(buffer);
        int length = wson_next_uint(buffer);
        std::string connId((char*)wson_next_bts(buffer, length), length);
        
        wson_next_type(buffer);
        length = wson_next_uint(buffer);
        std::string authId((char*)wson_next_bts(buffer, length), length);
        
        wson_next_type(buffer);
        length = wson_next_uint(buffer);
        std::string action((char*)wson_next_bts(buffer, length), length);
        
        wson_next_type(buffer);
        length = wson_next_uint(buffer);
        std::string innerMsg((char*)wson_next_bts(buffer, length), length);
        
        buffer->data = NULL;
     }
}

int main() {
    const char *json = "{\"name\":\"Mash\",\"star\":4,\"hits\":[2,2,1,3],\"k\":\"中文\"}";
    std::string msg(json);
    std::string serverMsg = json::yy::toJsonMsg(msg);
    std::cout << serverMsg << std::endl;
    serverMsg = json::rapid::toJsonMsg(msg);
    std::cout << serverMsg << std::endl;
    int64_t bts = 0;
    for(int i=0; i<10000*100; i++) {
         bts +=serverMsg.length();
    }
     std::cout << serverMsg.length() << "data length used " << (bts/(1000*1000)) << "mb" << std::endl;
    auto start = std::chrono::system_clock::now();
    for(int i=0; i<10000*100; i++) {
        serverMsg = json::yy::toJsonMsg(msg);
    }
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double, std::milli>  used = (end - start);
    std::cout << "yy serialized used " << used.count() << "ms" << std::endl;

    start = std::chrono::system_clock::now();
    for(int i=0; i<10000*100; i++) {
        json::yy::parse(msg);
    }
    end = std::chrono::system_clock::now();
    used = (end - start);
    std::cout << "yyjson parse used " << used.count() << "ms" << std::endl;


    start = std::chrono::system_clock::now();
    for(int i=0; i<10000*100; i++) {
        serverMsg = json::rapid::toJsonMsg(msg);
    }
    end = std::chrono::system_clock::now();
    used = (end - start);
    std::cout << "rapidjson json used " << used.count() << "ms" << std::endl;

 
    start = std::chrono::system_clock::now();
    for(int i=0; i<10000*100; i++) {
        json::rapid::parse(msg);
    }
    end = std::chrono::system_clock::now();
    used = (end - start);
    std::cout << "rapidjson parse used " << used.count() << "ms" << std::endl;

 

    std::cout << json::toPson(msg) << std::endl; 

    start = std::chrono::system_clock::now();
    for(int i=0; i<10000*100; i++) {
        serverMsg = json::toPson(msg);
    }
    end = std::chrono::system_clock::now();
    used = (end - start);
    std::cout << "normal json used " << used.count() << "ms" << std::endl;
    std::cout << json::toCson(msg) << std::endl; 
    start = std::chrono::system_clock::now();
    for(int i=0; i<10000*100; i++) {
        serverMsg = json::toCson(msg);
    }
    end = std::chrono::system_clock::now();
    used = (end - start);
    std::cout << "normal cson used " << used.count() << "ms" << std::endl;

    json::parse(serverMsg); 

    start = std::chrono::system_clock::now();
    for(int i=0; i<10000*100; i++) {
        json::parse(serverMsg);
    }
    end = std::chrono::system_clock::now();
    used = (end - start);
    std::cout << "normal cson parse used " << used.count() << "ms" << std::endl;


     std::cout << json::pb::toJsonMsg(msg) << std::endl; 
    start = std::chrono::system_clock::now();
    for(int i=0; i<10000*100; i++) {
        serverMsg = json::pb::toJsonMsg(msg);
    }
    end = std::chrono::system_clock::now();
    used = (end - start);
    std::cout << "normal pb used " << used.count() << "ms" << std::endl;

    json::pb::parse(serverMsg); 

    start = std::chrono::system_clock::now();
    for(int i=0; i<10000*100; i++) {
        json::pb::parse(serverMsg);
    }
    end = std::chrono::system_clock::now();
    used = (end - start);
    std::cout << "normal pb parse used " << used.count() << "ms" << std::endl;



    std::cout << "hello world" << std::endl;
}
