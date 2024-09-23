package org.hwss.test.bench.replay;

import com.alibaba.fastjson2.JSON;
import org.apache.commons.lang3.RandomUtils;
import org.java_websocket.client.WebSocketClient;
import org.java_websocket.handshake.ServerHandshake;

import java.net.URI;
import java.nio.ByteBuffer;
import java.util.*;

/***
 * 11个客户端：
 * start new turn total message 21999945
 * receive used 36391.7ms
 * 2200 万 36秒  等于每秒61万
 *
 * start new turn total message 22000002
 * receive used 41355.9ms
 *  53万每秒。
 *
 *  closed with exit code 1006 additional info: The connection was closed because the other endpoint did not respond with a pong in time. For more information check: https://github.com/TooTallNate/Java-WebSocket/wiki/Lost-connection-detection
 *
 *  Exception in thread "Thread-9" java.lang.RuntimeException: org.java_websocket.exceptions.WebsocketNotConnectedException
 * 	at org.hwss.test.bench.replay.MessageBenchMultiMoreClient2$1.run(MessageBenchMultiMoreClient2.java:98)
 * 	at java.base/java.lang.Thread.run(Thread.java:842)
 * Caused by: org.java_websocket.exceptions.WebsocketNotConnectedException
 * 	at org.java_websocket.WebSocketImpl.send(WebSocketImpl.java:674)
 * 	at org.java_websocket.WebSocketImpl.send(WebSocketImpl.java:650)
 * 	at org.java_websocket.client.WebSocketClient.send(WebSocketClient.java:433)
 * 	at org.hwss.test.bench.replay.MessageBenchMultiMoreClient2$1.run(MessageBenchMultiMoreClient2.java:94)
 * 	... 1 more
 *
 *   线程太多，速度反而很慢 。猜测可能受gc和内存影响。
 *   线程改为4个，吞吐率反而更高。
 *   线程多了，速度反而慢，估计切换概率高。
 *
 * */
public class MessageBenchMultiMoreClient2 extends WebSocketClient {

    private List<String> messages = new ArrayList<>();
    private  long startTime = 0;



    public MessageBenchMultiMoreClient2(URI serverUri) {
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
        //messages.add(message);
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

        byte[] bts = RandomUtils.nextBytes(1024);

        Map<String, Object> json = new HashMap<>();
        json.put("action", "bmsg");
        json.put("msg",  Base64.getEncoder().encodeToString(bts));
        json.put("authId", UUID.randomUUID().toString());
        json.put("authToken", UUID.randomUUID().toString());
        json.put("appId", RandomUtils.nextInt(10, 2000));
        json.put("hwssId", UUID.randomUUID().toString());

        String msg =  JSON.toJSONString(json);

        System.out.println(msg);

        WebSocketClient mainClient = new MessageBenchMultiMoreClient2(new URI("wss://localhost:9001/wss?app_id=434&group=client"));
        mainClient.setConnectionLostTimeout(120);
        mainClient.connectBlocking();
        long start = System.currentTimeMillis();
        mainClient.send("start_");
        Runnable runnable = new Runnable() {
            @Override
            public void run() {
                // Create all-trusting host name verifier
                 try {
                     WebSocketClient client = new MessageBenchMultiMoreClient2(new URI("wss://localhost:9001/wss?app_id=434&group=client"));
                     client.connectBlocking();
                     client.setConnectionLostTimeout(120);
                     client.send("start_");
                     for(int i=0; i < 10000*200; i++) {
                         client.send(msg);
                     }
                     client.send("end_" + UUID.randomUUID().toString());
                } catch (Exception e) {
                    throw new RuntimeException(e);
                }
            }
        };
        List<Thread> threads = new ArrayList<>();
        int size = 4;
        for(int i=0; i<size; i++) {
            Thread thread = new Thread(runnable);
            thread.start();;
            threads.add(thread);
        }

        for(int i=0; i < 10000*200; i++) {
            mainClient.send(msg);
        }
        for(Thread thread : threads) {
            thread.join();
        }
        mainClient.send("end_" + UUID.randomUUID().toString());

        System.out.println("send message end " + (System.currentTimeMillis() - start));
    }
}
