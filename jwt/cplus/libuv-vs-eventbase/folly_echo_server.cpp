#include <folly/io/async/AsyncServerSocket.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/EventBase.h>
#include <folly/io/async/EventBaseManager.h>
#include <folly/io/IOBuf.h>
#include <glog/logging.h>
#include <iostream>
#include <atomic>

using namespace folly;

class EchoHandler : public AsyncSocket::ReadCallback {
public:
    EchoHandler(AsyncSocket* socket, EventBase* eventBase)
        : socket_(socket), eventBase_(eventBase) {}
    
    void getReadBuffer(void** bufReturn, size_t* lenReturn) override {
        *bufReturn = buffer_;
        *lenReturn = sizeof(buffer_);
    }
    
    void readDataAvailable(size_t len) noexcept override {
        message_count += len;
        
        auto data = IOBuf::copyBuffer(buffer_, len);
        socket_->writeChain(
            nullptr,
            std::move(data),
            false
        );
        
        socket_->read(this);
    }
    
    void readEOF() noexcept override {
        connection_count--;
        delete this;
    }
    
    void readErr(const AsyncSocketException& ex) noexcept override {
        LOG(ERROR) << "Read error: " << ex.what();
        connection_count--;
        delete this;
    }
    
    static std::atomic<size_t> connection_count;
    static std::atomic<size_t> message_count;
    
private:
    AsyncSocket* socket_;
    EventBase* eventBase_;
    char buffer_[4096];
};

std::atomic<size_t> EchoHandler::connection_count(0);
std::atomic<size_t> EchoHandler::message_count(0);

class EchoAcceptor : public AsyncServerSocket::AcceptCallback {
public:
    EchoAcceptor(EventBase* eventBase) : eventBase_(eventBase) {}
    
    void connectionAccepted(
        int fd,
        const SocketAddress& clientAddr) noexcept override {
        LOG(INFO) << "Accepted connection from " << clientAddr.describe();
        
        auto socket = AsyncSocket::newSocket(eventBase_, fd);
        EchoHandler::connection_count++;
        
        auto handler = new EchoHandler(socket.get(), eventBase_);
        socket->read(handler);
        
        sockets_.push_back(std::move(socket));
    }
    
    void acceptError(const std::exception& ex) noexcept override {
        LOG(ERROR) << "Accept error: " << ex.what();
    }
    
private:
    EventBase* eventBase_;
    std::vector<std::unique_ptr<AsyncSocket>> sockets_;
};

void print_stats() {
    static size_t last_messages = 0;
    static size_t last_connections = 0;
    
    size_t msg_diff = EchoHandler::message_count - last_messages;
    size_t conn_diff = EchoHandler::connection_count - last_connections;
    
    printf("Connections: %zu (new: %zu), Messages: %zu bytes (new: %zu bytes)\n",
           EchoHandler::connection_count.load(), conn_diff, 
           EchoHandler::message_count.load(), msg_diff);
    
    last_messages = EchoHandler::message_count;
    last_connections = EchoHandler::connection_count;
}

int main(int argc, char* argv[]) {
    google::InitGoogleLogging(argv[0]);
    
    EventBase eventBase;
    
    AsyncServerSocket serverSocket;
    serverSocket.bind(7000);
    serverSocket.listen(128);
    
    EchoAcceptor acceptor(&eventBase);
    serverSocket.setAcceptCallback(&acceptor, &eventBase);
    
    // 设置定时器每秒打印统计信息
    eventBase.runAfterDelay([]() {
        print_stats();
        // 继续运行
        EventBaseManager::get()->getEventBase()->runAfterDelay(
            []() {
                print_stats();
                EventBaseManager::get()->getEventBase()->runAfterDelay(
                    []() {
                        print_stats();
                        // 可以继续递归...
                    },
                    1000
                );
            },
            1000
        );
    }, 1000);
    
    LOG(INFO) << "Server started on port 7000";
    
    eventBase.loopForever();
    
    return 0;
}
    