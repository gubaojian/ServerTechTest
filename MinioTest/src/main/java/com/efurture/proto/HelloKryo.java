package com.efurture.proto;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.annotation.JSONField;
import com.efurture.util.CHStringUtil;
import com.esotericsoftware.kryo.Kryo;
import com.esotericsoftware.kryo.SerializerFactory;
import com.esotericsoftware.kryo.io.Input;
import com.esotericsoftware.kryo.io.Output;
import com.esotericsoftware.kryo.serializers.CompatibleFieldSerializer;
import com.esotericsoftware.reflectasm.FieldAccess;
import org.apache.commons.io.FileUtils;
import org.apache.commons.lang3.RandomUtils;
import org.msgpack.MessagePack;
import org.msgpack.annotation.Message;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.lang.reflect.Field;
import java.util.ArrayList;
import java.util.List;
import java.util.UUID;

public class HelloKryo {


    static public void main (String[] args) throws Exception {
        /**
         * kryo 缺点不支持添加字段，仅限在java语言
         * */
        Kryo kryo = new Kryo();
        kryo.setDefaultSerializer(CompatibleFieldSerializer.class);
        kryo.register(ArrayList.class);
        kryo.register(MessageDO.class);
        kryo.register(Messages.class);
        kryo.register(Person.class);
        kryo.register(byte[].class);
        long start = System.currentTimeMillis();

        Messages object = genMessage();

        start = System.currentTimeMillis();
        Output output = new Output(new FileOutputStream("file.bin"));
        kryo.writeObject(output, object);
        output.close();
        System.out.println((System.currentTimeMillis() - start) + "  " + object + " message size " + object.messages.size());

        start = System.currentTimeMillis();
        Input input = new Input(new FileInputStream("file.bin"));
        Messages object2 = kryo.readObject(input, Messages.class);
        input.close();
        System.out.println((System.currentTimeMillis() - start) + " "  + object2 + " message size " + object2.messages.size());

        start = System.currentTimeMillis();
        String  result = JSON.toJSONString(object);
        FileUtils.write(new File("file.json"), result);
        System.out.println((System.currentTimeMillis() - start)  + " length "  + result.getBytes().length);


        start = System.currentTimeMillis();
        byte[] sbts = FileUtils.readFileToByteArray(new File("file.json"));
        JSON.parseObject(sbts, Messages.class);
        System.out.println((System.currentTimeMillis() - start)  + " json parse length "  + result.getBytes().length);


        MessagePack packer = new MessagePack();
        packer.register(Messages.class);
        packer.register(MessageDO.class);
        packer.register(byte[].class);

        start = System.currentTimeMillis();
        byte[] bts = packer.write(object);
        FileUtils.writeByteArrayToFile(new File("file.mp"), bts);
        System.out.println((System.currentTimeMillis() - start)  + " pack length "  + bts.length);



        start = System.currentTimeMillis();
         bts = FileUtils.readFileToByteArray(new File("file.mp"));
        Messages messages = packer.read(bts, Messages.class);
        System.out.println((System.currentTimeMillis() - start)  + " pack length "  + bts.length
        + "  " + messages.messages.size());


        testRelect(object);
        testRelect2(object);
    }


    public static  void testRelect(Messages object) throws IllegalAccessException {

        Field[] fields =  MessageDO.class.getDeclaredFields();
        long start = System.currentTimeMillis();
       List<MessageDO>  messageDOS = object.getMessages();
       for(MessageDO messageDO : messageDOS){
           for(Field field : fields){
               field.get(messageDO);
           }
       }
        System.out.println( "reflect " + (System.currentTimeMillis() - start));
    }


    public static  void testRelect2(Messages object) throws IllegalAccessException {

        FieldAccess access = FieldAccess.get(MessageDO.class);
        long start = System.currentTimeMillis();
        List<MessageDO>  messageDOS = object.getMessages();
        for(MessageDO messageDO : messageDOS){
            for(int i=0; i<access.getFieldCount(); i++){
                access.get(messageDO, i);
            }
        }
        System.out.println( "reflect " + (System.currentTimeMillis() - start));
    }


    public  static  Messages genMessage(){
        Messages messages = new Messages();
        messages.id = System.currentTimeMillis();
        messages.uuid = UUID.randomUUID().toString();
        messages.owner = UUID.randomUUID().toString();
        messages.preMessages = UUID.randomUUID().toString();
        messages.nextMessages = UUID.randomUUID().toString();


        ArrayList<MessageDO> list = new ArrayList<>();
        int num = 10000;
        for(int i=0; i<num; i++){
            MessageDO message = new MessageDO();
            message.id = RandomUtils.nextLong(0, Long.MAX_VALUE);
            message.time = System.currentTimeMillis();
            int len = RandomUtils.nextInt(10, 50);
            message.message = CHStringUtil.randomString(len);
            list.add(message);
        }
        messages.messages = list;
        return  messages;
    }


    static public class Messages {
        long id;
        String uuid;
        String owner;

        String preMessages;
        String nextMessages;



        ArrayList<MessageDO> messages;

        long time;









        public long getId() {
            return id;
        }

        public void setId(long id) {
            this.id = id;
        }

        public String getUuid() {
            return uuid;
        }

        public void setUuid(String uuid) {
            this.uuid = uuid;
        }

        public String getOwner() {
            return owner;
        }

        public void setOwner(String owner) {
            this.owner = owner;
        }

        public String getNextMessages() {
            return nextMessages;
        }

        public void setNextMessages(String nextMessages) {
            this.nextMessages = nextMessages;
        }

        public String getPreMessages() {
            return preMessages;
        }

        public void setPreMessages(String preMessages) {
            this.preMessages = preMessages;
        }

        public ArrayList<MessageDO> getMessages() {
            return messages;
        }

        public void setMessages(ArrayList<MessageDO> messages) {
            this.messages = messages;
        }
    }



    @Message
    static public class MessageDO {
        long id;
        long time;
        String message;

        public long getId() {
            return id;
        }

        public void setId(long id) {
            this.id = id;
        }

        public long getTime() {
            return time;
        }

        public void setTime(long time) {
            this.time = time;
        }

        public String getMessage() {
            return message;
        }

        public void setMessage(String message) {
            this.message = message;
        }

    }


    static public class Person {
        long id;
        String name;
    }
}
