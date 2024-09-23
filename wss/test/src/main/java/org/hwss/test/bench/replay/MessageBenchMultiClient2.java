package org.hwss.test.bench.replay;

import com.alibaba.fastjson2.JSON;
import netscape.javascript.JSObject;
import org.apache.commons.lang3.RandomUtils;
import org.java_websocket.client.WebSocketClient;
import org.java_websocket.handshake.ServerHandshake;

import java.net.URI;
import java.nio.ByteBuffer;
import java.util.*;

/**
 * send message end 12972
 * hwss:
 * start new turn total message 8000002 admin size 2
 * receive used 44027.1ms
 * admin1 & admin2 client
 *
 * start new turn total message 3625586 admin size 2
 * receive used 18721.3ms
 *
 * hwss： admin1
 * start new turn total message 5091181
 * receive used 25424.7ms
 * start new turn total message 8000002
 * receive used 36436.1ms
 *
 * hwss multi role :
 * client start new turn total message 8000002
 * receive used 12214.5ms
 *
 * 参考只接收不发送：
 *  start new turn total message 8000002
 *  receive used 11398.9ms
 *  每秒 80万左右
 *
 * 说明消息卡到admin接收端：
 * admin
 * received message size : 8000002
 * received message used : 37211
 *
 * received message size : 8000002
 * received message used : 36446
 *
 * admin multirole :
 * received message size : 2421198
 * received message used : 13269
 *
 * received message size : 3110848
 * received message used : 14464
 *
 * client start new turn total message 8000002
 * receive used 13052.3ms
 * handle message done 14000008
 * handle message total used 27888.4ms
 *
 * client start new turn total message 8000002
 * receive used 11869.2ms
 * handle message done 30106321
 * handle message total used 28282.5ms
 *
 * client start new turn total message 8000002
 * receive used 12622.4ms
 * handle message done 7886306
 * handle message total used 28379.3ms
 *
 * 速度发送速度卡在网卡上或者接收端的速度上。20-25万每秒。
 * receive used 12469ms
 * receive message after end 8000004
 * client start new turn total message 8000004
 * receive used 12469ms
 * handle message done 7967383
 * handle message total used 33012.5ms
 *
 * 每秒21万左右：
 * 时间可能大部分消耗在发送端。
 *
 * start new turn total message 7999763
 * receive used 13365.2ms
 *
 * start new turn total message 7999608
 * receive used 12758.2ms
 *
 * 每秒 : 66万
 *
 * 1024 byte json:
 * send message end 31734
 *
 *start new turn total message 8000005
 * receive used 49341.8ms
 *
 * start new turn total message 6819819
 * receive used 41131.6ms
 * start new turn total message 7277450
 * receive used 43166.6ms
 * start new turn total message 7812767
 * receive used 45398.6ms
 * start new turn total message 8000005
 * receive used 46676ms
 *
 * start new turn total message 7194629
 * receive used 43635.8ms
 * start new turn total message 7517745
 * receive used 45082.8ms
 * start new turn total message 7559903
 * receive used 45385.4ms
 * start new turn total message 8000005
 * receive used 48355.8ms
 *
 * 每秒16万  发送速度 400-500ms，接收高峰600-700mb
 *
 * 小点的数据：
 * start new turn total message 7446526
 * receive used 11627.4ms
 * start new turn total message 7581187
 * receive used 11764.8ms
 * start new turn total message 7965040
 * receive used 12436.7ms
 * start new turn total message 8000005
 * receive used 12567.4ms
 *
 * 数据1553减少到176字节. 每秒：66万消息。
 *
 * byte 1024改为：256
 * start new turn total message 7790906
 * receive used 22452.7ms
 * start new turn total message 7954672
 * receive used 22836.5ms
 * start new turn total message 7963173
 * receive used 22855.3ms
 * start new turn total message 8000005
 * receive used 23138.1ms
 *
 * 每秒：34万消息。
 *
 *
 * 1024 byte
 *
 * start new turn total message 5999229
 * receive used 39755.8ms
 * start new turn total message 6377813
 * receive used 41719.1ms
 * start new turn total message 6949252
 * receive used 45317.6ms
 * start new turn total message 8000005
 * receive used 52788.3ms
 *
 * */
public class MessageBenchMultiClient2 extends WebSocketClient {

    /***
     * 数据比较大，保存Java会oom，或者cpu飙升到100%处理速度很慢
    private List<String> messages = new ArrayList<>();
    private  long startTime = 0; */


    public MessageBenchMultiClient2(URI serverUri) {
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
        /***
        if (message.startsWith("start_")) {
            messages.clear();
            startTime = System.currentTimeMillis();
            System.out.println("start new turn received message size : " + messages.size());
        }
        messages.add(message);
        if (message.startsWith("end_")) {
            System.out.println("received message size : " + messages.size());
            System.out.println("received message used : " + (System.currentTimeMillis() - startTime));
        }*/
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

       System.out.println(msg);

        WebSocketClient mainClient = new MessageBenchMultiClient2(new URI("wss://localhost:9001/wss?app_id=434&group=client"));
        mainClient.setConnectionLostTimeout(180);
        mainClient.connectBlocking();
        long start = System.currentTimeMillis();
        mainClient.send("start_");
        Runnable runnable = new Runnable() {
            @Override
            public void run() {
                // Create all-trusting host name verifier
                 try {
                     WebSocketClient client = new MessageBenchMultiClient2(new URI("wss://localhost:9001/wss?app_id=434&group=client"));
                     client.setConnectionLostTimeout(180);
                     client.connectBlocking();
                     for(int i=0; i < 10000*100; i++) {
                         client.send(msg);
                     }
                     client.send("end_" + UUID.randomUUID().toString());
                 } catch (Exception e) {
                    throw new RuntimeException(e);
                }
            }
        };
        Thread thread1 = new Thread(runnable);
        Thread thread2 = new Thread(runnable);
        Thread thread3 = new Thread(runnable);
        thread1.start();;
        thread2.start();;
        thread3.start();;

        for(int i=0; i < 10000*100; i++) {
            mainClient.send(msg);
        }
        thread1.join();
        thread2.join();
        thread3.join();
        mainClient.send("end_" + UUID.randomUUID().toString());


        System.out.println("send message end " + (System.currentTimeMillis() - start));
    }
}
