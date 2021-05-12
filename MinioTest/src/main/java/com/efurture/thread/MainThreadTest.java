package com.efurture.thread;

import java.util.*;
import java.util.concurrent.Semaphore;
import java.util.concurrent.atomic.*;

public class MainThreadTest {

    public static void main(String[] args) {
        //Scanner in = new Scanner(System.in);
        //int a = in.nextInt();
        //System.out.println(a);
        System.out.println("Hello World!");


        AtomicInteger waitNum = new AtomicInteger(2);
        Semaphore semaphore = new Semaphore(1);
        try {
            semaphore.acquire();
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        EchoThread oneThread = new EchoThread(1, waitNum, false, semaphore);
        EchoThread twoThread = new EchoThread(2, waitNum, false, semaphore);
        EchoThread threeThread = new EchoThread(3, waitNum, true, semaphore);

        threeThread.start();

        oneThread.start();
        twoThread.start();
    }

    public static synchronized void  print(int num){
        System.out.println(num);
    }

    public static class EchoThread extends Thread{
        public int num;
        public AtomicInteger waitNum;
        public boolean wait;
        Semaphore semaphore;

        public EchoThread(int num, AtomicInteger waitNum, boolean wait, Semaphore semaphore){
            this.num = num;
            this.waitNum = waitNum;
            this.wait = wait;
            this.semaphore = semaphore;
        }

        @Override
        public void run(){
            super.run();
            if(!wait){
                print(num);
                waitNum.decrementAndGet();
                if(waitNum.get() == 0){
                    semaphore.release();
                }
            }else{
                while (waitNum.get() > 0){
                    try {
                        semaphore.acquire();
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                }
                print(num);
            }
        }

    }
}
