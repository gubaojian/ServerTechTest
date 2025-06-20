package org.example;

import org.apache.commons.lang3.RandomStringUtils;
import org.apache.commons.lang3.StringUtils;
import org.java_websocket.client.WebSocketClient;
import org.java_websocket.drafts.Draft_6455;
import org.java_websocket.handshake.ServerHandshake;

import java.net.URI;
import java.net.URISyntaxException;

/**
 * tcp接收端的速度，也会影响发送端的速度。
 * 有时满可能是因为接收端处理速度慢导致的。
 * */
public class Main {
    public static void main(String[] args) throws URISyntaxException, InterruptedException {
        WebSocketClient client = new WebSocketClient(new URI("ws://localhost:8080")) {
            @Override
            public void onOpen(ServerHandshake handshakedata) {

            }

            @Override
            public void onMessage(String message) {

            }

            @Override
            public void onClose(int code, String reason, boolean remote) {

            }

            @Override
            public void onError(Exception ex) {

            }
        };
        client.connectBlocking();
        Thread.sleep(100);
        //Draft_6455.useFastMask = false;
        String text = RandomStringUtils.insecure().next(1024);
        long start = System.currentTimeMillis();
        for(int i=0; i<1000*1000; i++) {
            client.send(text);
        }

        System.out.println("send complete " + (System.currentTimeMillis() - start));

    }
}