package com.efurture.minio.test;

import io.minio.ObjectWriteResponse;
import io.minio.UploadObjectArgs;
import org.apache.commons.io.FileUtils;
import org.rocksdb.Env;
import org.rocksdb.Options;
import org.rocksdb.RocksDB;
import org.rocksdb.RocksDBException;

import java.io.File;
import java.io.IOException;
import java.util.UUID;
import java.util.concurrent.*;

public class RocksdbTest {

    static {
        RocksDB.loadLibrary();
    }

    /**
     * 数据库存储路径
     * */
    private static String rocksdbFilePath = "/Users/efurture/data/rdata";

    /**
     * 数据库存储
     * */
    private static RocksDB fileStoreDB;


    public static void main(String[] args) throws IOException, InterruptedException {
        initRocksDb();

        String[] datas = {"/Users/efurture/Downloads/settings.xml"};

        byte[][] dataBts = new byte[datas.length][];
        for(int i=0 ;i<dataBts.length; i++){
            dataBts[i] = FileUtils.readFileToByteArray(new File(datas[i]));
        }

        save(dataBts[0]);

        int numOfFiles = 10000*1000;


        long start = System.currentTimeMillis();
        for(int i=0; i<numOfFiles; i++){
            final byte[] bts =  dataBts[i%datas.length];
            String saveName = save(bts);

        }
        System.out.println("used " + (System.currentTimeMillis() - start)  + " ms ");
        fileStoreDB.close();

    }


    public static  synchronized String save(byte[] bts) {
        String uuid = UUID.randomUUID().toString();
        try {
            fileStoreDB.put(uuid.getBytes(), bts);
        } catch (RocksDBException e) {
            throw new RuntimeException(e);
        }
        return uuid;
    }

    public byte[] get(String key) {
        try {
            return fileStoreDB.get(key.getBytes());
        } catch (RocksDBException e) {
            throw new RuntimeException(e);
        }
    }


    private static void initRocksDb(){
        try {
            Options options = new Options().setCreateIfMissing(true);
            System.out.println(options.maxBackgroundCompactions() + " "
            + options.allowMmapReads() + "  " + options.allowMmapWrites()
            + "  " + Runtime.getRuntime().availableProcessors());
            int threads = 4; Math.min(4, Runtime.getRuntime().availableProcessors());
            //options.setAllowMmapReads(true);
            //options.setAllowMmapWrites(true);
            options.setIncreaseParallelism(threads);
            options.setMaxBackgroundCompactions(threads);
            // options.optimizeLevelStyleCompaction(1<<30);

            Env.getDefault().setBackgroundThreads(threads*2);

            options.setEnv(Env.getDefault());
            fileStoreDB = RocksDB.open(options, rocksdbFilePath);
        } catch (RocksDBException e) {
            throw new RuntimeException(e);
        }

    }

}
