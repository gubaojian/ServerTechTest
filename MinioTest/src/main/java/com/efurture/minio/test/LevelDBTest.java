package com.efurture.minio.test;

import org.apache.commons.io.FileUtils;
import org.iq80.leveldb.DB;
import org.iq80.leveldb.DBFactory;
import org.iq80.leveldb.Options;
import org.iq80.leveldb.WriteBatch;
import org.iq80.leveldb.impl.Iq80DBFactory;

import java.io.File;
import java.io.IOException;
import java.util.UUID;

public class LevelDBTest {


    public static void main(String[] args) throws IOException {

        String path = "/Users/efurture/data/ldata";

        DBFactory factory = Iq80DBFactory.factory;
        File dir = new File(path);
//如果数据不需要reload，则每次重启，尝试清理磁盘中path下的旧数据。  
        //if(cleanup) {  
          //factory.destroy(dir,null);//清除文件夹内的所有文件。  
        //}  
        Options options = new Options().createIfMissing(true);
//重新open新的db  
        DB db = factory.open(dir,options);


        String[] datas = {"/Users/efurture/Downloads/settings.xml", "/Users/efurture/Downloads/5c919e180001d70808320538.jpg",
                "/Users/efurture/照片/2019/11/IMG_4420.JPG", "/Users/efurture/Downloads/房屋租赁合同简单版修改版本.pdf"};

        byte[][] dataBts = new byte[datas.length][];
        for(int i=0 ;i<dataBts.length; i++){
            dataBts[i] = FileUtils.readFileToByteArray(new File(datas[i]));
        }


        int numOfFiles = 10000;
        long start = 0;

        /*
        start = System.currentTimeMillis();
        for(int i=0; i<numOfFiles; i++){
            final byte[] bts =  dataBts[i%datas.length];
            String uuid = UUID.randomUUID().toString();
            db.put(uuid.getBytes(), bts);
        }
        System.out.println("used " + (System.currentTimeMillis() - start)  + " ms ");

         */



        int numBatchSize = 1000;
        int length = numOfFiles/numBatchSize;
        start = System.currentTimeMillis();
        for(int i=0; i<length; i++){
            WriteBatch writeBatch = db.createWriteBatch();
            for(int m=0; m<numBatchSize; m++){
                int num = i*numBatchSize + m;
                final byte[] bts =  dataBts[num%datas.length];
                String uuid = UUID.randomUUID().toString();
                writeBatch.put(uuid.getBytes(), bts);
            }
            db.write(writeBatch);
            writeBatch.close();
        }
        System.out.println("batch used " + (System.currentTimeMillis() - start)  + " ms ");


        db.close();
    }
}
