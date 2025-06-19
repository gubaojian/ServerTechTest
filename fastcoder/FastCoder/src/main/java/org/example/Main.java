package org.example;

import com.wsg.protocol.BinaryProtocol;
import com.wsg.protocol.JsonProtocol;
import com.wsg.protocol.Protocol;
import org.apache.commons.lang3.RandomStringUtils;
import org.apache.commons.lang3.RandomUtils;
import org.xmpp.packet.IQ;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.charset.StandardCharsets;
import java.util.Map;
import java.util.UUID;

//TIP 要<b>运行</b>代码，请按 <shortcut actionId="Run"/> 或
// 点击装订区域中的 <icon src="AllIcons.Actions.Execute"/> 图标。
public class Main {
    public static void main(String[] args) {
          testJson();
          testBinary();

        int mask = RandomUtils.insecure().randomInt();
        String message = RandomStringUtils.insecure().next(1024);


        System.out.println(message);

        ByteBuffer one = testMask(message, mask);
        ByteBuffer two = testFastMask(message, mask);
        ByteBuffer three = testFastMask(message, mask);


        System.out.println(one.equals(two));
        System.out.println(three.equals(two));
        byte[] bts = two.array();
        ByteBuffer back = testFastMaskBack(bts, mask);
        System.out.println(new String(back.array()));

        //testMaskBench(message, mask);

        testXml();
    }


    public static void testXml() {
        final IQ pingRequest = new IQ(IQ.Type.get);
        pingRequest.setChildElement("ping", "urn:xmpp:ping");
        pingRequest.setFrom("sourcefrom23255555");
        pingRequest.setTo("8887346346");
        System.out.println(pingRequest.toXML());
    }

    public static  void testMaskBench(String message, int mask) {
        byte[] bts = message.getBytes(StandardCharsets.UTF_8);
        long start = System.currentTimeMillis();
        for(int i=0; i<1000*10*100; i++) {
            testMask(message, mask);
        }
        long end = System.currentTimeMillis();
        System.out.println("normal mask used " + (end - start));

        start = System.currentTimeMillis();
        for(int i=0; i<1000*10*100; i++) {
            testFastMask32(message, mask);
        }
        end = System.currentTimeMillis();
        System.out.println("fast32 mask used " + (end - start));

        start = System.currentTimeMillis();
        for(int i=0; i<1000*10*100; i++) {
            testFastMask(message, mask);
        }
        end = System.currentTimeMillis();
        System.out.println("fast64 mask used " + (end - start));

        start = System.currentTimeMillis();
        for(int i=0; i<1000*10*100; i++) {
            testMask(bts, mask);
        }
        end = System.currentTimeMillis();
        System.out.println("normal mask used " + (end - start));

        start = System.currentTimeMillis();
        for(int i=0; i<1000*10*100; i++) {
            testFastMask(bts, mask);
        }
        end = System.currentTimeMillis();
        System.out.println("fast64 mask used " + (end - start));
    }

    public static ByteBuffer testMask(String message, int mask) {
        ByteBuffer masks = ByteBuffer.allocate(4);
        masks.putInt(mask);
        byte[] bts = message.getBytes(StandardCharsets.UTF_8);
        ByteBuffer read = ByteBuffer.wrap(bts);
        ByteBuffer write = ByteBuffer.allocate(bts.length);
        int i=0;
        while (read.hasRemaining()) {
            write.put((byte) (read.get() ^ masks.get(i%4)));
            i++;
        }
        return write;
    }

    public static ByteBuffer testFastMask(String message, int mask) {
        ByteBuffer masks = ByteBuffer.allocate(8).order(ByteOrder.BIG_ENDIAN);;
        masks.putInt(mask);
        masks.putInt(mask);
        byte[] bts = message.getBytes(StandardCharsets.UTF_8);
        ByteBuffer read = ByteBuffer.wrap(bts);
        ByteBuffer write = ByteBuffer.allocate(bts.length);

        int len = read.remaining()/8;
        long maskLong = masks.getLong(0);
        for(int j=0; j<len; j++) {
           write.putLong( read.getLong()^maskLong);
        }
        int i=0;
        while (read.hasRemaining()) {
            write.put((byte) (read.get() ^ masks.get(i%4)));
            i++;
        }
        return write;
    }

