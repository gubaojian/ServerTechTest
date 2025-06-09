#include <iostream>
#include <string>
#include <memory>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = net::ip::tcp;               // from <boost/asio/ip/tcp.hpp>

// 异步WebSocket客户端类
class WebSocketClient : public std::enable_shared_from_this<WebSocketClient> {
public:
    explicit WebSocketClient(net::io_context& ioc)
        : resolver_(net::io_context::strand(ioc)),
          ws_(net::io_context::strand(ioc)) {}

    // 启动连接
    void run(const std::string& host, const std::string& port, const std::string& target) {
        // 保存参数
        host_ = host;
        port_ = port;
        target_ = target;

        // 解析主机名
        resolver_.async_resolve(
            host,
            port,
            beast::bind_front_handler(
                &WebSocketClient::on_resolve,
                shared_from_this()));
    }

private:
    // 解析完成回调
    void on_resolve(beast::error_code ec, tcp::resolver::results_type results) {
        if (ec) {
            std::cerr << "Resolve failed: " << ec.message() << std::endl;
            return;
        }

        // 连接到服务器
        beast::get_lowest_layer(ws_).expires_after(std::chrono::seconds(30));
        net::async_connect(
            beast::get_lowest_layer(ws_).next_layer(),
            results.begin(),
            results.end(),
            beast::bind_front_handler(
                &WebSocketClient::on_connect,
                shared_from_this()));
    }

    // 连接完成回调
    void on_connect(beast::error_code ec, tcp::endpoint) {
        if (ec) {
            std::cerr << "Connect failed: " << ec.message() << std::endl;
            return;
        }

        // 执行WebSocket握手
        beast::get_lowest_layer(ws_).expires_never();
        ws_.set_option(websocket::stream_base::timeout::suggested(beast::role_type::client));
        ws_.set_option(websocket::stream_base::decorator(
            [](websocket::request_type& req) {
                req.set(http::field::user_agent,
                    std::string(BOOST_BEAST_VERSION_STRING) +
                    " websocket-client-async");
            }));

        ws_.async_handshake(host_, target_,
            beast::bind_front_handler(
                &WebSocketClient::on_handshake,
                shared_from_this()));
    }

    // 握手完成回调
    void on_handshake(beast::error_code ec) {
        if (ec) {
            std::cerr << "Handshake failed: " << ec.message() << std::endl;
            return;
        }

        // 发送消息
        ws_.async_write(
            net::buffer(std::string("Hello, WebSocket Server!")),
            beast::bind_front_handler(
                &WebSocketClient::on_write,
                shared_from_this()));
    }

    // 写操作完成回调
    void on_write(beast::error_code ec, std::size_t bytes_transferred) {
        boost::ignore_unused(bytes_transferred);

        if (ec) {
            std::cerr << "Write failed: " << ec.message() << std::endl;
            return;
        }

        // 读取响应
        ws_.async_read(
            buffer_,
            beast::bind_front_handler(
                &WebSocketClient::on_read,
                shared_from_this()));
    }

    // 读操作完成回调
    void on_read(beast::error_code ec, std::size_t bytes_transferred) {
        boost::ignore_unused(bytes_transferred);

        if (ec) {
            std::cerr << "Read failed: " << ec.message() << std::endl;
            return;
        }

        // 输出响应内容
        std::cout << "Received: " << beast::buffers_to_string(buffer_.data()) << std::endl;

        // 关闭WebSocket连接
        ws_.async_close(websocket::close_code::normal,
            beast::bind_front_handler(
                &WebSocketClient::on_close,
                shared_from_this()));
    }

    // 关闭完成回调
    void on_close(beast::error_code ec) {
        if (ec) {
            std::cerr << "Close failed: " << ec.message() << std::endl;
            return;
        }

        std::cout << "WebSocket connection closed normally" << std::endl;
    }

    tcp::resolver resolver_;
    websocket::stream<tcp::socket> ws_;
    beast::flat_buffer buffer_;
    std::string host_;
    std::string port_;
    std::string target_;
};

int main() {
    try {
        // 检查命令行参数
        auto const host = "127.0.0.1";
        auto const port = "64";
        auto const target = "/ws";

        // 创建I/O上下文
        net::io_context ioc;

        // 创建并启动客户端
        auto client = std::make_shared<WebSocketClient>(ioc);
        client->run(host, port, target);

        // 运行I/O上下文
        ioc.run();
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}    