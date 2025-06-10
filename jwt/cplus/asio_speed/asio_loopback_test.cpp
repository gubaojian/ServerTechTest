#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

using boost::asio::ip::tcp;
using namespace std::chrono;

constexpr size_t BUFFER_SIZE = 1024 * 1024;  // 1MB 缓冲区
constexpr int PORT = 12345;
constexpr int TEST_DURATION_SECONDS = 30;
constexpr int THREAD_COUNT = 4;  // 线程数

// 服务器类 - 接收数据并统计吞吐量
class Server : public std::enable_shared_from_this<Server> {  // 新增继承
public:
    Server(boost::asio::io_context& io_context)
        : acceptor_(io_context, tcp::endpoint(tcp::v4(), PORT)),
          socket_(io_context),
          bytes_received_(0),
          start_time_(high_resolution_clock::now()) {
        start_accept();
    }

    uint64_t get_bytes_received() const {
        return bytes_received_;
    }

    duration<double> get_elapsed_time() const {
        return high_resolution_clock::now() - start_time_;
    }

private:
    void start_accept() {
        acceptor_.async_accept(socket_,
            [this](boost::system::error_code ec) {
                if (!ec) {
                    // 开始接收数据
                    start_read();
                }
                // 继续接受下一个连接
                start_accept();
            });
    }

    void start_read() {
        auto self(shared_from_this());  // 现在可以正常使用
        socket_.async_read_some(
            boost::asio::buffer(data_, BUFFER_SIZE),
            [this, self](boost::system::error_code ec, std::size_t length) {
                if (!ec) {
                    bytes_received_ += length;
                    start_read();
                }
            });
    }

    tcp::acceptor acceptor_;
    tcp::socket socket_;
    std::array<char, BUFFER_SIZE> data_;
    std::atomic<uint64_t> bytes_received_;
    time_point<high_resolution_clock> start_time_;
};

// 客户端类 - 发送数据
class Client : public std::enable_shared_from_this<Client> {  // 新增继承
public:
    Client(boost::asio::io_context& io_context)
        : socket_(io_context),
          data_(BUFFER_SIZE, 'A'),  // 填充测试数据
          bytes_sent_(0) {}

    void connect() {
        tcp::endpoint endpoint(boost::asio::ip::make_address("127.0.0.1"), PORT);
        socket_.connect(endpoint);
        start_write();
        start_write2();
        start_write3();
        start_write4();
        start_write5();
        start_write5();
    }

    uint64_t get_bytes_sent() const {
        return bytes_sent_;
    }

private:
    void start_write() {
        auto self(shared_from_this());  // 现在可以正常使用
        boost::asio::async_write(
            socket_,
            boost::asio::buffer(data_),
            [this, self](boost::system::error_code ec, std::size_t length) {
                if (!ec) {
                    bytes_sent_ += length;
                    start_write();
                }
            });
    }
    
    void start_write2() {
        auto self(shared_from_this());  // 现在可以正常使用
        boost::asio::async_write(
            socket_,
            boost::asio::buffer(data_),
            [this, self](boost::system::error_code ec, std::size_t length) {
                if (!ec) {
                    bytes_sent_ += length;
                    start_write2();
                }
            });
    }
    
    void start_write3() {
        auto self(shared_from_this());  // 现在可以正常使用
        boost::asio::async_write(
            socket_,
            boost::asio::buffer(data_),
            [this, self](boost::system::error_code ec, std::size_t length) {
                if (!ec) {
                    bytes_sent_ += length;
                    start_write3();
                }
            });
    }
    
    void start_write4() {
        auto self(shared_from_this());  // 现在可以正常使用
        boost::asio::async_write(
            socket_,
            boost::asio::buffer(data_),
            [this, self](boost::system::error_code ec, std::size_t length) {
                if (!ec) {
                    bytes_sent_ += length;
                    start_write4();
                }
            });
    }
    
    void start_write5() {
        auto self(shared_from_this());  // 现在可以正常使用
        boost::asio::async_write(
            socket_,
            boost::asio::buffer(data_),
            [this, self](boost::system::error_code ec, std::size_t length) {
                if (!ec) {
                    bytes_sent_ += length;
                    start_write5();
                }
            });
    }
    
