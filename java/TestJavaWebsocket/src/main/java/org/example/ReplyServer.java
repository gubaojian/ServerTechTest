package org.example;

import org.java_websocket.WebSocket;
import org.java_websocket.client.WebSocketClient;
import org.java_websocket.handshake.ClientHandshake;
import org.java_websocket.handshake.ServerHandshake;
import org.java_websocket.server.WebSocketServer;

import java.io.IOException;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.URI;
import java.net.URISyntaxException;
import java.util.UUID;

/**
 * tcp接收端的速度，也会影响发送端的速度。
 * 有时满可能是因为接收端处理速度慢导致的。
 * 如果接收端处理速度很慢，也会导致发送端速度慢。
 * */
public class ReplyServer {
    public static void main(String[] args) throws URISyntaxException, InterruptedException, IOException {
        String host = "localhost";
        int port = 8887;
        InetSocketAddress address = new InetSocketAddress(host, port);
        WebSocketServer server = new WebSocketServer(address) {
            @Override
            public void onOpen(WebSocket conn, ClientHandshake handshake) {

            }

            @Override
            public void onClose(WebSocket conn, int code, String reason, boolean remote) {

            }

            @Override
            public void onMessage(WebSocket conn, String message) {
                System.out.println(message);
                conn.send("reply " + message);
            }

            @Override
            public void onError(WebSocket conn, Exception ex) {

            }

            @Override
            public void onStart() {

            }
        };
        server.setReuseAddr(false);
        server.run();


    }
}