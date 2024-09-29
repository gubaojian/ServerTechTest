package org.hwss.test.bench.replay;

import org.java_websocket.client.WebSocketClient;
import org.java_websocket.handshake.ServerHandshake;

import java.net.URI;
import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.List;
import java.util.UUID;

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
 * start new turn total message 7999992
 * receive used 12639.2ms
 *
 * received message size : 2000002
 * received message used : 12642
 *
 * received message size : 2000002
 * received message used : 11668
 *
 * start new turn total message 7999992
 * receive used 12372.2ms
 *
 * 66万每秒。
 *
 * wss加密不加密ws区别不大， 说明限制在网卡。数据小QPS提升明显，
 * 说明限制在包处理，及数据加密解密？限制在发送端？
 * 用 iperf3 测试本地tcp速度。
 *  iperf3 -s
 * iperf3 -c 192.168.1.66
 *
 * Connecting to host 192.168.1.66, port 5201
 * [  5] local 192.168.1.66 port 53380 connected to 192.168.1.66 port 5201
 * [ ID] Interval           Transfer     Bitrate
 * [  5]   0.00-1.00   sec  8.42 GBytes  72.3 Gbits/sec
 * [  5]   1.00-2.00   sec  8.42 GBytes  72.3 Gbits/sec
 * [  5]   2.00-3.00   sec  8.50 GBytes  73.1 Gbits/sec
 * [  5]   3.00-4.00   sec  8.62 GBytes  74.0 Gbits/sec
 * [  5]   4.00-5.00   sec  8.61 GBytes  74.0 Gbits/sec
 * [  5]   5.00-6.00   sec  7.66 GBytes  65.6 Gbits/sec
 * [  5]   6.00-7.00   sec  8.47 GBytes  72.9 Gbits/sec
 * [  5]   7.00-8.00   sec  8.54 GBytes  73.4 Gbits/sec
 * [  5]   8.00-9.00   sec  8.51 GBytes  72.9 Gbits/sec
 * [  5]   9.00-10.00  sec  8.49 GBytes  73.0 Gbits/sec
 * - - - - - - - - - - - - - - - - - - - - - - - - -
 * [ ID] Interval           Transfer     Bitrate
 * [  5]   0.00-10.00  sec  84.3 GBytes  72.3 Gbits/sec                  sender
 * [  5]   0.00-10.00  sec  84.3 GBytes  72.3 Gbits/sec
 *
 * iperf3 -c 127.0.0.1
 * Connecting to host 127.0.0.1, port 5201
 * [  5] local 127.0.0.1 port 53710 connected to 127.0.0.1 port 5201
 * [ ID] Interval           Transfer     Bitrate
 * [  5]   0.00-1.00   sec  7.29 GBytes  62.4 Gbits/sec
 * [  5]   1.00-2.00   sec  6.83 GBytes  58.9 Gbits/sec
 * [  5]   2.00-3.00   sec  7.41 GBytes  63.7 Gbits/sec
 * [  5]   3.00-4.00   sec  7.37 GBytes  63.3 Gbits/sec
 * [  5]   4.00-5.00   sec  7.49 GBytes  64.1 Gbits/sec
 * [  5]   5.00-6.00   sec  7.36 GBytes  63.2 Gbits/sec
 * [  5]   6.00-7.00   sec  7.41 GBytes  63.6 Gbits/sec
 * [  5]   7.00-8.00   sec  7.39 GBytes  63.7 Gbits/sec
 * [  5]   8.00-9.00   sec  7.38 GBytes  63.4 Gbits/sec
 * [  5]   9.00-10.00  sec  7.45 GBytes  64.0 Gbits/sec
 * - - - - - - - - - - - - - - - - - - - - - - - - -
 * [ ID] Interval           Transfer     Bitrate
 * [  5]   0.00-10.00  sec  73.4 GBytes  63.0 Gbits/sec                  sender
 * [  5]   0.00-10.00  sec  73.4 GBytes  63.0 Gbits/sec
 *
 * 多线程反而增加切换开销：
 * iperf3 -c 192.168.1.66 -P 4
 * Connecting to host 192.168.1.66, port 5201
 * [  5] local 192.168.1.66 port 53980 connected to 192.168.1.66 port 5201
 * [  8] local 192.168.1.66 port 53981 connected to 192.168.1.66 port 5201
 * [ 10] local 192.168.1.66 port 53982 connected to 192.168.1.66 port 5201
 * [ 12] local 192.168.1.66 port 53983 connected to 192.168.1.66 port 5201
 * [ ID] Interval           Transfer     Bitrate
 * [  5]   0.00-1.00   sec  1.22 GBytes  10.5 Gbits/sec
 * [  8]   0.00-1.00   sec  1.06 GBytes  9.08 Gbits/sec
 * [ 10]   0.00-1.00   sec  1.22 GBytes  10.5 Gbits/sec
 * [ 12]   0.00-1.00   sec  1.06 GBytes  9.08 Gbits/sec
 * [SUM]   0.00-1.00   sec  4.56 GBytes  39.2 Gbits/sec
 * - - - - - - - - - - - - - - - - - - - - - - - - -
 * [  5]   1.00-2.00   sec  1.19 GBytes  10.2 Gbits/sec
 * [  8]   1.00-2.00   sec  1.19 GBytes  10.2 Gbits/sec
 * [ 10]   1.00-2.00   sec  1.19 GBytes  10.2 Gbits/sec
 * [ 12]   1.00-2.00   sec  1.19 GBytes  10.2 Gbits/sec
 * [SUM]   1.00-2.00   sec  4.77 GBytes  40.9 Gbits/sec
 * - - - - - - - - - - - - - - - - - - - - - - - - -
 * [  5]   2.00-3.00   sec  1.19 GBytes  10.3 Gbits/sec
 * [  8]   2.00-3.00   sec  1.19 GBytes  10.3 Gbits/sec
 * [ 10]   2.00-3.00   sec  1.19 GBytes  10.3 Gbits/sec
 * [ 12]   2.00-3.00   sec  1.19 GBytes  10.3 Gbits/sec
 * [SUM]   2.00-3.00   sec  4.78 GBytes  41.0 Gbits/sec
 * - - - - - - - - - - - - - - - - - - - - - - - - -
 * [  5]   3.00-4.00   sec  1.22 GBytes  10.5 Gbits/sec
 * [  8]   3.00-4.00   sec  1.22 GBytes  10.5 Gbits/sec
 * [ 10]   3.00-4.00   sec  1.22 GBytes  10.5 Gbits/sec
 * [ 12]   3.00-4.00   sec  1.22 GBytes  10.5 Gbits/sec
 * [SUM]   3.00-4.00   sec  4.89 GBytes  42.1 Gbits/sec
 * - - - - - - - - - - - - - - - - - - - - - - - - -
 * [  5]   4.00-5.00   sec  1.19 GBytes  10.2 Gbits/sec
 * [  8]   4.00-5.00   sec  1.18 GBytes  10.2 Gbits/sec
 * [ 10]   4.00-5.00   sec  1.19 GBytes  10.2 Gbits/sec
 * [ 12]   4.00-5.00   sec  1.19 GBytes  10.2 Gbits/sec
 * [SUM]   4.00-5.00   sec  4.74 GBytes  40.7 Gbits/sec
 * - - - - - - - - - - - - - - - - - - - - - - - - -
 * [  5]   5.00-6.00   sec  1.18 GBytes  10.1 Gbits/sec
 * [  8]   5.00-6.00   sec  1.18 GBytes  10.1 Gbits/sec
 * [ 10]   5.00-6.00   sec  1.18 GBytes  10.1 Gbits/sec
 * [ 12]   5.00-6.00   sec  1.18 GBytes  10.1 Gbits/sec
 * [SUM]   5.00-6.00   sec  4.70 GBytes  40.4 Gbits/sec
 * - - - - - - - - - - - - - - - - - - - - - - - - -
 * [  5]   6.00-7.00   sec  1.21 GBytes  10.4 Gbits/sec
 * [  8]   6.00-7.00   sec  1.21 GBytes  10.4 Gbits/sec
 * [ 10]   6.00-7.00   sec  1.21 GBytes  10.4 Gbits/sec
 * [ 12]   6.00-7.00   sec  1.21 GBytes  10.4 Gbits/sec
 * [SUM]   6.00-7.00   sec  4.83 GBytes  41.4 Gbits/sec
 * - - - - - - - - - - - - - - - - - - - - - - - - -
 * [  5]   7.00-8.00   sec  1.22 GBytes  10.5 Gbits/sec
 * [  8]   7.00-8.00   sec  1.22 GBytes  10.5 Gbits/sec
 * [ 10]   7.00-8.00   sec  1.22 GBytes  10.5 Gbits/sec
 * [ 12]   7.00-8.00   sec  1.22 GBytes  10.5 Gbits/sec
 * [SUM]   7.00-8.00   sec  4.88 GBytes  41.9 Gbits/sec
 * - - - - - - - - - - - - - - - - - - - - - - - - -
 * [  5]   8.00-9.00   sec  1.25 GBytes  10.7 Gbits/sec
 * [  8]   8.00-9.00   sec  1.25 GBytes  10.7 Gbits/sec
 * [ 10]   8.00-9.00   sec  1.25 GBytes  10.7 Gbits/sec
 * [ 12]   8.00-9.00   sec  1.25 GBytes  10.7 Gbits/sec
 * [SUM]   8.00-9.00   sec  4.99 GBytes  42.8 Gbits/sec
 * - - - - - - - - - - - - - - - - - - - - - - - - -
 * [  5]   9.00-10.00  sec  1.21 GBytes  10.4 Gbits/sec
 * [  8]   9.00-10.00  sec  1.21 GBytes  10.4 Gbits/sec
 * [ 10]   9.00-10.00  sec  1.21 GBytes  10.4 Gbits/sec
 * [ 12]   9.00-10.00  sec  1.21 GBytes  10.4 Gbits/sec
 * [SUM]   9.00-10.00  sec  4.85 GBytes  41.8 Gbits/sec
 * - - - - - - - - - - - - - - - - - - - - - - - - -
 * [ ID] Interval           Transfer     Bitrate
 * [  5]   0.00-10.00  sec  12.1 GBytes  10.4 Gbits/sec                  sender
 * [  5]   0.00-10.00  sec  12.1 GBytes  10.4 Gbits/sec                  receiver
 * [  8]   0.00-10.00  sec  11.9 GBytes  10.2 Gbits/sec                  sender
 * [  8]   0.00-10.00  sec  11.9 GBytes  10.2 Gbits/sec                  receiver
 * [ 10]   0.00-10.00  sec  12.1 GBytes  10.4 Gbits/sec                  sender
 * [ 10]   0.00-10.00  sec  12.1 GBytes  10.4 Gbits/sec                  receiver
 * [ 12]   0.00-10.00  sec  11.9 GBytes  10.2 Gbits/sec                  sender
 * [ 12]   0.00-10.00  sec  11.9 GBytes  10.2 Gbits/sec                  receiver
 * [SUM]   0.00-10.00  sec  48.0 GBytes  41.2 Gbits/sec                  sender
 * [SUM]   0.00-10.00  sec  48.0 GBytes  41.2 Gbits/sec                  receiver
 *
 *iperf3 -c 192.168.1.66 -P 2
 * Connecting to host 192.168.1.66, port 5201
 * [  5] local 192.168.1.66 port 54063 connected to 192.168.1.66 port 5201
 * [  8] local 192.168.1.66 port 54064 connected to 192.168.1.66 port 5201
 * [ ID] Interval           Transfer     Bitrate
 * [  5]   0.00-1.00   sec  4.99 GBytes  42.7 Gbits/sec
 * [  8]   0.00-1.00   sec  5.00 GBytes  42.8 Gbits/sec
 * [SUM]   0.00-1.00   sec  9.99 GBytes  85.5 Gbits/sec
 * - - - - - - - - - - - - - - - - - - - - - - - - -
 * [  5]   1.00-2.00   sec  5.19 GBytes  44.6 Gbits/sec
 * [  8]   1.00-2.00   sec  5.20 GBytes  44.7 Gbits/sec
 * [SUM]   1.00-2.00   sec  10.4 GBytes  89.2 Gbits/sec
 * - - - - - - - - - - - - - - - - - - - - - - - - -
 * [  5]   2.00-3.00   sec  5.30 GBytes  45.6 Gbits/sec
 * [  8]   2.00-3.00   sec  5.30 GBytes  45.6 Gbits/sec
 * [SUM]   2.00-3.00   sec  10.6 GBytes  91.2 Gbits/sec
 * - - - - - - - - - - - - - - - - - - - - - - - - -
 * [  5]   3.00-4.00   sec  5.02 GBytes  43.2 Gbits/sec
 * [  8]   3.00-4.00   sec  5.01 GBytes  43.1 Gbits/sec
 * [SUM]   3.00-4.00   sec  10.0 GBytes  86.3 Gbits/sec
 * - - - - - - - - - - - - - - - - - - - - - - - - -
 * [  5]   4.00-5.00   sec  4.51 GBytes  38.6 Gbits/sec
 * [  8]   4.00-5.00   sec  4.51 GBytes  38.6 Gbits/sec
 * [SUM]   4.00-5.00   sec  9.02 GBytes  77.2 Gbits/sec
 * - - - - - - - - - - - - - - - - - - - - - - - - -
 * [  5]   5.00-6.00   sec  4.80 GBytes  41.2 Gbits/sec
 * [  8]   5.00-6.00   sec  4.79 GBytes  41.2 Gbits/sec
 * [SUM]   5.00-6.00   sec  9.59 GBytes  82.4 Gbits/sec
 * - - - - - - - - - - - - - - - - - - - - - - - - -
 * [  5]   6.00-7.00   sec  4.84 GBytes  41.5 Gbits/sec
 * [  8]   6.00-7.00   sec  4.84 GBytes  41.5 Gbits/sec
 * [SUM]   6.00-7.00   sec  9.67 GBytes  83.0 Gbits/sec
 * - - - - - - - - - - - - - - - - - - - - - - - - -
 * [  5]   7.00-8.00   sec  4.83 GBytes  41.5 Gbits/sec
 * [  8]   7.00-8.00   sec  4.83 GBytes  41.6 Gbits/sec
 * [SUM]   7.00-8.00   sec  9.65 GBytes  83.1 Gbits/sec
 * - - - - - - - - - - - - - - - - - - - - - - - - -
 * [  5]   8.00-9.00   sec  5.24 GBytes  45.1 Gbits/sec
 * [  8]   8.00-9.00   sec  5.24 GBytes  45.1 Gbits/sec
 * [SUM]   8.00-9.00   sec  10.5 GBytes  90.1 Gbits/sec
 * - - - - - - - - - - - - - - - - - - - - - - - - -
 * [  5]   9.00-10.00  sec  5.52 GBytes  47.2 Gbits/sec
 * [  8]   9.00-10.00  sec  5.52 GBytes  47.2 Gbits/sec
 * [SUM]   9.00-10.00  sec  11.0 GBytes  94.5 Gbits/sec
 * - - - - - - - - - - - - - - - - - - - - - - - - -
 * [ ID] Interval           Transfer     Bitrate
 * [  5]   0.00-10.00  sec  50.2 GBytes  43.1 Gbits/sec                  sender
 * [  5]   0.00-10.00  sec  50.2 GBytes  43.1 Gbits/sec                  receiver
 * [  8]   0.00-10.00  sec  50.2 GBytes  43.1 Gbits/sec                  sender
 * [  8]   0.00-10.00  sec  50.2 GBytes  43.1 Gbits/sec                  receiver
 * [SUM]   0.00-10.00  sec   100 GBytes  86.3 Gbits/sec                  sender
 * [SUM]   0.00-10.00  sec   100 GBytes  86.3 Gbits/sec                  receiver
 *
 * */
public class MessageBenchMultiPlainClient extends WebSocketClient {

    private List<String> messages = new ArrayList<>();
    private  long startTime = 0;


    public MessageBenchMultiPlainClient(URI serverUri) {
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
        messages.add(message);
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

        WebSocketClient mainClient = new MessageBenchMultiPlainClient(new URI("ws://localhost:9002/wss?app_id=434&group=client"));
        mainClient.connectBlocking();
        long start = System.currentTimeMillis();
        mainClient.send("start_");
        Runnable runnable = new Runnable() {
            @Override
            public void run() {
                // Create all-trusting host name verifier
                 try {
                     WebSocketClient client = new MessageBenchMultiPlainClient(new URI("ws://localhost:9002/wss?app_id=434&group=client"));
                     client.connectBlocking();
                     client.send("start_");
                     for(int i=0; i < 10000*200; i++) {
                         client.send(UUID.randomUUID().toString());
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

        for(int i=0; i < 10000*200; i++) {
            mainClient.send(UUID.randomUUID().toString());
        }
        thread1.join();
        thread2.join();
        thread3.join();
        mainClient.send("end_" + UUID.randomUUID().toString());


        System.out.println("send message end " + (System.currentTimeMillis() - start));
    }
}
