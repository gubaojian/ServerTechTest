//
//  tbb_test.cc
//  follyd
//
//  Created by efurture on 2024/10/8.
//
#include <iostream>
#include <string>
#include <iostream>
#include <chrono>
#include <map>
#include <set>
#include <cstdlib>
#include <unistd.h>

//#include <oneapi/tbb.h>


std::atomic<int> tbbTaskNum;



/**
 
 size_t length = 10000*300;
 tbb send fun done 1529ms
 */

int tbb_test_main(int argc, const char * argv[]) {
    /**
    auto start = std::chrono::high_resolution_clock::now();
    auto end = std::chrono::high_resolution_clock::now();
    auto used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
 
    oneapi::tbb::task_scheduler_handle scheduler;
      // 设置线程个数为4，例如
     

     oneapi::tbb::task_group group;
    
    start = std::chrono::high_resolution_clock::now();
    // 100万 send fun done task message done 348ms
    // 200万 send fun done task message done 348*2ms
    size_t length = 10000*300;
    for(int i=0; i<length; i++) {
        group.run([] {
            tbbTaskNum++;
        });
    }
    end = std::chrono::high_resolution_clock::now();
    used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "tbb send fun done " << used.count() << "ms "  << std::endl;
  
    group.wait();
   
    int sum = oneapi::tbb::parallel_reduce(
            oneapi::tbb::blocked_range<int>(1,101), 0,
            [](oneapi::tbb::blocked_range<int> const& r, int init) -> int {
                for (int v = r.begin(); v != r.end(); v++) {
                    init += v;
                }
                return init;
            },
            [](int lhs, int rhs) -> int {
                return lhs + rhs;
            }
        );

   printf("Sum: %d\n", sum);
     */
    return 0;
}
