package org.hwss.test;

import org.java_websocket.client.WebSocketClient;
import org.java_websocket.drafts.Draft;
import org.java_websocket.handshake.ServerHandshake;

import java.net.URI;
import java.nio.ByteBuffer;

/**
 * $JAVA_HOME
 * /Library/Java/JavaVirtualMachines/jdk-17.jdk/Contents/Home
 * */
public class EmptyClient2 extends WebSocketClient {

    public EmptyClient2(URI serverUri, Draft draft) {
        super(serverUri, draft);
    }

    public EmptyClient2(URI serverURI) {
        super(serverURI);
    }

    @Override
    public void onOpen(ServerHandshake handshakedata) {
        long start = System.currentTimeMillis();
        for(int i=0; i<10; i++) {
            send("Hello, it is me. Mario :)Hello, it is me. Mario :)");
        }
        System.out.println("new connection opened used " + (System.currentTimeMillis() - start) + " ms");
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
        WebSocketClient client = new EmptyClient2(new URI("wss://127.0.0.1:9001/dd?app_id=434&group=clien"));
        client.connect();
    }
}