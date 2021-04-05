package com.efurture.minio.test;

import org.apache.commons.io.FileUtils;
import org.rocksdb.*;

import java.io.File;
import java.io.IOException;
import java.util.UUID;

public class RocksdbBatchTest {

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


    public static void main(String[] args) throws IOException, InterruptedException, RocksDBException {
        initRocksDb();

        String[] datas = {"/Users/efurture/Downloads/settings.xml", "/Users/efurture/Downloads/5c919e180001d70808320538.jpg",
                "/Users/efurture/照片/2019/11/IMG_4420.JPG", "/Users/efurture/Downloads/房屋租赁合同简单版修改版本.pdf"};

        byte[][] dataBts = new byte[datas.length][];
        for(int i=0 ;i<dataBts.length; i++){
            dataBts[i] = FileUtils.readFileToByteArray(new File(datas[i]));
        }

        save(dataBts[0]);

        int numOfFiles = 100000;

        int numBatchSize = 1000;
        int length = numOfFiles/numBatchSize;
        long start = System.currentTimeMillis();
        for(int i=0; i<length; i++){
            WriteBatch writeBatch = new WriteBatch();
            WriteOptions writeOptions = new WriteOptions();
            writeOptions.setSync(false);
            for(int m=0; m<numBatchSize; m++){
                int num = i*numBatchSize + m;
                final byte[] bts =  dataBts[num%datas.length];
                String uuid = UUID.randomUUID().toString();
                writeBatch.put(uuid.getBytes(), bts);
            }
            fileStoreDB.write(writeOptions, writeBatch);
            //writeBatch.close();
           // writeOptions.close();
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

            Env.getDefault().setBackgroundThreads(threads);

            options.setEnv(Env.getDefault());
            fileStoreDB = RocksDB.open(options, rocksdbFilePath);
        } catch (RocksDBException e) {
            throw new RuntimeException(e);
        }

    }

}
