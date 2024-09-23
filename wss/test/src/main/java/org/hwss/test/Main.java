package org.hwss.test;

import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;

public class Main {

    private static ExecutorService executorService = Executors.newSingleThreadExecutor();

    public static void main(String[] args) throws InterruptedException {
        try {
            Runtime.getRuntime().addShutdownHook(new Thread(new Runnable() {
                @Override
                public void run() {
                    System.out.println("Hello shutdown hook 55");
                    executorService.shutdown();
                    try {
                        executorService.awaitTermination(2000, TimeUnit.MILLISECONDS);
                    } catch (InterruptedException e) {
                        throw new RuntimeException(e);
                    }
                    System.out.println("Hello shutdown hook ");
                }
            }));
            System.out.println("Hello world!");
            Thread.sleep(1000);
        } finally {
            System.out.println("Hello world! finally ");
        }
    }
}