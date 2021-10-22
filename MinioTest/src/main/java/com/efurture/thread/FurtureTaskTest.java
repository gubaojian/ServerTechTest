package com.efurture.thread;

import org.junit.Test;

import java.util.concurrent.*;

public class FurtureTaskTest {

    /**
     * 15万左右
     * */
    @Test
    public void testSingleThreadOneRequest() throws ExecutionException, InterruptedException {
        ExecutorService executors = Executors.newFixedThreadPool(1);
        long start = System.currentTimeMillis();
        for (int i=0; i<150000; i++) {
            Future<?> furture = executors.submit(new Runnable() {
                @Override
                public void run() {

                }
            });
            furture.get();
        }
        System.out.println("end used " + (System.currentTimeMillis() - start));
    }


    /**
     * 50-80万左右
     * */
    @Test
    public void testSingleThreadMultiRequest() throws ExecutionException, InterruptedException {
        final ExecutorService serverThread = Executors.newFixedThreadPool(1);
        ExecutorService requestThreads = Executors.newFixedThreadPool(10);
        long requestStart = System.currentTimeMillis();
        for(int i=0; i<20; i++) {
            requestThreads.execute(new Runnable() {
                @Override
                public void run() {
                    try {
                        long start = System.currentTimeMillis();
                        for (int i = 0; i < 100000; i++) {
                            Future<?> furture = serverThread.submit(new Runnable() {
                                @Override
                                public void run() {

                                }
                            });
                            furture.get();
                        }
                        System.out.println("end used " + (System.currentTimeMillis() - start));
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                }
            });
        }
        requestThreads.shutdown();
        requestThreads.awaitTermination(Integer.MAX_VALUE, TimeUnit.DAYS);
        System.out.println("all end used " + (System.currentTimeMillis() - requestStart));

    }




}