    void start_write6() {
        auto self(shared_from_this());  // 现在可以正常使用
        boost::asio::async_write(
            socket_,
            boost::asio::buffer(data_),
            [this, self](boost::system::error_code ec, std::size_t length) {
                if (!ec) {
                    bytes_sent_ += length;
                    start_write6();
                }
            });
    }

    tcp::socket socket_;
    std::vector<char> data_;
    std::atomic<uint64_t> bytes_sent_;
};

/*
 * 总发送字节数: 74894540800 字节
 * 测试时长: 31.0076 秒
 * 吞吐量: 19322.9 Mbps
 * 吞吐量: 2415.36 MB/秒
 * 线程数: 1
 * 总发送字节数: 78576091136 字节
 * 测试时长: 31.0135 秒
 * 吞吐量: 20268.9 Mbps
 * 吞吐量: 2533.61 MB/秒
 * 线程数: 4
 * 缓冲区大小: 1024 KB
 
 测试开始，持续 30 秒...
 测试结束，测试时间 30 秒...
 测试结果:
 总发送字节数: 288450674688 字节
 测试时长: 31.007 秒
 吞吐量: 74422.1 Mbps
 吞吐量: 9302.76 MB/秒
 线程数: 4
 缓冲区大小: 1024 KB
 
 */
int main() {
    try {
        // 创建服务器
        boost::asio::io_context server_io_context;
        auto server = std::make_shared<Server>(server_io_context);
        
        // 启动服务器线程
        std::thread server_thread([&server_io_context]() {
            server_io_context.run();
        });

        // 等待服务器初始化
        std::this_thread::sleep_for(std::chrono::seconds(1));

        // 创建客户端 I/O 上下文和线程池
        std::vector<std::shared_ptr<boost::asio::io_context>> client_io_contexts;
        std::vector<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>> work_guards;
        std::vector<std::thread> client_threads;

        for (int i = 0; i < THREAD_COUNT; ++i) {
            auto io_context = std::make_shared<boost::asio::io_context>();
            
            // 使用 make_work_guard 替代 io_context::work
            work_guards.emplace_back(boost::asio::make_work_guard(*io_context));
            
            client_io_contexts.push_back(io_context);

            client_threads.emplace_back([io_context]() {
                io_context->run();
            });
        }

        // 创建并启动客户端
        std::vector<std::shared_ptr<Client>> clients;
        for (int i = 0; i < THREAD_COUNT; ++i) {
            auto client = std::make_shared<Client>(*client_io_contexts[i]);
            clients.push_back(client);
            client->connect();
        }

        // 运行测试
        std::cout << "测试开始，持续 " << TEST_DURATION_SECONDS << " 秒..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(TEST_DURATION_SECONDS));

        std::cout << "测试结束，测试时间 " << TEST_DURATION_SECONDS << " 秒..." << std::endl;
       
        // 停止客户端
        work_guards.clear();  // 释放工作保护，允许 io_context 退出
        for (int i = 0; i < THREAD_COUNT; ++i) {
            client_io_contexts[i]->stop();
        }
        for (auto& thread : client_threads) {
            thread.join();
        }
       
        // 停止服务器
        server_io_context.stop();
        if (server_thread.joinable()) {
            server_io_context.stop();
            server_thread.join();
        }

        // 计算总吞吐量
        uint64_t total_bytes_sent = 0;
        for (const auto& client : clients) {
            total_bytes_sent += client->get_bytes_sent();
        }

        auto elapsed = server->get_elapsed_time();
        double throughput_mbps = (total_bytes_sent * 8.0) / (elapsed.count() * 1000000.0);
        double throughput_mb_per_sec = total_bytes_sent / (elapsed.count() * 1000000.0);

        // 输出结果
        std::cout << "测试结果:" << std::endl;
        std::cout << "总发送字节数: " << total_bytes_sent << " 字节" << std::endl;
        std::cout << "测试时长: " << elapsed.count() << " 秒" << std::endl;
        std::cout << "吞吐量: " << throughput_mbps << " Mbps" << std::endl;
        std::cout << "吞吐量: " << throughput_mb_per_sec << " MB/秒" << std::endl;
        std::cout << "线程数: " << THREAD_COUNT << std::endl;
        std::cout << "缓冲区大小: " << BUFFER_SIZE / 1024 << " KB" << std::endl;

    } catch (std::exception& e) {
        std::cerr << "异常: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
