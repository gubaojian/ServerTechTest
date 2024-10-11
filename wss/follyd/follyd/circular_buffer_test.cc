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
#include <thread>
#include <memory>
#include <boost/circular_buffer.hpp>
#include <boost/timer/timer.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/thread.hpp>
#include <boost/call_traits.hpp>
#include <boost/bind.hpp>



template <class T>
class bounded_buffer
{
public:

  typedef boost::circular_buffer<T> container_type;
  typedef typename container_type::size_type size_type;
  typedef typename container_type::value_type value_type;
  typedef typename boost::call_traits<value_type>::param_type param_type;

  explicit bounded_buffer(size_type capacity) : m_unread(0), m_container(capacity) {}

  void push_front(typename boost::call_traits<value_type>::param_type item)
  { // `param_type` represents the "best" way to pass a parameter of type `value_type` to a method.

      boost::mutex::scoped_lock lock(m_mutex);
      m_not_full.wait(lock, boost::bind(&bounded_buffer<value_type>::is_not_full, this));
      m_container.push_front(item);
      ++m_unread;
      lock.unlock();
      m_not_empty.notify_one();
  }

  void pop_back(value_type* pItem) {
      boost::mutex::scoped_lock lock(m_mutex);
      m_not_empty.wait(lock, boost::bind(&bounded_buffer<value_type>::is_not_empty, this));
      *pItem = m_container[--m_unread];
      lock.unlock();
      m_not_full.notify_one();
  }

private:
  bounded_buffer(const bounded_buffer&);              // Disabled copy constructor.
  bounded_buffer& operator = (const bounded_buffer&); // Disabled assign operator.

  bool is_not_empty() const { return m_unread > 0; }
  bool is_not_full() const { return m_unread < m_container.capacity(); }

  size_type m_unread;
  container_type m_container;
  boost::mutex m_mutex;
  boost::condition m_not_empty;
  boost::condition m_not_full;
};


/**
 
 https://github.com/boostorg/circular_buffer/blob/develop/example/bounded_buffer_comparison.cpp
 
 client send event used 789ms
 client send event used 764ms
 
 */

int circular_buffer_test_main(int argc, const char * argv[]) {
    auto start = std::chrono::high_resolution_clock::now();
    auto end = std::chrono::high_resolution_clock::now();
    auto used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    
    std::shared_ptr<bounded_buffer<int>> buffer = std::make_shared<bounded_buffer<int>>(1024);
    
    std::thread consumer([buffer] {
        std::cout << "comsumer start " << std::endl;
        while (true) {
            int item;
            buffer->pop_back(&item);
            //std::cout << "comsumer " << item << std::endl;
        }
    });
    
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(1000ms);
    
    auto produceFunc = [buffer] {
        std::cout << "producer start " << std::endl;
        auto start = std::chrono::high_resolution_clock::now();
       
        for(int i=0; i<10000*300; i++) {
            buffer->push_front(i);
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "client send event used " << used.count() << "ms" << std::endl;
        
    };
    
    std::thread producter(produceFunc);
    producter.join();
    
    consumer.join();
    return 0;
}
