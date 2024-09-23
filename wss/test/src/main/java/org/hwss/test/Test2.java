package org.hwss.test;

import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;

public class Test2 {

    private static ExecutorService executorService = Executors.newSingleThreadExecutor();

    public static void main(String[] args) throws InterruptedException {
        executorService.execute(new Runnable() {

            @Override
            public void run() {
                System.out.println("first run" + Thread.currentThread().getName());
            }
        });
        executorService.execute(new Runnable() {

            @Override
            public void run() {
                throw  new RuntimeException("hello world");
            }
        });
        Thread.sleep(1000*1000);
        executorService.execute(new Runnable() {

            @Override
            public void run() {
                System.out.println("third run" + Thread.currentThread().getName());
            }
        });
        executorService.execute(new Runnable() {

            @Override
            public void run() {
                System.out.println("four run" + Thread.currentThread().getName());
            }
        });
        executorService.shutdown();
    }
}
