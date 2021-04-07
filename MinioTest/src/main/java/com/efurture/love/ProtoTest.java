package com.efurture.love;

import com.efurture.util.CHStringUtil;
import org.apache.commons.io.FileUtils;
import org.apache.commons.lang3.RandomUtils;

import java.io.File;
import java.io.IOException;
import java.util.UUID;

public class ProtoTest {


    static public void main (String[] args) throws Exception {
        MessageOuterClass.Messages messages = genMessage();
        long start = System.currentTimeMillis();
        byte[] bts =  messages.toByteArray();
        FileUtils.writeByteArrayToFile(new File("file.pb"), bts);
        System.out.println((System.currentTimeMillis() - start)  + " pb length "  + bts.length
         + " " +  messages.getMessagesList().size());

        testRead();


    }

    public static   void testRead() throws IOException {
        long start = System.currentTimeMillis();
        byte[] bts = FileUtils.readFileToByteArray(new File("file.pb"));
        com.efurture.love.MessageOuterClass.Messages.Builder messages = com.efurture.love.MessageOuterClass.Messages.newBuilder();
        messages.mergeFrom(bts);
        System.out.println((System.currentTimeMillis() - start)  + " pb length "  + bts.length
                + " " +  messages.getMessagesList().size());
    }





    public  static  MessageOuterClass.Messages genMessage(){
        com.efurture.love.MessageOuterClass.Messages.Builder messages = com.efurture.love.MessageOuterClass.Messages.newBuilder();
        messages.setId(System.currentTimeMillis());
        messages.setUuid(UUID.randomUUID().toString());
        messages.setOwner(UUID.randomUUID().toString());
        messages.setPreMessage(UUID.randomUUID().toString());
        messages.setNextMessage(UUID.randomUUID().toString());


       int num = 10000;
        for(int i=0; i<num; i++){
            com.efurture.love.MessageOuterClass.Message.Builder message = com.efurture.love.MessageOuterClass.Message.newBuilder();
            message.setId(RandomUtils.nextLong(0, Long.MAX_VALUE));
            message.setTime(System.currentTimeMillis());
            int len = RandomUtils.nextInt(100, 500);
            message.setMessage(CHStringUtil.randomString(len));
            messages.addMessages(message);
        }
        return  messages.build();
    }
}
