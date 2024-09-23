package org.hwss.test.bench.mutiadmin;

import org.hwss.test.bench.improve.ClientFastTextFrame;
import org.java_websocket.client.WebSocketClient;
import org.java_websocket.handshake.ServerHandshake;
import org.java_websocket.util.Charsetfunctions;

import java.net.URI;
import java.nio.ByteBuffer;
import java.util.UUID;

/***
 * hwss
 * start new turn total message 2000002
 * receive used 8986.37ms
 *
 * start new turn total message 2000002 admin size 2
 * receive used 9792.13ms
 *
 * start new turn total message 2000002
 * receive used 8841.89ms
 *
 * admin
 * received message size : 1961148
 * received message used : 8200
 *
 * received message size : 2000002
 * received message used : 8839
 *
 * 参考只接受，不发送：
 * start new turn total message 2000002
 *  * receive used 6010.39ms
 *  每秒30万左右。
 *
 *  sendFrame替换后:
 *
 *  start new turn total message 2000002 admin size 2
 * receive used 9348.31ms
 *
 * start new turn total message 2000002 admin size 3
 * receive used 9687.38ms
 *
 * start new turn total message 2000002 admin size 3
 * receive used 8568.78ms
 *
 * start new turn total message 2000002 admin size 3
 * receive used 8468.64ms
 *
 * */
public class MessageBenchClient extends WebSocketClient {

    public MessageBenchClient(URI serverUri) {
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
        // Create all-trusting host name verifier
        WebSocketClient client = new MessageBenchClient(new URI("wss://localhost:9001/wss?app_id=434&group=client"));
        client.connectBlocking();
        client.getConnection();
        long start = System.currentTimeMillis();

        client.send("start_" + UUID.randomUUID().toString());
        for(int i=0; i < 10000*200; i++) {
            // 解码效率太差 send string
            //client.send(UUID.randomUUID().toString());
            ClientFastTextFrame frame  = new ClientFastTextFrame();
            frame.setPayload(ByteBuffer.wrap(Charsetfunctions.utf8Bytes(UUID.randomUUID().toString())));
            frame.setTransferemasked(true);
            client.sendFrame(frame);
        }
        client.send("end_" + UUID.randomUUID().toString());

        System.out.println("send message end " + (System.currentTimeMillis() - start));
    }
}
