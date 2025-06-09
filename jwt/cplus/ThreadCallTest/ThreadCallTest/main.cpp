#include <iostream>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

class Client {
public:
    Client(boost::asio::io_context& io_context)
        : resolver_(io_context),
          socket_(io_context),
          timer_(io_context) {}

    void connect(const std::string& host, const std::string& port) {
        // 创建解析查询
        tcp::resolver::query query(host, port);
        
        // 异步解析域名
        resolver_.async_resolve(query,
            [this](const boost::system::error_code& ec, tcp::resolver::results_type endpoints) {
                if (!ec) {
                    std::cout << "域名解析成功" << std::endl;
                    start_connect(endpoints);
                } else {
                    std::cerr << "解析失败: " << ec.message() << std::endl;
                }
            });
    }

private:
    void start_connect(tcp::resolver::results_type endpoints) {
        // 设置连接超时（例如 5 秒）
        timer_.expires_from_now(boost::posix_time::seconds(5));
        timer_.async_wait([this](const boost::system::error_code& ec) {
            if (!ec) {
                std::cerr << "连接超时" << std::endl;
                socket_.close();
            }
        });

        // 异步连接（自动尝试所有解析结果）
        boost::asio::async_connect(
            socket_,
            endpoints,
            [this](const boost::system::error_code& ec, tcp::endpoint endpoint) {
                timer_.cancel(); // 连接完成，取消定时器
                
                if (!ec) {
                    std::cout << "连接成功: " << endpoint << std::endl;
                    // 这里可以开始读写数据
                } else {
                    std::cerr << "连接失败: " << ec.message() << std::endl;
                }
            });
    }

    tcp::resolver resolver_;
    tcp::socket socket_;
    boost::asio::deadline_timer timer_;
};

int main() {
    try {
        boost::asio::io_context io_context;
        Client client(io_context);
        
        // 连接到 example.com 的 80 端口
        client.connect("example.com", "80");
        
        // 启动事件循环
        io_context.run();
    }
    catch (std::exception& e) {
        std::cerr << "异常: " << e.what() << std::endl;
    }
    
    return 0;
}
