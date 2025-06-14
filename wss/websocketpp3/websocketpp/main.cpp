//
//  main.cpp
//  websocketpp
//
//  Created by efurture on 2024/9/29.
//

#include <iostream>
#include <map>
#include <string>
#include <memory>
#include <unistd.h>


extern int websocket_pp_no_tts_test_main(int argc, const char * argv[]);
extern int websocket_pp_tts_test_main(int argc, const char * argv[]);
extern int websocket_pp_tts_test_multi_client_main(int argc, const char * argv[]);

extern int websocket_pp_frame_test(int argc, const char * argv[]);

extern int batch_queue_test(int argc, const char * argv[]);

int main(int argc, const char * argv[]) {
    int run_program = 14;
    switch (run_program) {
        case 1:
            websocket_pp_tts_test_main(argc, argv);
            break;
        case 11:
            websocket_pp_no_tts_test_main(argc, argv);
            break;
        case 12:
            websocket_pp_tts_test_multi_client_main(argc, argv);
            break;
        case 13:
            websocket_pp_frame_test(argc, argv);
            break;
        case 14:
            batch_queue_test(argc, argv);
            break;
        default:
            break;
    }
    std::cout << "run program " << run_program << " success " << std::endl;
    return 0;
}
