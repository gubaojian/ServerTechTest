//
//  main.cpp
//  cplus
//
//  Created by efurture on 2024/9/26.
//

#include <iostream>
#include <map>
#include <string>
#include <memory>
#include <unistd.h>

extern int share_ptr_test_main(int argc, const char * argv[]);
extern int string_memory_test_main(int argc, const char * argv[]);
extern int map_find_test_main(int argc, const char * argv[]);
extern int map_find_test_int_main(int argc, const char * argv[]);
extern int uuid_compare_test_main(int argc, const char * argv[]);
extern int base64_compare_test_main(int argc, const char * argv[]);
extern int base64_usage_test_main(int argc, const char * argv[]);
extern int uuid_int128_map_test_main(int argc, const char * argv[]);
extern int json_test_main(int argc, const char * argv[]);

int main(int argc, const char * argv[]) {
    int run_program = 4;
    switch (run_program) {
        case 1:
            share_ptr_test_main(argc, argv);
            break;
        case 2:
            string_memory_test_main(argc, argv);
            break;
        case 3:
            map_find_test_main(argc, argv);
            break;
        case 4:
            uuid_compare_test_main(argc, argv);
            break;
        case 5:
            base64_compare_test_main(argc, argv);
            break;
        case 6:
            base64_usage_test_main(argc, argv);
            break;
        case 7:
            map_find_test_int_main(argc, argv);
            break;
        case 8:
            map_find_test_main(argc, argv);
            usleep(1000*20);
            break;
        case 9:
            uuid_int128_map_test_main(argc, argv);
            break;
        case 10:
            json_test_main(argc, argv);
            break;
        default:
            break;
    }
    std::cout << "run program " << run_program << " success " << std::endl;
    return 0;
}
