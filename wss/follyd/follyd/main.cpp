//
//  main.cpp
//  follyd
//
//  Created by efurture on 2024/9/29.
//

#include <iostream>


extern int folly_executor_test_main(int argc, const char * argv[]);
extern int json_test_main(int argc, const char * argv[]);
extern int event_bus_test_main(int argc, const char * argv[]);
extern int event_bus_json_test_main(int argc, const char * argv[]);
extern int event_bus_json_router_test_main(int argc, const char * argv[]);
extern int event_bus_json_test2_main(int argc, const char * argv[]);

int main(int argc, const char * argv[]) {
    int run_program = 6;
    switch (run_program) {
        case 1:
            folly_executor_test_main(argc, argv);
            break;
        case 2:
            json_test_main(argc, argv);
            break;
        case 3:
            event_bus_test_main(argc, argv);
            break;
        case 4:
            event_bus_json_test_main(argc, argv);
            break;
        case 5:
            event_bus_json_router_test_main(argc, argv);
            break;
        case 6:
            event_bus_json_test2_main(argc, argv);
            break;
        default:
            break;
    }
    std::cout << "run program " << run_program << " success " << std::endl;
    return 0;
}
