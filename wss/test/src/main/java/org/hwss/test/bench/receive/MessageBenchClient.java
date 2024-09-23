package org.hwss.test.bench.receive;

import org.java_websocket.client.WebSocketClient;
import org.java_websocket.handshake.ServerHandshake;

import java.net.URI;
import java.nio.ByteBuffer;
import java.util.UUID;

/**
 * on client
 * start new turn total message 2000002
 * receive used 6010.39ms
 *
 * 每秒30万左右。
 *
 * start new turn total message 2000002
 * receive used 6263.76ms
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
        client.send("start_" + UUID.randomUUID().toString());
        for(int i=0; i < 10000*200; i++) {
            client.send(UUID.randomUUID().toString());
        }
        client.send("end_" + UUID.randomUUID().toString());

        System.out.println("send message end");
    }
}
