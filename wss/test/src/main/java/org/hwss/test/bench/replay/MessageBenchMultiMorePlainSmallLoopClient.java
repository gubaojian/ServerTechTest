package org.hwss.test.bench.replay;

import org.java_websocket.client.WebSocketClient;
import org.java_websocket.handshake.ServerHandshake;

import java.net.URI;
import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.List;
import java.util.UUID;

/***
 * 11个客户端：
 * start new turn total message 21999945
 * receive used 36391.7ms
 * 2200 万 36秒  等于每秒61万
 *
 * start new turn total message 21973927
 * receive used 36126.4ms
 *
 *  start new turn total message 21954524
 * receive used 11804.4ms
 * server单线程。
 * 11个client线程：180万每秒
 *
 * */
public class MessageBenchMultiMorePlainSmallLoopClient extends WebSocketClient {

    private List<String> messages = new ArrayList<>();
    private  long startTime = 0;


    public MessageBenchMultiMorePlainSmallLoopClient(URI serverUri) {
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
        if (message.startsWith("start_")) {
            messages.clear();
            startTime = System.currentTimeMillis();
            System.out.println("start new turn received message size : " + messages.size());
        }
        messages.add(message);
        if (message.startsWith("end_")) {
            System.out.println("received message size : " + messages.size());
            System.out.println("received message used : " + (System.currentTimeMillis() - startTime));
        }
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

        WebSocketClient mainClient = new MessageBenchMultiMorePlainSmallLoopClient(new URI("ws://localhost:9002/wss?app_id=434&group=client"));
        mainClient.connectBlocking();
        long start = System.currentTimeMillis();
        mainClient.send("start_");
        Runnable runnable = new Runnable() {
            @Override
            public void run() {
                // Create all-trusting host name verifier
                 try {
                     WebSocketClient client = new MessageBenchMultiMorePlainSmallLoopClient(new URI("ws://localhost:9002/wss?app_id=434&group=client"));
                     client.connectBlocking();
                     for(int i=0; i < 10000*2000; i++) {
                         client.send("a");
                     }
                } catch (Exception e) {
                    throw new RuntimeException(e);
                }
            }
        };
        List<Thread> threads = new ArrayList<>();
        int size = 10;
        for(int i=0; i<10; i++) {
            Thread thread = new Thread(runnable);
            thread.start();;
            threads.add(thread);
        }

        for(int i=0; i < 10000*2000; i++) {
            mainClient.send("a");
        }
        for(Thread thread : threads) {
            thread.join();
        }
        mainClient.send("end_" + UUID.randomUUID().toString());

        System.out.println("send message end " + (System.currentTimeMillis() - start));
    }
}
