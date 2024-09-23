package org.hwss.test.bench.mutiadmin;

import org.java_websocket.client.WebSocketClient;
import org.java_websocket.handshake.ServerHandshake;

import java.net.URI;
import java.nio.ByteBuffer;
import java.util.HashSet;

/**
 *
 * */
public class MessageBenchiAdmin1 extends WebSocketClient {

    private static HashSet<String> messages = new HashSet<>();
    private static long startTime = 0;

    public MessageBenchiAdmin1(URI serverUri) {
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

        //System.out.println(message + Thread.currentThread());
        /**
       synchronized (messages) {
           messages.add(message);
           if (messages.contains("start_")) {
               startTime = System.currentTimeMillis();
               System.out.println("start new turn received message size : " + messages.size());
           }
           if (messages.contains("end_")) {
               System.out.println("received message size : " + messages.size());
               System.out.println("received message used : " + (System.currentTimeMillis() - startTime));
           }
       }*/
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
        WebSocketClient client = new MessageBenchiAdmin1(new URI("wss://127.0.0.1:9001/wss/?app_id=434&group=admin"));
        client.connect();
    }
}
