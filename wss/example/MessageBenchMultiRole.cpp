/* We simply call the root header file "App.h", giving you uWS::App and uWS::SSLApp */
#include "App.h"
#include <memory>
#include <string>
#include <vector>
#include <utility>
#include <string_view>
#include <cstdlib> 
#include <mutex>
#include <thread>
#include <chrono>

/* ws->getUserData returns one of these */
struct PerSocketData {
    /* Define your user data */
    bool admin = false;
};


/* This is a simple WebSocket "sync" upgrade example.
 * You may compile it with "WITH_OPENSSL=1 make" or with "make" */

static std::shared_ptr<std::vector<std::string>> adminBuffer = std::make_shared<std::vector<std::string>>();
std::mutex  adminBufferMutex;

auto start = std::chrono::system_clock::now();

bool pressure = false;
int  adminNum = 0;
int messageNum = 0;
bool messageEnd =  false;
unsigned int roundRobin = 0;

std::vector<uWS::WebSocket<true, true, PerSocketData>*> adminSockets;

uWS::Loop* adminLoop = nullptr;
uWS::Loop* clientLoop  = nullptr;

int handleMessageCount = 0;

 std::shared_ptr<std::vector<std::string>> handleBuffer = std::make_shared<std::vector<std::string>>();

void handleClientMessages() {
     if (handleBuffer->size() == 0) {
        {
            std::lock_guard<std::mutex> lock(adminBufferMutex);
            if (adminBuffer->size() > 0) {
               handleBuffer = adminBuffer;
               adminBuffer =std::make_shared<std::vector<std::string>>();
            }
        }
        if (handleBuffer->size() == 0) {
            return;
        }
     }
     if (adminSockets.size() == 0) {
        std::cout << "none admin connect " << std::endl;
        return;
     }
     size_t index = roundRobin%adminSockets.size();
     roundRobin++;
     auto adminWs = adminSockets.at(index);
     while(handleBuffer->size() > 0 ) {
        auto& message = handleBuffer->back();
        auto status = adminWs->send(message, uWS::OpCode::TEXT);
        if (status ==  uWS::WebSocket<true, true,PerSocketData>::SendStatus::BACKPRESSURE) {
           // std::cout << "admin backpressure drain" << std::endl;
            handleBuffer->pop_back();
            handleMessageCount++;
            break;
        }
        if (status ==  uWS::WebSocket<true, true,PerSocketData>::SendStatus::DROPPED) {
            std::cout << "admin message drain dropped " << std::endl;
            break;
        }
        handleBuffer->pop_back();
        handleMessageCount++;
    }  
    if (handleBuffer->size() == 0) {
        bool handleNext = false;
        {
            std::lock_guard<std::mutex> lock(adminBufferMutex);
            if (adminBuffer->size() > 0) {
               handleBuffer = adminBuffer;
               adminBuffer = std::make_shared<std::vector<std::string>>();
            }
        }
        if (handleBuffer->size() > 0){
            adminLoop->defer([]{
                handleClientMessages();
            });
        } else {
             std::cout << "handle message done " << handleMessageCount << std::endl;
              auto end = std::chrono::system_clock::now();
			  std::chrono::duration<double, std::milli>  used = (end - start);
               std::cout << "handle message total used " << used.count() << "ms" << std::endl;
                handleMessageCount = 0;
				
        }
     }
}
			

