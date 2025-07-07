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

/**
 * tcp接收端的速度，也会影响发送端的速度。
 * 有时满可能是因为接收端处理速度慢导致的。
 * 如果接收端处理速度很慢，也会导致发送端速度慢。
 * */
public class Main9001 {
    public static void main(String[] args) throws URISyntaxException, InterruptedException, IOException {
        WebSocketClient client = new WebSocketClient(new URI("ws://localhost:9001")) {
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
        client.setReceiveBufferSize(512*1024);
        client.connectBlocking();
        Thread.sleep(100);
        String text = RandomStringUtils.insecure().nextAlphabetic(1024);
        //Draft_6455.useFastMask = false;
        WebSocketImpl impl = (WebSocketImpl) client.getConnection();
        BinaryFrame binaryFrame = new BinaryFrame();
        binaryFrame.setPayload(ByteBuffer.wrap(text.getBytes(StandardCharsets.UTF_8)));
        ByteBuffer byteBuffer = impl.createEncodedBinaryFrame(binaryFrame);
        File binary = new File("binary.dat");
        FileUtils.writeByteArrayToFile(binary, byteBuffer.array());

        TextFrame textFrame = new TextFrame();
        String part = "hello world;hello world;hello world;hello world;hello world;hello world;hello world;hello world;";
        textFrame.setPayload(ByteBuffer.wrap(part.getBytes(StandardCharsets.UTF_8)));
        ByteBuffer textFrameByteBuffer = impl.createEncodedBinaryFrame(textFrame);
        File textFrameFile = new File("part_binary.dat");
        System.out.println(textFrameByteBuffer.array().length);
        FileUtils.writeByteArrayToFile(textFrameFile, textFrameByteBuffer.array());

        System.out.println(text);

        long start = System.currentTimeMillis();
        for(int i=0; i<1000*1000*10; i++) {
            client.send(text);
        }



        System.out.println("send complete " + (System.currentTimeMillis() - start));

    }
}