package org.hwss.test.bench.server;

import org.hwss.test.bench.single.MessageBenchAdmin;
import org.java_websocket.WebSocket;
import org.java_websocket.client.WebSocketClient;
import org.java_websocket.handshake.ClientHandshake;
import org.java_websocket.server.WebSocketServer;

import java.net.InetSocketAddress;
import java.net.URI;
import java.util.ArrayList;
import java.util.List;

public class NormalServerTest extends WebSocketServer {
    public NormalServerTest(InetSocketAddress inetSocketAddress) {
        super(inetSocketAddress);
    }

    @Override
    public void onOpen(WebSocket webSocket, ClientHandshake clientHandshake) {

    }

    @Override
    public void onClose(WebSocket webSocket, int i, String s, boolean b) {

    }

    private List<String> messages = new ArrayList<>();
    private  long startTime = 0;

    @Override
    public void onMessage(WebSocket webSocket, String message) {
        if (message.contains("start_")) {
            messages.clear();
            startTime = System.currentTimeMillis();
            System.out.println("start new turn received message size : " + messages.size());
        }
        messages.add(message);
        if (message.contains("end_")) {
            System.out.println("received message size : " + messages.size());
            System.out.println("received message used : " + (System.currentTimeMillis() - startTime));
        }
        webSocket.send(message);
    }

    @Override
    public void onError(WebSocket webSocket, Exception e) {

    }

    @Override
    public void onStart() {

    }

    public static void main(String[] args) throws Exception {
        // Create all-trusting host name verifier
        String host = "localhost";
        int port = 8889;

        WebSocketServer server = new NormalServerTest(new InetSocketAddress(host, port));
        server.run();
    }
}