int main() {

    std::thread adminServer([] {
            /* Keep in mind that uWS::SSLApp({options}) is the same as uWS::App() when compiled without SSL support.
            * You may swap to using uWS:App() if you don't need SSL */
            uWS::SSLApp({
                /* There are example certificates in uWebSockets.js repo */
                .key_file_name = "/Users/efurture/hwss/hwss/tool/example.com-key.pem",
                .cert_file_name = "/Users/efurture/hwss/hwss/tool/example.com.pem",
                .passphrase = ""
            }).ws<PerSocketData>("/*", {
                /* Settings */
                .compression = uWS::DISABLED,
                .maxPayloadLength = 16 * 1024,
                //.idleTimeout = 10,
                .maxBackpressure = 32 * 1024 * 1024, //越大约不容易丢失消息： 16加大这个可以增加缓存，避免队列，但不能提示
                /* Handlers */
                .upgrade = [](auto *res, auto *req, auto *context) {
                    adminNum+=1;
                    std::cout << "upgrade : " << adminNum << std::endl;
                    /* You may read from req only here, and COPY whatever you need into your PerSocketData.
                    * PerSocketData is valid from .open to .close event, accessed with ws->getUserData().
                    * HttpRequest (req) is ONLY valid in this very callback, so any data you will need later
                    * has to be COPIED into PerSocketData here. */

                    /* Immediately upgrading without doing anything "async" before, is simple */
                    res->template upgrade<PerSocketData>({
                        /* We initialize PerSocketData struct here */
                        .admin = true
                    }, req->getHeader("sec-websocket-key"),
                        req->getHeader("sec-websocket-protocol"),
                        req->getHeader("sec-websocket-extensions"),
                        context);

                    /* If you don't want to upgrade you can instead respond with custom HTTP here,
                    * such as res->writeStatus(...)->writeHeader(...)->end(...); or similar.*/

                    /* Performing async upgrade, such as checking with a database is a little more complex;
                    * see UpgradeAsync example instead. */
                },
                .open = [](auto *ws) {
                    /* Open event here, you may access ws->getUserData() which points to a PerSocketData struct.
                    * Here we simply validate that indeed, something == 13 as set in upgrade handler. */
                    PerSocketData* data = ws->getUserData();
                    adminSockets.emplace_back(ws);
                    std::cout << "Admin connect is: " << static_cast<PerSocketData *>(ws->getUserData())->admin << std::endl;
                },
                .message = [](auto *ws, std::string_view message, uWS::OpCode opCode) {
                    if (message.starts_with("start_") == true) {
                        messageNum = 0;
                        messageEnd = false;
                        start = std::chrono::system_clock::now();
                        std::cout << "admin start new turn " << " admin size " << adminSockets.size() << std::endl;
                    }
                    messageNum++;
                    if (messageEnd) {
                        std::cout << "admin receive message after end " << messageNum << std::endl;
                    }
                    if (message.starts_with("end_") == true) {
                        messageEnd = true;
                        auto end = std::chrono::system_clock::now();
                        std::chrono::duration<double, std::milli>  used = (end - start);
                        std::cout << "admin start new turn total message " << messageNum << " admin size " <<adminSockets.size() << std::endl;
                        std::cout << "admin receive used " << used.count() << "ms" << std::endl;
                    }
                    /** 
                    PerSocketData* data = ws->getUserData();
                    if (data->admin) {
                    return;
                    }
                    /* We simply echo whatever data we get 
                    int server = roundRobin%admins.size();
                    auto adminWs = admins.at(server);
                    auto status = adminWs->send(message, opCode);
                    if (status ==  uWS::WebSocket<true, true,PerSocketData>::SendStatus::BACKPRESSURE) {
                        //std::cout << "backpressure" << std::endl;
                        //pressure = true;
                    } else if (status ==  uWS::WebSocket<true, true,PerSocketData>::SendStatus::DROPPED) {
                        //std::cout << "message dropped " << std::endl;
                        //    adminBuffer->emplace_back(message);
                        //    pressure = true;
                    }
                    roundRobin+=1;*/
                },
                .drain = [](auto *ws) {
                    handleClientMessages();
                     /* 
                    auto adminWs = ws;
                   Check ws->getBufferedAmount() here 
                    while(adminBuffer->size() > 0 ) {
                        auto& message = adminBuffer->back();
                        auto status = adminWs->send(message, uWS::OpCode::TEXT);
                        if (status ==  uWS::WebSocket<true, true,PerSocketData>::SendStatus::BACKPRESSURE) {
                            std::cout << "backpressure drain dropped " << std::endl;
                            adminBuffer->pop_back();
                            break;
                        }
                        if (status ==  uWS::WebSocket<true, true,PerSocketData>::SendStatus::DROPPED) {
                            std::cout << "message drain dropped " << std::endl;
                            break;
                        }
                        adminBuffer->pop_back();
                    }
                    //adminBuffer->shrink_to_fit();
                    pressure = (adminBuffer->size() != 0);*/
                },
                .ping = [](auto */*ws*/, std::string_view) {
                    /* You don't need to handle this one, we automatically respond to pings as per standard */
                },
                .pong = [](auto */*ws*/, std::string_view) {
                    /* You don't need to handle this one either */
                },
                .close = [](auto */*ws*/, int /*code*/, std::string_view /*message*/) {
                    /* You may access ws->getUserData() here, but sending or
                    * doing any kind of I/O with the socket is not valid. */
                }
            }).listen(9002, [](auto *listen_socket) {
                if (listen_socket) {
                    std::cout << "Admin Server Listening on port " << 9002  << std::endl;
                }
                adminLoop  = uWS::Loop::get();
            }).run();
    });

   std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    /* Keep in mind that uWS::SSLApp({options}) is the same as uWS::App() when compiled without SSL support.
     * You may swap to using uWS:App() if you don't need SSL */
    uWS::SSLApp({
        /* There are example certificates in uWebSockets.js repo */
	       .key_file_name = "/Users/efurture/hwss/hwss/tool/example.com-key.pem",
	    .cert_file_name = "/Users/efurture/hwss/hwss/tool/example.com.pem",
	    .passphrase = ""
	}).ws<PerSocketData>("/*", {
        /* Settings */
        .compression = uWS::DISABLED,
        .maxPayloadLength = 16 * 1024,
        //.idleTimeout = 10,
        .maxBackpressure = 32 * 1024 * 1024, //越大约不容易丢失消息： 16加大这个可以增加缓存，避免队列，但不能提示
        /* Handlers */
        .upgrade = [](auto *res, auto *req, auto *context) {
             adminNum+=1;
             std::cout << "upgrade : " << adminNum << std::endl;
            /* You may read from req only here, and COPY whatever you need into your PerSocketData.
             * PerSocketData is valid from .open to .close event, accessed with ws->getUserData().
             * HttpRequest (req) is ONLY valid in this very callback, so any data you will need later
             * has to be COPIED into PerSocketData here. */

            /* Immediately upgrading without doing anything "async" before, is simple */
            res->template upgrade<PerSocketData>({
                /* We initialize PerSocketData struct here */
                .admin = false
            }, req->getHeader("sec-websocket-key"),
                req->getHeader("sec-websocket-protocol"),
                req->getHeader("sec-websocket-extensions"),
                context);

            /* If you don't want to upgrade you can instead respond with custom HTTP here,
             * such as res->writeStatus(...)->writeHeader(...)->end(...); or similar.*/

            /* Performing async upgrade, such as checking with a database is a little more complex;
             * see UpgradeAsync example instead. */
        },
        .open = [](auto *ws) {
            /* Open event here, you may access ws->getUserData() which points to a PerSocketData struct.
             * Here we simply validate that indeed, something == 13 as set in upgrade handler. */
            PerSocketData* data = ws->getUserData();
            if (data->admin) {
              // admins.emplace_back(ws);
            }
            std::cout << "Something is: " << static_cast<PerSocketData *>(ws->getUserData())->admin << std::endl;
        },
        .message = [](auto *ws, std::string_view message, uWS::OpCode opCode) {
            if (message.starts_with("start_") == true) {
                messageNum = 0;
                 messageEnd = false;
                start = std::chrono::system_clock::now();
                std::cout << "client start new turn " << std::endl;
            }
             messageNum++;
            if (messageEnd) {
                std::cout << "receive message after end " << messageNum << std::endl;
            }
            if (message.starts_with("end_") == true) {
                 messageEnd = true;
                 auto end = std::chrono::system_clock::now();
				 std::chrono::duration<double, std::milli>  used = (end - start);
                 std::cout << "client start new turn total message " << messageNum  << std::endl;
				 std::cout << "receive used " << used.count() << "ms" << std::endl;
            }
            bool  notify = false;
            {
                std::lock_guard<std::mutex> lock(adminBufferMutex);
                notify  = (adminBuffer->size() == 0);
                adminBuffer->emplace_back(message);
            }
            if (notify) {
                adminLoop->defer([] {
                    handleClientMessages();
                });
            }
        },
        .drain = [](auto *ws) {
            std::cout << "backpressure drain " << std::endl;
            /** 
             auto adminWs = ws;
            /* Check ws->getBufferedAmount() here
            while(adminBuffer->size() > 0 ) {
                auto& message = adminBuffer->back();
                auto status = adminWs->send(message, uWS::OpCode::TEXT);
                if (status ==  uWS::WebSocket<true, true,PerSocketData>::SendStatus::BACKPRESSURE) {
                      std::cout << "backpressure drain dropped " << std::endl;
                       adminBuffer->pop_back();
                      break;
                }
                if (status ==  uWS::WebSocket<true, true,PerSocketData>::SendStatus::DROPPED) {
                    std::cout << "message drain dropped " << std::endl;
                    break;
                }
                adminBuffer->pop_back();
            }
            //adminBuffer->shrink_to_fit();
            pressure = (adminBuffer->size() != 0);*/
        },
        .ping = [](auto */*ws*/, std::string_view) {
            /* You don't need to handle this one, we automatically respond to pings as per standard */
        },
        .pong = [](auto */*ws*/, std::string_view) {
            /* You don't need to handle this one either */
        },
        .close = [](auto */*ws*/, int /*code*/, std::string_view /*message*/) {
            /* You may access ws->getUserData() here, but sending or
             * doing any kind of I/O with the socket is not valid. */
        }
    }).listen(9001, [](auto *listen_socket) {
        if (listen_socket) {
            std::cout << "client Listening on port " << 9001  << "endl" << std::endl;
        }
        clientLoop = uWS::Loop::get();
    }).run();
    adminServer.join();
}
