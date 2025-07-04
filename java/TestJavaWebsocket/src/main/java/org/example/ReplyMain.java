package org.example;

import org.apache.commons.io.FileUtils;
import org.apache.commons.lang3.RandomStringUtils;
import org.java_websocket.WebSocketImpl;
import org.java_websocket.client.WebSocketClient;
import org.java_websocket.framing.BinaryFrame;
import org.java_websocket.framing.TextFrame;
import org.java_websocket.handshake.ServerHandshake;

import java.io.File;
import java.io.IOException;
import java.net.URI;
import java.net.URISyntaxException;
import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;
import java.util.UUID;

/**
 * tcp接收端的速度，也会影响发送端的速度。
 * 有时满可能是因为接收端处理速度慢导致的。
 * 如果接收端处理速度很慢，也会导致发送端速度慢。
 * */
public class ReplyMain {
    public static void main(String[] args) throws URISyntaxException, InterruptedException, IOException {
        WebSocketClient client = new WebSocketClient(new URI("ws://localhost:8887")) {
            @Override
            public void onOpen(ServerHandshake handshakedata) {

            }

            @Override
            public void onMessage(String message) {
                System.out.println("replay message " + message);
            }

            @Override
            public void onClose(int code, String reason, boolean remote) {

            }

            @Override
            public void onError(Exception ex) {

            }
        };
        client.connectBlocking();
        Thread.sleep(1000);

        client.send(UUID.randomUUID().toString());
        client.send(UUID.randomUUID().toString());
        client.send(UUID.randomUUID().toString());

        client.send("fast mark works well");

    }
}