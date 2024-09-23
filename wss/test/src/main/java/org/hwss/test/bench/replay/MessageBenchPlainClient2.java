package org.hwss.test.bench.replay;

import com.alibaba.fastjson2.JSON;
import org.apache.commons.lang3.RandomUtils;
import org.java_websocket.client.WebSocketClient;
import org.java_websocket.handshake.ServerHandshake;

import java.net.URI;
import java.nio.ByteBuffer;
import java.util.*;

/***
 * hwss
 * start new turn total message 2000002
 * receive used 8986.37ms
 *
 * start new turn total message 2000002 admin size 2
 * receive used 9792.13ms
 *
 * start new turn total message 2000002
 * receive used 8841.89ms
 *
 * admin
 * received message size : 1961148
 * received message used : 8200
 *
 * received message size : 2000002
 * received message used : 8839
 *
 * admin role:
 * received message size : 1971886
 * received message used : 7657
 *
 * received message size : 1920734
 * received message used : 8255
 *
 * client start new turn total message 2000002
 * receive used 8264.83ms
 * handle message done 2000002
 * handle message total used 8434.88ms
 *
 * 参考只接受，不发送：
 * start new turn total message 2000002
 *  * receive used 6010.39ms
 *  每秒30万左右。
 *
 *  加密一个字符，和16个字符，加密结果长度都一样。隐藏测试一下不加密的效果。
 *  start new turn total message 2000002
 * receive used 5264.74ms
 *
 * received message size : 2000002
 * received message used : 5263
 *
 * 40万每秒
 *
 * json ：
 * start new turn total message 2000002
 * receive used 24103.1ms
 *
 * start new turn total message 2000002
 * receive used 23905.6ms
 *
 * start new turn total message 2000002
 * receive used 22505.4ms
 *
 * start new turn total message 2000002
 * receive used 22447.5ms
 *
 * 每秒8-9万     200-300mb每秒
 *
 *
 *
 *
 * */
public class MessageBenchPlainClient2 extends WebSocketClient {

    private List<String> messages = new ArrayList<>();
    private  long startTime = 0;


    public MessageBenchPlainClient2(URI serverUri) {
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
        if (message.startsWith("start_")) {
            messages.clear();
            startTime = System.currentTimeMillis();
            System.out.println("start new turn received message size : " + messages.size());
        }
        //messages.add(message);
        if (message.startsWith("end_")) {
            System.out.println("received message size : " + messages.size());
            System.out.println("received message used : " + (System.currentTimeMillis() - startTime));
        }
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

        byte[] bts = RandomUtils.nextBytes(1024);

        Map<String, Object> json = new HashMap<>();
        json.put("action", "bmsg");
        json.put("msg",  Base64.getEncoder().encodeToString(bts));
        json.put("authId", UUID.randomUUID().toString());
        json.put("authToken", UUID.randomUUID().toString());
        json.put("appId", RandomUtils.nextInt(10, 2000));
        json.put("hwssId", UUID.randomUUID().toString());
        String msg =  JSON.toJSONString(json);


        // Create all-trusting host name verifier
        WebSocketClient client = new MessageBenchPlainClient2(new URI("ws://localhost:9002/wss?app_id=434&group=client"));
        client.connectBlocking();
        client.getConnection();
        long start = System.currentTimeMillis();

        client.send("start_" + UUID.randomUUID().toString());
        for(int i=0; i < 10000*200; i++) {
            client.send(msg);
        }
        client.send("end_" + UUID.randomUUID().toString());

        System.out.println("send message end " + (System.currentTimeMillis() - start));
    }
}
