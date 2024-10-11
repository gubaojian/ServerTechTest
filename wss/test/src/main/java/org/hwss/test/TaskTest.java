package org.hwss.test;

import java.util.Random;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

/**
 * 10000*300
 * used 1043ms
 * */
public class TaskTest {
    static  volatile int times = 0;
    static Random random = new Random();
    public static void main(String[] args) throws InterruptedException {
        long start = System.currentTimeMillis();
        ExecutorService executorService = Executors.newFixedThreadPool(4);
        for(int i=0; i<10000*300; i++) {
            executorService.submit(new Runnable() {
                @Override
                public void run() {
                    if (times%10 == 0) {
                        times = random.nextInt(10);
                    } else {
                        times ++;
                    }
                }
            });
        }
        long end = System.currentTimeMillis();
        System.out.println("send used " + (end - start) + "ms");
        System.out.println("used " + times);
        executorService.shutdown();
        System.out.println("used " + (System.currentTimeMillis() - start) + "ms");
    }
}
