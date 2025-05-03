package org.hwss.test.bench.memory;

import org.java_websocket.client.WebSocketClient;
import org.java_websocket.handshake.ServerHandshake;

import java.net.URI;
import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.List;

/**
 * 每秒钟可以接收20-25万条。 速度受限于单个socket的接收速度。
 * */
public class MessageBenchAdmin extends WebSocketClient {


    public MessageBenchAdmin(URI serverUri) {
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
        WebSocketClient client = new MessageBenchAdmin(new URI("ws://127.0.0.1:9001/wsg?role=server&serverAppId=081713074824&serverAppToken=Y8FG0tbs7pgujQccNcEABIuW1it2Rhfw"));
        client.setConnectionLostTimeout(120);
        client.connect();
    }
}