    public static ByteBuffer testMask(byte[] bts, int mask) {
        ByteBuffer masks = ByteBuffer.allocate(4);
        masks.putInt(mask);
        ByteBuffer read = ByteBuffer.wrap(bts);
        ByteBuffer write = ByteBuffer.allocate(bts.length);
        int i=0;
        while (read.hasRemaining()) {
            write.put((byte) (read.get() ^ masks.get(i%4)));
            i++;
        }
        return write;
    }

    public static ByteBuffer testFastMask(byte[] bts, int mask) {
        ByteBuffer masks = ByteBuffer.allocate(8).order(ByteOrder.BIG_ENDIAN);;
        masks.putInt(mask);
        masks.putInt(mask);
        ByteBuffer read = ByteBuffer.wrap(bts);
        ByteBuffer write = ByteBuffer.allocate(bts.length);

        int len = read.remaining()/8;
        long maskLong = masks.getLong(0);
        for(int j=0; j<len; j++) {
            write.putLong( read.getLong()^maskLong);
        }
        int i=0;
        while (read.hasRemaining()) {
            write.put((byte) (read.get() ^ masks.get(i%4)));
            i++;
        }
        return write;
    }

    public static ByteBuffer testFastMaskBack(byte[] bts, int mask) {
        ByteBuffer masks = ByteBuffer.allocate(8);
        masks.putInt(mask);
        masks.putInt(mask);
        ByteBuffer read = ByteBuffer.wrap(bts);
        ByteBuffer write = ByteBuffer.allocate(bts.length);

        int len = read.capacity()/8;
        for(int j=0; j<len; j++) {
            write.putLong( read.getLong()^masks.getLong(0));
        }
        int i=0;
        while (read.hasRemaining()) {
            write.put((byte) (read.get() ^ masks.get(i%4)));
            i++;
        }
        return write;
    }

    public static ByteBuffer testFastMask32(String message, int mask) {
        ByteBuffer masks = ByteBuffer.allocate(4);
        masks.putInt(mask);
        byte[] bts = message.getBytes(StandardCharsets.UTF_8);
        ByteBuffer read = ByteBuffer.wrap(bts);
        ByteBuffer write = ByteBuffer.allocate(bts.length);

        int len = read.capacity()/4;
        for(int j=0; j<len; j++) {
            write.putInt( read.getInt()^masks.getInt(0));
        }
        int i=0;
        while (read.hasRemaining()) {
            write.put((byte) (read.get() ^ masks.get(i%4)));
            i++;
        }
        return write;
    }

    public static void testJson() {

        Protocol protocol = new JsonProtocol();
        String wsgId = "wsg_88448848322";
        String connId = UUID.randomUUID().toString();
        String message = RandomStringUtils.insecure().nextAlphabetic(1024);
        long start = System.currentTimeMillis();
        byte[] packMessgae = null;
        for(int i=0; i<1024*10*200; i++) {
             packMessgae = protocol.packText(wsgId, connId, message);
        }
        long end = System.currentTimeMillis();
        System.out.println("json pack use "  + (end -start) + " length " + packMessgae.length);
        Map<String,Object> map = null;
        start = System.currentTimeMillis();
        for(int i=0; i<1024*10*200; i++) {
            map = protocol.unpackBinary(packMessgae);
        }
        end = System.currentTimeMillis();
        System.out.println("json unpack use "  + (end -start));
        System.out.println(map);
    }

    public static void testBinary() {
        long start = System.currentTimeMillis();
        Protocol protocol = new BinaryProtocol();
        String wsgId = "wsg_88448848322";
        String connId = UUID.randomUUID().toString();
        String message = RandomStringUtils.insecure().nextAlphabetic(1024);
        byte[] packMessgae = null;
        for(int i=0; i<1024*10*200; i++) {
            packMessgae = protocol.packText(wsgId, connId, message);
        }
        long end = System.currentTimeMillis();
        System.out.println("binary use "  + (end -start)+ " length " + packMessgae.length);

        Map<String,Object> map = null;
        start = System.currentTimeMillis();
        for(int i=0; i<1024*10*200; i++) {
            map = protocol.unpackBinary(packMessgae);
        }
        end = System.currentTimeMillis();
        System.out.println("binary unpack use "  + (end -start));
        System.out.println(map);
    }
}