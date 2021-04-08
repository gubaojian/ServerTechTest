package com.efurture.redis;

import org.apache.commons.io.FileUtils;
import org.junit.Test;
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

    /**
     * 插入性能每秒10万左右
     * */
    @Test
    public void testPut() throws IOException, InterruptedException {

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


    /**
     * 每秒 15-20万
     * */
    @Test
    public void testGet() throws IOException, InterruptedException {

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
            final String object = UUID.randomUUID().toString();
            executor.execute(new Runnable() {
                @Override
                public void run() {
                    Jedis jedis =  local.get();
                    jedis.get(object.getBytes());
                }
            });
        }
        executor.shutdown();
        executor.awaitTermination(10, TimeUnit.DAYS);
        System.out.println("used " + (System.currentTimeMillis() - start)  + " ms ");
    }


    /**
     * 每秒 15-20万
     * */
    @Test
    public void testGet2() throws IOException, InterruptedException {

        String[] datas = {"/Users/efurture/Downloads/settings.xml"};

        byte[][] dataBts = new byte[datas.length][];
        for(int i=0 ;i<dataBts.length; i++){
            dataBts[i] = FileUtils.readFileToByteArray(new File(datas[i]));
        }

        ThreadPoolExecutor executor = new ThreadPoolExecutor(16, 16, 8, TimeUnit.MINUTES,
                new LinkedBlockingDeque<Runnable>());

        long start = System.currentTimeMillis();
        for(int m=0; m<32; m++) {
            executor.execute(new Runnable() {
                @Override
                public void run() {

                    for (int i = 0; i < 20000; i++) {
                        final String object = UUID.randomUUID().toString();
                        Jedis jedis = local.get();
                        jedis.get(object.getBytes());
                    }

                }
            });
        }
        executor.shutdown();
        executor.awaitTermination(10, TimeUnit.DAYS);
        System.out.println("used " + (System.currentTimeMillis() - start)  + " ms ");
    }
}
