package com.efurture.proto;

public class GCTest {
    
    /**
     * GC参数：
     *
     * -verbose:gc -XX:+PrintGCDetails -XX:+PrintGCTimeStamps -XX:+PrintGCApplicationStoppedTime -XX:+PrintReferenceGC -Dcom.sun.management.jmxremote.authenticate=false -Dcom.sun.management.jmxremote.ssl=false
     * */
    public static void main(String args[]){
        for(int i=0; i<1000; i++){
            byte[] bts = new byte[1024*1024];
            //bts = null;
        }
    }
}
