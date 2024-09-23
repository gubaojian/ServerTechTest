package org.hwss.test.bench.replay;

import com.alibaba.fastjson2.JSON;
import org.apache.commons.lang3.RandomUtils;
import org.eclipse.jetty.websocket.api.Callback;
import org.eclipse.jetty.websocket.api.Session;
import org.eclipse.jetty.websocket.api.annotations.OnWebSocketMessage;
import org.eclipse.jetty.websocket.api.annotations.WebSocket;

import java.net.URI;
import java.util.*;
import java.util.concurrent.CompletableFuture;

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
 *  received message size : 2000002
 * received message used : 7361
 *
 * json
 * start new turn total message 2000002
 * receive used 23483.1ms
 *
 * start new turn total message 2000002
 * receive used 21990.3ms
 *
 * start new turn total message 2000002
 * receive used 21509.5ms
 *
 * 每秒8-9万  200-300mb每秒
 *
 * 性能太差：
 * start new turn total message 2000002
 * receive used 25498.5ms
 *
 * 速度也比较慢，估计做了过多验证。
 *
 * */
@WebSocket
public class MessageBenchJettyClient2  {

    private List<String> messages = new ArrayList<>();
    private  long startTime = 0;

    public MessageBenchJettyClient2() {
        super();
    }

    @OnWebSocketMessage
    public  void onMessage(String message) {
         //System.out.println(message);
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

        org.eclipse.jetty.websocket.client.WebSocketClient webSocketClient = new org.eclipse.jetty.websocket.client.WebSocketClient();

// Configure WebSocketClient, for example:
        webSocketClient.setMaxTextMessageSize(8 * 1024);

// Start WebSocketClient.
        webSocketClient.start();
        MessageBenchJettyClient2 clientEndPoint = new MessageBenchJettyClient2();
// The server URI to connect to.
        URI serverURI = URI.create("wss://localhost:9001/wss?app_id=434&group=client");

// Connect the client EndPoint to the server.
        CompletableFuture<Session> clientSessionPromise = webSocketClient.connect(clientEndPoint, serverURI);

        Session session = clientSessionPromise.get();

        // Create all-trusting host name verifier
        long start = System.currentTimeMillis();


       session.sendText("start_" + UUID.randomUUID().toString(), Callback.NOOP);
        for(int i=0; i < 10000*200; i++) {
            session.sendText(msg,  Callback.NOOP);
        }
        session.sendText("end_" + UUID.randomUUID().toString(),  Callback.NOOP);

        System.out.println("send message end " + (System.currentTimeMillis() - start));
    }


}
