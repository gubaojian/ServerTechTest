package org.efurture.test;

import org.apache.commons.lang3.RandomStringUtils;
import redis.clients.jedis.BinaryJedisPubSub;
import redis.clients.jedis.Jedis;

import java.nio.charset.StandardCharsets;
import java.util.Arrays;
// brew install redis
//TIP To <b>Run</b> code, press <shortcut actionId="Run"/> or
// click the <icon src="AllIcons.Actions.Execute"/> icon in the gutter.
public class PubMain {
    public static void main(String[] args) throws InterruptedException {
        String channel = "test";

        Jedis publisherJedis = new Jedis("localhost", 6379);
        publisherJedis.connect();
        Thread subThread = new Thread(new Runnable() {
            @Override
            public void run() {
                Jedis subscriberJedis = new Jedis("localhost", 6379);
                subscriberJedis.connect();
                subscriberJedis.subscribe(new BinaryJedisPubSub() {
                    @Override
                    public void onMessage(byte[] channel, byte[] message) {
                        super.onMessage(channel, message);
                    }
                }, channel.getBytes(StandardCharsets.UTF_8));
            }
        });
        subThread.start();
        Thread.sleep(1000);
        long start = System.currentTimeMillis();
        byte[] bts = RandomStringUtils.insecure().nextAlphanumeric(512).getBytes(StandardCharsets.UTF_8);
        for(int i=0; i<10000*200; i++) {
            publisherJedis.publish(bts, channel.getBytes(StandardCharsets.UTF_8));
        }
        long end = System.currentTimeMillis();
        System.out.println("send used "  + (end - start) + "ms");
        subThread.join();
    }
}