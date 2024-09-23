package org.hwss.test.bench.multirole;

import org.java_websocket.client.WebSocketClient;
import org.java_websocket.handshake.ServerHandshake;

import java.net.URI;
import java.nio.ByteBuffer;
import java.util.UUID;

/**
 * send message end 12972
 * hwss:
 * start new turn total message 8000002 admin size 2
 * receive used 44027.1ms
 * admin1 & admin2 client
 *
 * start new turn total message 3625586 admin size 2
 * receive used 18721.3ms
 *
 * hwss： admin1
 * start new turn total message 5091181
 * receive used 25424.7ms
 * start new turn total message 8000002
 * receive used 36436.1ms
 *
 * hwss multi role :
 * client start new turn total message 8000002
 * receive used 12214.5ms
 *
 * 参考只接收不发送：
 *  start new turn total message 8000002
 *  receive used 11398.9ms
 *  每秒 80万左右
 *
 * 说明消息卡到admin接收端：
 * admin
 * received message size : 8000002
 * received message used : 37211
 *
 * received message size : 8000002
 * received message used : 36446
 *
 * admin multirole :
 * received message size : 2421198
 * received message used : 13269
 *
 * received message size : 3110848
 * received message used : 14464
 *
 * client start new turn total message 8000002
 * receive used 13052.3ms
 * handle message done 14000008
 * handle message total used 27888.4ms
 *
 * client start new turn total message 8000002
 * receive used 11869.2ms
 * handle message done 30106321
 * handle message total used 28282.5ms
 *
 * client start new turn total message 8000002
 * receive used 12622.4ms
 * handle message done 7886306
 * handle message total used 28379.3ms
 *
 * 速度发送速度卡在网卡上或者接收端的速度上。20-25万每秒。
 * receive used 12469ms
 * receive message after end 8000004
 * client start new turn total message 8000004
 * receive used 12469ms
 * handle message done 7967383
 * handle message total used 33012.5ms
 *
 * 每秒21万左右：
 * 时间可能大部分消耗在发送端。
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
        mainClient.send("end_" + UUID.randomUUID().toString());
        mainClient.send("end_" + UUID.randomUUID().toString());


        System.out.println("send message end " + (System.currentTimeMillis() - start));
    }
}
