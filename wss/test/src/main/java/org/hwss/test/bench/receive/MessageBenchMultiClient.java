package org.hwss.test.bench.receive;

import org.java_websocket.client.WebSocketClient;
import org.java_websocket.handshake.ServerHandshake;

import java.net.URI;
import java.nio.ByteBuffer;
import java.util.UUID;

/**
 * 接收能力测试：
 * 5个client
 * hwss：
 * start new turn total message 8000002
 * receive used 11129.7ms
 *
 * start new turn total message 8000002
 * receive used 11398.9ms
 * 每秒 80万左右
 *
 * start new turn total message 8000002
 * receive used 12144.1ms
 *
 *
 * send message end 5422
 * */
public class MessageBenchMultiClient extends WebSocketClient {

    public MessageBenchMultiClient(URI serverUri) {
        super(serverUri);
    }

    @Override
    public void onOpen(ServerHandshake handshakedata) {
        System.out.println("new connection opened ");
    }

    @Override
    public void onClose(int code, String reason, boolean remote) {
        System.out.println("closed with exit code " + code + " additional info: " + reason);
    }

    @Override
    public void onMessage(String message) {
        System.out.println("received message: " + message);
    }

    @Override
    public void onMessage(ByteBuffer message) {
        System.out.println("received ByteBuffer");
    }

    @Override
    public void onError(Exception ex) {
        System.err.println("an error occurred:" + ex);
    }

    public static void main(String[] args) throws Exception {

        WebSocketClient mainClient = new MessageBenchMultiClient(new URI("wss://localhost:9001/wss?app_id=434&group=client"));
        mainClient.connectBlocking();
        long start = System.currentTimeMillis();
        mainClient.send("start_");
        Runnable runnable = new Runnable() {
            @Override
            public void run() {
                // Create all-trusting host name verifier
                 try {
                     WebSocketClient client = new MessageBenchMultiClient(new URI("wss://localhost:9001/wss?app_id=434&group=client"));
                     client.connectBlocking();
                     for(int i=0; i < 10000*200; i++) {
                         client.send(UUID.randomUUID().toString());
                     }
                } catch (Exception e) {
                    throw new RuntimeException(e);
                }
            }
        };
        Thread thread1 = new Thread(runnable);
        Thread thread2 = new Thread(runnable);
        Thread thread3 = new Thread(runnable);
        thread1.start();;
        thread2.start();;
        thread3.start();;

        for(int i=0; i < 10000*200; i++) {
            mainClient.send(UUID.randomUUID().toString());
        }
        thread1.join();
        thread2.join();
        thread3.join();
        mainClient.send("end_" + UUID.randomUUID().toString());

        System.out.println("send message end " + (System.currentTimeMillis() - start));
    }
}
