use websocket::sync::Server as SyncServer;

fn main() {
    // 使用同步服务器（SyncServer）
    let mut server = SyncServer::bind("127.0.0.1:8080").unwrap();
    println!("服务器启动，监听 ws://127.0.0.1:8080");
    
    let mut message_count = 0;
    
    // 同步接受连接（不再使用 incoming()）
    loop {
        match server.accept() {
            Ok(request) => {
                let mut client = request.accept().unwrap();
                println!("新客户端连接");
                
                // 处理消息
                for message in client.incoming_messages() {
                    if message.is_ok() {
                        message_count += 1;
                        if (message_count % 10000 == 0){
                            println!("总消息数: {}", message_count);
                        }
                    }
                }
            }
            Err(e) => {
                println!("接受连接错误:");
            }
        }
    }
}
