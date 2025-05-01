/* We simply call the root header file "App.h", giving you uWS::App and uWS::SSLApp */
#include "App.h"
#include <memory>
#include <string>
#include <vector>
#include <utility>
#include <string_view>

/* 
4mb 
start new turn total message 6777319
receive used 45665.2ms
start new turn total message 7317541
receive used 48413.6ms
start new turn total message 7727644
receive used 50789.7ms
start new turn total message 8000005
receive used 53125.7ms

start new turn total message 5476094
receive used 41796.2ms
start new turn total message 7232348
receive used 52456.5ms
start new turn total message 7413535
receive used 53599.5ms
start new turn total message 8000005
receive used 58642.6ms

8mb:
start new turn total message 8000005
receive used 50853.8ms

start new turn total message 8000005
receive used 49522.3ms

start new turn total message 8000005
receive used 53050ms

start new turn total message 8000005
receive used 54576.9ms

开启压缩后时间边长。 cpu占用也多。 压缩自己扩展比较好。：
start new turn total message 6505251
receive used 57752.8ms
start new turn total message 7612912
receive used 63364.5ms
start new turn total message 7757442
receive used 64206.5ms
start new turn total message 8000005
receive used 66487.2ms


在STL中 vector和string 是比较特殊的，clear()之后是不会释放内存空间的，也就是size()会清零，但capacity()不会改变，需要手动去释放。释放方法除了swap一个空string外，好像c++11里新加入的的shrink_to_fit也可以。

作者：张贰柱
链接：https://www.zhihu.com/question/53069538/answer/133319002
来源：知乎
著作权归作者所有。商业转载请联系作者获得授权，非商业转载请注明出处。

This is a simple WebSocket "sync" upgrade example.
 * You may compile it with "WITH_OPENSSL=1 make" or with "make" */

auto start = std::chrono::system_clock::now();

bool pressure = false;
bool admin = true;
int messageNum = 0;
			
