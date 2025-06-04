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
 *  https://github.com/KDE/kcachegrind
 *    https://user-images.githubusercontent.com/19547/88445602-879c0300-cdd8-11ea-92cc-ba22b1c5d121.png
 *
 *    https://github.com/mruby/mruby/issues/5047
 *
 *  https://github.com/mruby/mruby/pull/5069/files
 *    https://valgrind.org/docs/manual/manual.html
 *
 * https://www.ctyun.cn/developer/article/444952717525061
 *  https://github.com/openjdk/jdk21/blob/jdk-21%2B35/src/hotspot/os/linux/os_linux.cpp#L5518
 *  https://github.com/alibaba/arthas/issues/3020
 * std::list std::deque 调用shrink_to_fit试试。
 *  https://www.cnblogs.com/shizioo/p/rnacos_readme.html
 * 尝试替代queue保持各平台内存一致性。或者创建新对象替换老对象的方式释放。
 * std::queue 在linux平台上不释放内存，具体看deque实现源代码
 *  https://github.com/gcc-mirror/gcc/blob/master/libstdc%2B%2B-v3/include/bits/stl_deque.h
 *  https://github.com/gcc-mirror/gcc/blob/master/libstdc%2B%2B-v3/include/bits/stl_list.h
 *  pop不会释放内存，
 *      //! <b>Effects</b>: Removes the first element from the deque.
 *     //!
 *     //! <b>Throws</b>: Nothing.
 *     //!
 *     //! <b>Complexity</b>: Constant time.
 *     void pop_front() BOOST_NOEXCEPT_OR_NOTHROW
 *     {
 *        BOOST_ASSERT(!this->empty());
 *        if (this->members_.m_start.m_cur != this->members_.m_start.m_last - 1) {
 *           allocator_traits_type::destroy
 *              ( this->alloc()
 *              , boost::movelib::to_raw_pointer(this->members_.m_start.m_cur)
 *              );
 *           ++this->members_.m_start.m_cur;
 *        }
 *        else
 *           this->priv_pop_front_aux();
 *     }
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
