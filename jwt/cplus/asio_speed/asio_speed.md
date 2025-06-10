g++ -std=c++11 -O3 -I/opt/homebrew/Cellar/boost/1.88.0/include/ -L/opt/homebrew/Cellar/boost/1.88.0/lib asio_loopback_test.cpp -lboost_system -lboost_thread -lpthread -o asio_loopback_test


g++ -std=c++11 -O3 -I/opt/homebrew/Cellar/boost/1.88.0/include/ -L/opt/homebrew/Cellar/boost/1.88.0/lib asio_loopback_test2.cpp -lboost_system -lboost_thread -lpthread -o asio_loopback_test2


g++ -std=c++11 -O3 asio_loopback_test3.cpp -lboost_system -lboost_thread -lpthread -o asio_loopback_test3



g++ -std=c++11 -O3 asio_loopback_test4.cpp -lboost_system -lboost_thread -lpthread -o asio_loopback_test4


g++ -std=c++11 -O3 asio_loopback_test5.cpp -lboost_system -lboost_thread -lpthread -o asio_loopback_test5


g++ -std=c++11 -O3 beast_test.cpp -lboost_system -lboost_thread -lpthread -o beast_test


g++ -std=c++11 -O3 websocket_async_client.cpp -lboost_system -lboost_thread -lpthread -o beast_async



g++ -std=c++11 -O3 -I/opt/homebrew/Cellar/boost/1.88.0/include/ -L/opt/homebrew/Cellar/boost/1.88.0/lib post_speed.cpp -lboost_system -lboost_thread -lpthread -o post_speed


g++ -std=c++14 -O3 -I/opt/homebrew/Cellar/boost/1.88.0/include/ -L/opt/homebrew/Cellar/boost/1.88.0/lib post_speed2.cpp -lboost_system -lboost_thread -lpthread -o post_speed2
