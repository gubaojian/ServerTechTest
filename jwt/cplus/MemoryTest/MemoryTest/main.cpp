//
//  main.cpp
//  MemoryTest
//
//  Created by efurture on 2025/6/3.
//

#include <iostream>
#include <unistd.h>
#include <memory>
#include <string>
#include <queue>
#include <list>

/**
 * std::list std::deque 调用shrink_to_fit试试。
 * 尝试替代queue保持各平台内存一致性。或者创建新对象替换老对象的方式释放。
 * std::queue 在linux平台上不释放内存，具体看deque实现源代码
 *  https://github.com/gcc-mirror/gcc/blob/master/libstdc%2B%2B-v3/include/bits/stl_deque.h
 *  https://github.com/gcc-mirror/gcc/blob/master/libstdc%2B%2B-v3/include/bits/stl_list.h
 *  pop不会释放内存，
 * Android和Mac平台pop时会释放不必要的内存，平台上libc实现不一致。
 *   template <class _Tp, class _Allocator>
 *   void deque<_Tp, _Allocator>::pop_front() {
 *     size_type __old_sz    = size();
 *     size_type __old_start = __start_;
 *     allocator_type& __a   = __alloc();
 *     __alloc_traits::destroy(
 *         __a, std::__to_address(*(__map_.begin() + __start_ / __block_size) + __start_ % __block_size));
 *     --__size();
 *     ++__start_;
 *     __annotate_shrink_front(__old_sz, __old_start);
 *     __maybe_remove_front_spare();
 *   }
 *
 *  微软实现也一样：
 *     https://github.com/microsoft/STL/blob/main/stl/inc/queue
 *     https://github.com/microsoft/STL/blob/main/stl/inc/deque
 *
 */
int main(int argc, const char * argv[]) {
    // insert code here...
    std::queue<int64_t> queue;
    for(int i=0; i<1024*1024*4; i++) {
        queue.push(i);
    }
    sleep(50);
    std::cout << "push done" << std::endl;
    while (!queue.empty()) {
        queue.pop();
    }
    
    std::cout << "pop empty" << std::endl;
    for(int i=0; i<1000; i++) {
        sleep(5);
    }
    
    return 0;
}
