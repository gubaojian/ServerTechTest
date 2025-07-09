package org.efurture.test;

import io.aeron.Aeron;
import io.aeron.Publication;
import io.aeron.Subscription;
import io.aeron.driver.MediaDriver;
import io.aeron.logbuffer.FragmentHandler;
import io.aeron.Aeron;
import io.aeron.Publication;
import org.agrona.concurrent.UnsafeBuffer;
import org.agrona.concurrent.SigInt;
import java.util.concurrent.atomic.AtomicBoolean;

public class AeronAsyncClient {
    private static final String CHANNEL = "aeron:udp?endpoint=localhost:40123";
    private static final int STREAM_ID = 10;
    private static final int MESSAGE_LENGTH = 1024;
    private static final int MESSAGES_TO_SEND = 1000*1000*2;

    public static void main(String[] args) {
        final AtomicBoolean running = new AtomicBoolean(true);

        // 注册 Ctrl+C 处理器
        SigInt.register(() -> running.set(false));
       try {

          Aeron aeron = Aeron.connect();

          Publication publication = aeron.addPublication(CHANNEL, STREAM_ID);

                // 等待发布者连接
                while (!publication.isConnected() && running.get()) {
                    System.out.println("Waiting for publication to connect...");
                    Thread.sleep(100);
                }

                System.out.println("Publication connected, sending messages...");

                // 准备消息缓冲区
                final byte[] messageBytes = new byte[MESSAGE_LENGTH];
                for (int i = 0; i < MESSAGE_LENGTH; i++) {
                    messageBytes[i] = (byte) (i % 127);
                }

                long start = System.currentTimeMillis();
                // 发送消息
                int messagesSent = 0;
                while (messagesSent < MESSAGES_TO_SEND) {
                    // 非阻塞发送
                    long result = publication.offer(new UnsafeBuffer(messageBytes));

                    if (result > 0) {
                        // 发送成功
                        messagesSent++;
                        if (messagesSent % 10000 == 0) {
                            System.out.printf("Sent message #%d%n", messagesSent);
                        }
                    } else if (result == Publication.BACK_PRESSURED) {
                        System.out.println("Back pressure detected, waiting");
                        Thread.sleep(1); // 短暂等待
                    } else if (result == Publication.NOT_CONNECTED) {
                        System.out.println("Publication is not connected, connecting...");
                        Thread.sleep(100);
                    } else if (result == Publication.ADMIN_ACTION) {
                        System.out.println("Administrative action in progress, retrying...");
                        Thread.sleep(1);
                    } else if (result == Publication.CLOSED) {
                        System.out.println("Publication is closed");
                        break;
                    } else {
                        System.out.printf("Unknown publication result: %d%n", result);
                        break;
                    }
                }

               System.out.println("Sent used " + (System.currentTimeMillis()  - start) + "ms");

                System.out.printf("Sent %d messages successfully%n", messagesSent);

                aeron.close();
        } catch (Exception e) {
            e.printStackTrace();
        }



    }
}