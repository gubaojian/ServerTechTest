/* We simply call the root header file "App.h", giving you uWS::App and uWS::SSLApp */
#include "App.h"
#include <memory>
#include <string>
#include <vector>
#include <utility>
#include <string_view>

/* This is a simple WebSocket "sync" upgrade example.
 * You may compile it with "WITH_OPENSSL=1 make" or with "make" */

static std::shared_ptr<std::vector<std::string>> adminBuffer = std::make_shared<std::vector<std::string>>();

auto start = std::chrono::system_clock::now();

bool pressure = false;
bool admin = true;
int messageNum = 0;
			
int main() {
    /* ws->getUserData returns one of these */
    struct PerSocketData {
        /* Define your user data */
        bool admin;
    };

    

    uWS::WebSocket<true, true, PerSocketData>*  adminWs;
    

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
        .maxBackpressure = 1 * 1024 * 1024, //越大约不容易丢失消息： 16加大这个可以增加缓存，避免队列，但不能提示
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
                .admin = admin
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
        .open = [&adminWs](auto *ws) {
            /* Open event here, you may access ws->getUserData() which points to a PerSocketData struct.
             * Here we simply validate that indeed, something == 13 as set in upgrade handler. */
            PerSocketData* data = ws->getUserData();
            if (data->admin) {
                adminWs = ws;
            }
            std::cout << "Something is: " << static_cast<PerSocketData *>(ws->getUserData())->admin << std::endl;
        },
        .message = [&adminWs](auto *ws, std::string_view message, uWS::OpCode opCode) {
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
            /* We simply echo whatever data we get 
            if (adminWs != ws) {
                if (pressure) {
                    adminBuffer->emplace_back(message);
                } else {
                    auto status = adminWs->send(message, opCode);
                    if (status ==  uWS::WebSocket<true, true,PerSocketData>::SendStatus::BACKPRESSURE) {
                        std::cout << "backpressure" << std::endl;
                        pressure = true;
                    } else if (status ==  uWS::WebSocket<true, true,PerSocketData>::SendStatus::DROPPED) {
                        std::cout << "message dropped " << std::endl;
                        adminBuffer->emplace_back(message);
                        pressure = true;
                    }
                }
            }*/
        },
        .drain = [&adminWs](auto *ws) {
             /* Check ws->getBufferedAmount() here */
            while(adminBuffer->size() > 0 ) {
                auto& message = adminBuffer->back();
                auto status = adminWs->send(message, uWS::OpCode::BINARY);
                if (status ==  uWS::WebSocket<true, true,PerSocketData>::SendStatus::BACKPRESSURE) {
                      //std::cout << "backpressure" << std::endl;
                      //adminBuffer->pop_back();
                      //break;
                }
                if (status ==  uWS::WebSocket<true, true,PerSocketData>::SendStatus::DROPPED) {
                    std::cout << "message drain dropped " << std::endl;
                    break;
                }
                adminBuffer->pop_back();
            }
            pressure = (adminBuffer->size() != 0);
            std::cout << "drain size " << adminBuffer->size() << std::endl;
          
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
            std::cout << "Listening on port " << 9001  << "endl" << std::endl;
        }
    }).run();
}
