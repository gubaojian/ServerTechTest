package com.efurture.minio.test;

import org.apache.commons.io.FileUtils;
import redis.clients.jedis.Jedis;

import java.io.File;
import java.io.IOException;
import java.util.UUID;
import java.util.concurrent.LinkedBlockingDeque;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;

public class JedisTest {



    static ThreadLocal<Jedis> local = new ThreadLocal<Jedis>(){
        public Jedis initialValue(){
            return  new Jedis("localhost");
        }
    };

    public static void main(String[] args) throws IOException, InterruptedException {

        String[] datas = {"/Users/efurture/Downloads/settings.xml"};

        byte[][] dataBts = new byte[datas.length][];
        for(int i=0 ;i<dataBts.length; i++){
            dataBts[i] = FileUtils.readFileToByteArray(new File(datas[i]));
        }

        ThreadPoolExecutor executor = new ThreadPoolExecutor(32, 64, 8, TimeUnit.MINUTES,
                new LinkedBlockingDeque<Runnable>());

        int numOfFiles = 10000*10;
        long start = System.currentTimeMillis();

        for(int i=0; i<numOfFiles; i++){
            final byte[] bts =  dataBts[i%datas.length];
            final String object = UUID.randomUUID().toString();
            executor.execute(new Runnable() {
                @Override
                public void run() {
                    Jedis jedis =  local.get();

                    jedis.set(object.getBytes(), bts);
                }
            });
       }
        executor.shutdown();
        executor.awaitTermination(10, TimeUnit.DAYS);
        System.out.println("used " + (System.currentTimeMillis() - start)  + " ms ");
    }
}
