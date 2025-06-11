//
//  websocket_pp_frame_test.cc
//  websocketpp
//
//  Created by baojian on 2025/6/11.
//
#include <string>
#include <iostream>
#include <uuid/uuid.h>
#include <chrono>
#include <map>
#include <set>
#include <cstdlib>
#include <queue>
#include <unistd.h>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "websocketpp/config/asio_no_tls_client.hpp"
#include "websocketpp/client.hpp"
#include "websocketpp/processors/fast_data_frame.hpp"

typedef websocketpp::client<websocketpp::config::asio_client> client;

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

// pull out the type of messages sent by our config
typedef websocketpp::config::asio_client::message_type::ptr message_ptr;

static client* mainClient;
static client::connection_ptr mainConn;

int websocket_pp_frame_test(int argc, const char * argv[]){
    auto start = std::chrono::high_resolution_clock::now();
    auto end = std::chrono::high_resolution_clock::now();
    auto used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
   
    client client;
    std::string uri = "ws://localhost:9001";

    try {
        // Set logging to be pretty verbose (everything except message payloads)
        client.set_access_channels(websocketpp::log::alevel::none);
        //c.clear_access_channels(websocketpp::log::alevel::frame_payload);

        
        // Initialize ASIO
        client.init_asio();

        // Register our message handler
        //client.set_message_handler(bind(&on_message,&client,::_1,::_2));

        websocketpp::lib::error_code ec;
        client::connection_ptr con =  client.get_connection(uri, ec);
        if (ec) {
            std::cout << "could not create connection because: " << ec.message() << std::endl;
            return 0;
        }
        
        con->set_fail_handler([](websocketpp::connection_hdl hdl) {
            std::cout << "client failed connect " << std::endl;
            mainClient = nullptr;
            mainConn = nullptr;
            
        });
        con->set_close_handler([](websocketpp::connection_hdl hdl){
            std::cout << "client be closed " << std::endl;
            mainClient = nullptr;
            mainConn = nullptr;
        });

        // Note that connect here only requests a connection. No network messages are
        // exchanged until the event loop starts running in the next line.
        mainConn = client.connect(con);
        mainClient = &client;
        
        auto processor = mainConn->get_processor(13);
        message_ptr message = mainConn->get_message(websocketpp::frame::opcode::value::binary, 0);
        std::string msg = R"({"wsgId":"wsg_88448848322","connId":"7ddf2d51-4593-4742-a8bc-32c179ebc06f","action":"textMsg","msg":"gwElnMclNKmhsLHPwoIsPVoYNtxTYZKuDUZJrVzkEYhBDXLAiGTXKEEEEUTEAkkdRcNAuVHOCUqjHeTJkUxnwKLpIsFjjitIDFpRKFNolTiUlkFAplSuEBKmPMUNdFHvabxUNXswBwLNAhZxYWPKEYjrRvjMqxBffjqIwQlxeqMBMOIvAmUSNQFvZiLpbcDqTVKyRXtmZfNkrntOVcybFGBrFFTDPacZRVOJQdfBwCYKMuQUZnLVbLAvlBhvPbLOEZODBngODkCofvSaepaQBtOEnvmJMMUFRAZnXTIXHAPtzdsfDIbSJDbhmePSLjvJMBrhKDrpRjkxlVNTishsdIQnDXwIswmSmJyEQtMhmsZUiJVbCXyqOsSoUWDrwrfdybgkAXIfWwlMpMxSWDPWAMLVuZEEzMGVWxHSfGUEvSxbJRFplTuSydcFRTLqZzBeIgRawKHCFdqvZqMoKJrIDjgIttXqyYZLoTvSlftNEgqJntbYaQaZpbEwlYgCyngFIWSnkAqDtWjiqlhdXiydIpLrQqNOIldNGlzKnjEomVGtTpgQSLSiEqSOSKZDPawvblVtlAdiipjByhwflwSjqonoWsoBJNWvoVejqfCUteHbZCynPjvBANhUoPseBKGuBxNvEkBnJqlwQAsdXMCjmrtIaadmhhnyqkGjEEzGMuGXKMQqckUdNYfcTzReLtIjtCJxmHXZeODNnBZskLNPbzHhuPpUWhsKXQgoJFuDUirWYqtactaYdIoaSwClciHunSqKtwWVoutGGXBwwqlfFNobFnInDwUVGjymFlnqPBUOsmWpUgQiEBqgsBLjfvehLSlmHsxpQgWooqruetQyFirUjLlwwsagwffPmPeDxQOLLJivsFKxMFFgguyhYzoRMVRVwtnRDQWTgcZXyMPhonPUwsCnMVyMpylueAJUtEGPkvskMvsLBcpARsCKAPQZSpBcvzpAhrIGFZgJKWWNObKcNuBQjemVcIlBZhsPMTyXbHEoeEKLxEOrMnbDlWAL"}
)";
        start = std::chrono::high_resolution_clock::now();
        std::queue<message_ptr> send_frames;
        for(int i=0; i<1000*10*400; i++) {
            auto it = std::make_shared<std::string>(msg);
            message_ptr out = mainConn->get_message();
            message->set_payload(std::move(*it));
            processor->prepare_data_frame(message, out);
            send_frames.push(out);
        }
        end = std::chrono::high_resolution_clock::now();
        used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "used " << used << std::endl;
        send_frames = std::queue<message_ptr>();
        start = std::chrono::high_resolution_clock::now();
        for(int i=0; i<1000*10*400; i++) {
            auto it = std::make_shared<std::string>(msg);
            message_ptr out;
            message->set_payload(std::move(*it));
            processor->prepare_data_frame_fast(message, out);
            send_frames.push(out);
        }
        end = std::chrono::high_resolution_clock::now();
        used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "fast used " << used << std::endl;
        
        send_frames = std::queue<message_ptr>();
        start = std::chrono::high_resolution_clock::now();
        for(int i=0; i<1000*10*400; i++) {
            auto it = std::make_shared<std::string>(msg);
            message->set_payload(std::move(*it));
            
            message_ptr out;
            websocketpp::processor::prepare_data_frame_fast(message, out);
            send_frames.push(out);
        }
        end = std::chrono::high_resolution_clock::now();
        used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "fast used " << used << std::endl;
        
        /**
         used 1551ms
         fast used 1313ms
         */
    
        // Start the ASIO io_service run loop
        // this will cause a single connection to be made to the server. c.run()
        // will exit when this connection is closed.
        
        //client.run();
    } catch (websocketpp::exception const & e) {
        std::cout << "websocket exception " << e.what() << std::endl;
    }
    
    return 0;
}
