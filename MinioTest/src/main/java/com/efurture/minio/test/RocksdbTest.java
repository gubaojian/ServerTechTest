package com.efurture.minio.test;

import io.minio.ObjectWriteResponse;
import io.minio.UploadObjectArgs;
import javafx.application.Application;
import javafx.stage.Stage;
import org.apache.commons.io.FileUtils;
import org.rocksdb.Options;
import org.rocksdb.RocksDB;
import org.rocksdb.RocksDBException;

import java.io.File;
import java.io.IOException;
import java.util.UUID;

public class RocksdbTest {

    static {
        xcccc.loadLibrary();
    }

    /**
     * 数据库存储路径
     * */
    private static String rocksdbFilePath = "/Users/efurture/data/rdata";

    /**
     * 数据库存储
     * */
    private static RocksDB fileStoreDB;


    public static void main(String[] args) throws IOException {
        initRocksDb();

        String[] datas = {"/Users/efurture/Downloads/settings.xml", "/Users/efurture/Downloads/5c919e180001d70808320538.jpg",
                "/Users/efurture/照片/2019/11/IMG_4420.JPG", "/Users/efurture/Downloads/房屋租赁合同简单版修改版本.pdf"};

        byte[][] dataBts = new byte[datas.length][];
        for(int i=0 ;i<dataBts.length; i++){
            dataBts[i] = FileUtils.readFileToByteArray(new File(datas[i]));
        }

        int numOfFiles = 10000;



        long start = System.currentTimeMillis();
        for(int i=0; i<numOfFiles; i++){
            byte[] bts =  dataBts[i%datas.length];
            String saveName = save(bts);

        }
        fileStoreDB.getDefaultColumnFamily();
        System.out.println("used " + (System.currentTimeMillis() - start)  + " ms ");
        fileStoreDB.close();

    }


    public static  String save(byte[] bts) {
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
            fileStoreDB = RocksDB.open(options, rocksdbFilePath);
        } catch (RocksDBException e) {
            throw new RuntimeException(e);
        }

    }

}
