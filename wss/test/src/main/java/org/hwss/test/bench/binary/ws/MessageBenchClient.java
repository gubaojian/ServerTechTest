package org.hwss.test.bench.binary.ws;

import org.apache.commons.lang3.RandomUtils;
import org.java_websocket.client.WebSocketClient;
import org.java_websocket.client.WebSocketClient2;
import org.java_websocket.handshake.ServerHandshake;

import java.net.URI;
import java.nio.ByteBuffer;
import java.util.UUID;

/***
 * hwss
 * start new turn total message 2000002
 * receive used 8163.93ms
 *
 * start new turn total message 2000002
 * receive used 6286.7ms
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
 * */
public class MessageBenchClient extends WebSocketClient2 {

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
        MessageBenchClient client = new MessageBenchClient(new URI("ws://127.0.0.1:9001/wss?app_id=434&group=client"));
        client.connectBlocking();
        client.getConnection();
        long start = System.currentTimeMillis();

        client.send("start_" + UUID.randomUUID().toString());
        byte[] bts = RandomUtils.nextBytes(32);
        ByteBuffer bytes = ByteBuffer.wrap(bts);
        for(int i=0; i < 10000*200; i++) {
            client.send(bytes);
        }
        client.send("end_" + UUID.randomUUID().toString());

        System.out.println("ws send message end " + (System.currentTimeMillis() - start));
    }
}