int main() {
    /* ws->getUserData returns one of these */
    struct PerSocketData {
        /* Define your user data */
        bool admin;
        bool pressure = false;
        std::shared_ptr<std::vector<std::string>> buffer = std::make_shared<std::vector<std::string>>();
    };

    


    /* Keep in mind that uWS::SSLApp({options}) is the same as uWS::App() when compiled without SSL support.
     * You may swap to using uWS:App() if you don't need SSL */
    uWS::SSLApp({
        /* There are example certificates in uWebSockets.js repo */
	       .key_file_name = "/Users/efurture/hwss/hwss/tool/example.com-key.pem",
	    .cert_file_name = "/Users/efurture/hwss/hwss/tool/example.com.pem",
	    .passphrase = ""
	}).ws<PerSocketData>("/*", {
        /* Settings */
        .compression =  uWS::CompressOptions(uWS::SHARED_COMPRESSOR  |  uWS::SHARED_DECOMPRESSOR) , //开启压缩后cpu飙升时间边长，建议自定义二进制消息压缩， 比如自定义zstd等效果更好。
        .maxPayloadLength = 32 * 1024,
        //.idleTimeout = 10,
        .maxBackpressure = 8*1024 * 1024, //越大约不容易丢失消息： 16加大这个可以增加缓存，避免队列，但不能提示
        /* Handlers */
        .upgrade = [](auto *res, auto *req, auto *context) {


             std::cout << "upgrade : " << std::endl;
            /* You may read from req only here, and COPY whatever you need into your PerSocketData.
             * PerSocketData is valid from .open to .close event, accessed with ws->getUserData().
             * HttpRequest (req) is ONLY valid in this very callback, so any data you will need later
             * has to be COPIED into PerSocketData here. */

            /* Immediately upgrading without doing anything "async" before, is simple */
            res->template upgrade<PerSocketData>({
                /* We initialize PerSocketData struct here */
                .admin = false,
                .pressure = false,
                .buffer = std::make_shared<std::vector<std::string>>()
            }, req->getHeader("sec-websocket-key"),
                req->getHeader("sec-websocket-protocol"),
                req->getHeader("sec-websocket-extensions"),
                context);
             admin = false;

            /* If you don't want to upgrade you can instead respond with custom HTTP here,
             * such as res->writeStatus(...)->writeHeader(...)->end(...); or similar.*/

            /* Performing async upgrade, such as checking with a database is a little more complex;
             * see UpgradeAsync example instead. */
        },
        .open = [](auto *ws) {
            /* Open event here, you may access ws->getUserData() which points to a PerSocketData struct.
             * Here we simply validate that indeed, something == 13 as set in upgrade handler. */
            PerSocketData* data = ws->getUserData();
            std::cout << "Something is: " << static_cast<PerSocketData *>(ws->getUserData())->admin << std::endl;
        },
        .message = [](auto *ws, std::string_view message, uWS::OpCode opCode) {
            if (message.starts_with("start_") == true) {
                messageNum = 0;
                start = std::chrono::system_clock::now();
                std::cout << "start new turn " << std::endl;
            }
             messageNum++;
            if (message.starts_with("end_") == true) {
                 auto end = std::chrono::system_clock::now();
				 std::chrono::duration<double, std::milli>  used = (end - start);
                 std::cout << "start new turn total message " << messageNum << std::endl;
				 std::cout << "receive used " << used.count() << "ms" << std::endl;
            }
             PerSocketData* data = ws->getUserData();
             if (data->pressure) {
                data->buffer->emplace_back(message);
                return;
             }   
            /* We simply echo whatever data we get */
            auto status = ws->send(message, opCode);
            if (status ==  uWS::WebSocket<true, true,PerSocketData>::SendStatus::BACKPRESSURE) {
               // std::cout << "backpressure" << std::endl;
                // 有pressure 还要继续放，这样能提升吞吐量1-2倍
                //data->pressure = true;
            } else if (status ==  uWS::WebSocket<true, true,PerSocketData>::SendStatus::DROPPED) {
                 //std::cout << "client message dropped " << std::endl;
                 data->buffer->emplace_back(message);
                 data->pressure = true;
            }
        },
        .dropped = [] (auto *ws, std::string_view message, uWS::OpCode opCode) {
               // std::cout << "message dropped " << std::endl;   
        },
        .drain = [](auto *ws) {
             /* Check ws->getBufferedAmount() here */
             PerSocketData* data = ws->getUserData(); 
             std::shared_ptr<std::vector<std::string>> buffer = data->buffer;
             //std::cout << "drain start size " << buffer->size() << std::endl;
            while(buffer->size() > 0 ) {
                auto& message = buffer->back(); 
                auto status = ws->send(message, uWS::OpCode::TEXT);
                if (status ==  uWS::WebSocket<true, true,PerSocketData>::SendStatus::BACKPRESSURE) {
                      //std::cout << "backpressure" << std::endl;
                     // buffer->pop_back();
                      //break;
                }
                if (status ==  uWS::WebSocket<true, true,PerSocketData>::SendStatus::DROPPED) {
                    //std::cout << "drain dropped " << std::endl;
                    break;
                }
                buffer->pop_back();
            }
            data->pressure = (buffer->size() != 0);
            //std::cout << "drain end size " << buffer->size() << std::endl;
        },
        /* 不要处理，交给框架处理，写了方法不处理会有异常
        .ping = [](auto *ws, std::string_view) {
            /* You don't need to handle this one, we automatically respond to pings as per standard 
        },
        .pong = [](auto *ws, std::string_view) {
            /* You don't need to handle this one either
        }, */
        .close = [](auto */*ws*/, int /*code*/, std::string_view /*message*/) {
            /* You may access ws->getUserData() here, but sending or
             * doing any kind of I/O with the socket is not valid. */
        }
    }).listen(9001, [](auto *listen_socket) {
        if (listen_socket) {
            std::cout << "Listening on port " << 9001  << "endl" << std::endl;
        }
    }).run();
}
