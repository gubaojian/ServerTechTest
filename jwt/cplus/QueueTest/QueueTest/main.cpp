//
//  main.cpp
//  QueueTest
//
//  Created by efurture on 2025/6/2.
//

#include <iostream>
#include <list>
#include <deque>
#include <queue>
#include <chrono>
#include <functional>
#include <string>

struct TestCase
{
    std::string title;
    std::function<void()> fun;
};

int main()
{
    std::function<void()> fun = []
    {
        std::list<int> l = {7, 5, 16, 8};
        for (int i = 0; i < 1000000; i++)
        {
            l.push_front(13);
            l.push_back(25);
        }
    };
    std::function<void()> fun2 = []
    {
        std::deque<int> d = {7, 5, 16, 8};
        for (int i = 0; i < 1000000; i++)
        {
            d.push_front(13);
            d.push_back(25);
        }
    };

    std::function<void()> fun3 = []
    {
        std::queue<int, std::list<int>> q;
        for (int i = 0; i < 1000000; i++)
        {
            q.push(i);
        }
        for (int i = 0; i < 5000; i++)
        {
            q.pop();
        }
    };
    std::function<void()> fun4 = []
    {
        std::queue<int> q;
        for (int i = 0; i < 1000000; i++)
        {
            q.push(i);
        }
        for (int i = 0; i < 5000; i++)
        {
            q.pop();
        }
    };

    TestCase tests[] = {
        {"list", fun},
        {"deque", fun2},
        {"queue list", fun3},
        {"queue deque", fun4}};

    for (auto t : tests)
    {

        auto begin = std::chrono::high_resolution_clock::now();
        t.fun();
        auto end = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin);
        std::cout << t.title << " Time measured:" << elapsed.count() * 1e-9 << " seconds.\n";
    }
    std::shared_ptr<std::string> from = std::make_shared<std::string>("test");
    
    if ("test" == *from) {
        std::cout << "hello match" << std::endl;
    }
    
    
    return 0;
}
