package com.efurture.common;

public class UncaughtExceptionHandlerTest {

    public static void main(String[] args){

        Thread.setDefaultUncaughtExceptionHandler(new Thread.UncaughtExceptionHandler() {
            @Override
            public void uncaughtException(Thread t, Throwable e) {
                e.printStackTrace();
                System.out.println("Thread.UncaughtExceptionHandler() " + e.getMessage()
                + " " + e.getStackTrace());
                throw new RuntimeException("dddd");
            }
        });

        throw new RuntimeException("ccc");

    }
}
