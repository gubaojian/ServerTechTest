package org.hwss.test.bench.multirole;

import org.java_websocket.client.WebSocketClient;
import org.java_websocket.handshake.ServerHandshake;

import java.net.URI;
import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.List;

/**
 * 每秒钟可以接收20-25万条。 速度受限于单个socket的接收速度。
 * */
public class MessageBenchAdmin2 extends WebSocketClient {

    private List<String> messages = new ArrayList<>();
    private  long startTime = 0;

    public MessageBenchAdmin2(URI serverUri) {
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
        // Create all-trusting host name verifier
        WebSocketClient client = new MessageBenchAdmin2(new URI("wss://127.0.0.1:9002/wss/?app_id=434&group=admin"));
        client.connect();
    }
}
