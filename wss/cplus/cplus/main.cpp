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

extern int share_ptr_test_main(int argc, const char * argv[]);
extern int string_memory_test_main(int argc, const char * argv[]);
extern int map_find_test_main(int argc, const char * argv[]);

int main(int argc, const char * argv[]) {
    int run_program = 3;
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
        default:
            break;
    }
    std::cout << "run program" << run_program << " success " << std::endl;
    return 0;
}
