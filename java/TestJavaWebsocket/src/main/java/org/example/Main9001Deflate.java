package org.example;

import org.apache.commons.io.FileUtils;
import org.apache.commons.lang3.RandomStringUtils;
import org.java_websocket.WebSocketImpl;
import org.java_websocket.client.WebSocketClient;
import org.java_websocket.drafts.Draft_6455;
import org.java_websocket.extensions.permessage_deflate.PerMessageDeflateExtension;
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
public class Main9001Deflate {
    public static void main(String[] args) throws URISyntaxException, InterruptedException, IOException {
        Draft_6455 draft6455 = new Draft_6455(new PerMessageDeflateExtension());
        WebSocketClient client = new WebSocketClient(new URI("ws://localhost:9001"), draft6455) {
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
        //内网传输速度太慢。
        String text = "{\"hostCfg\":[{\"hostname\":\"i.taobao.com\",\"enable\":true,\"id\":2,\"text\":\"及时获取购买商品订单、物流信息\"},{\"hostname\":\"jianghu.taobao.com\",\"bizCode\":\"\",\"enable\":true,\"id\":3,\"text\":\"及时获取您的淘宝网最新消息\"},{\"hostname\":\"\",\"bizCode\":\"home\",\"enable\":true,\"id\":4,\"text\":\"及时获取您的淘宝网最新消息\"}],\"siteMsgRedirectionConfig\":[{\"allowJumpDomainList\":\"huodong.taobao.com,tmall.com,jianghu.taobao.com,market.m.taobao.com,i.taobao.com,survey.taobao.com\",\"redirectList\":\"http://message/root::https://market.m.taobao.com/app/im/chat/index.html#/,https://web.m.taobao.com/app/message-social-front/mpds-container/msg-error::https://i.taobao.com/my_itaobao,https://u.tb.cn/::https://i.taobao.com/my_itaobao,https://market.m.taobao.com/app/dinamic/h5-tb-logistics/router::https://i.taobao.com/my_itaobao,https://tb.cn/n/im/dynamic/chat.html::https://i.taobao.com/my_itaobao\",\"id\":1,\"disallowShowDomainList\":\"\"}],\"fatigueCfg\":[{\"days\":7,\"id\":1}],\"commonCfg\":[{\"disable\":false,\"id\":1}],\"siteMsgHostCfg\":[{\"bizCode\":\"home\",\"enable\":true,\"id\":1},{\"hostname\":\"jianghu.taobao.com\",\"enable\":true,\"id\":2},{\"hostname\":\"i.taobao.com\",\"enable\":true,\"id\":3},{\"hostname\":\"pre-wormhole.wapa.taobao.com\",\"enable\":true,\"id\":4},{\"hostname\":\"pages-fast.m.taobao.com\",\"enable\":true,\"id\":5}],\"browserCfg\":[{\"browserName\":\"Chrome\",\"id\":1},{\"browserName\":\"Edge\",\"id\":2},{\"browserName\":\"360\",\"id\":3}]}\n";
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