package org.hwss.test.bench.mutiadmin;

import org.hwss.test.bench.improve.ClientFastTextFrame;
import org.java_websocket.client.WebSocketClient;
import org.java_websocket.handshake.ServerHandshake;
import org.java_websocket.util.Charsetfunctions;

import java.net.URI;
import java.nio.ByteBuffer;
import java.util.UUID;

/**
 * send message end 12972
 * hwss:
 * start new turn total message 8000002 admin size 2
 * receive used 44027.1ms
 * admin1 & admin2 client
 * start new turn total message 3625586 admin size 2
 * receive used 18721.3ms
 *
 * hwss： admin1
 * start new turn total message 5091181
 * receive used 25424.7ms
 * start new turn total message 8000002
 * receive used 36436.1ms
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
 * 每秒21万左右：
 * 时间可能大部分消耗在发送端。
 *
 * start new turn total message 6086197 admin size 3
 * receive used 27995.9ms
 *
 * 22万和单线程发送效率差不多。
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
                         //client.send(UUID.randomUUID().toString());

                         ClientFastTextFrame frame  = new ClientFastTextFrame();
                         frame.setPayload(ByteBuffer.wrap(Charsetfunctions.utf8Bytes(UUID.randomUUID().toString())));
                         frame.setTransferemasked(true);
                         client.sendFrame(frame);
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
            ClientFastTextFrame frame  = new ClientFastTextFrame();
            frame.setPayload(ByteBuffer.wrap(Charsetfunctions.utf8Bytes(UUID.randomUUID().toString())));
            frame.setTransferemasked(true);
            mainClient.sendFrame(frame);
        }
        thread1.join();
        thread2.join();
        thread3.join();
        mainClient.send("end_" + UUID.randomUUID().toString());

        System.out.println("send message end " + (System.currentTimeMillis() - start));
    }
}
