package org.hwss.test;

import java.net.*;
import java.nio.ByteBuffer;

import org.java_websocket.client.WebSocketClient;
import org.java_websocket.drafts.Draft;
import org.java_websocket.handshake.ServerHandshake;

public class EmptyClient extends WebSocketClient {

    public EmptyClient(URI serverUri, Draft draft) {
        super(serverUri, draft);
    }

    public EmptyClient(URI serverURI) {
        super(serverURI);
    }

    @Override
    public void onOpen(ServerHandshake handshakedata) {
        long start = System.currentTimeMillis();
        for(int i=0; i<10000; i++) {
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

    public static void main(String[] args) throws URISyntaxException, UnknownHostException {
        WebSocketClient client = new EmptyClient(new URI("ws://localhost:8887"));
        //client.connect();
        InetAddress addr =  Inet4Address.getByAddress("localhost", IPAddressUtil.textToNumericFormatV4("127.0.0.1"));
        InetSocketAddress address = new InetSocketAddress(addr, 8887);
        System.out.println(addr + " " + address);
        System.out.println( InetSocketAddress.createUnresolved("localhost", 8887));

    }
}