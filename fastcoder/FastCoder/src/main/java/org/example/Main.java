package org.example;

import com.wsg.protocol.PackProtocol;
import com.wsg.protocol.UnPackProtocol;
import org.apache.commons.lang3.RandomStringUtils;

import java.nio.charset.StandardCharsets;
import java.util.Map;
import java.util.UUID;

//TIP 要<b>运行</b>代码，请按 <shortcut actionId="Run"/> 或
// 点击装订区域中的 <icon src="AllIcons.Actions.Execute"/> 图标。
// normal mask used 2129
//fast32 mask used 1079
//fast64 mask used 976
//normal mask used 1174
//fast64 mask used 148
//fast128 mask used 173
public class Main {
    public static void main(String[] args) {
       testTextJson();
       testTextBinary();

        testBinaryJson();
        testBinaryBinary();
    }



    public static void testTextJson() {

        PackProtocol packProtocol = new PackProtocol();
        UnPackProtocol unPackProtocol = new UnPackProtocol();
        String wsgId = "wsg_88448848322";
        String connId = UUID.randomUUID().toString();
        String message = RandomStringUtils.insecure().nextAlphabetic(1024);
        byte[] bts = message.getBytes(StandardCharsets.UTF_8);
        long start = System.currentTimeMillis();
        byte[] packMessgae = null;
        for(int i=0; i<1024*10*200; i++) {
             packMessgae = packProtocol.jsonPackText(wsgId, connId, message);
        }
        long end = System.currentTimeMillis();
        System.out.println("json pack use "  + (end -start) + " length " + packMessgae.length);
        Map<String,Object> map = null;
        start = System.currentTimeMillis();
        for(int i=0; i<1024*10*200; i++) {
            map = unPackProtocol.jsonUnPack(packMessgae);
        }
        end = System.currentTimeMillis();
        System.out.println("json unpack use "  + (end -start));
        System.out.println(map);
    }

    public static void testTextBinary() {
        long start = System.currentTimeMillis();
        PackProtocol packProtocol = new PackProtocol();
        UnPackProtocol unPackProtocol = new UnPackProtocol();
        String wsgId = "wsg_88448848322";
        String connId = UUID.randomUUID().toString();
        String message = RandomStringUtils.insecure().nextAlphabetic(1024);
        byte[] bts = message.getBytes(StandardCharsets.UTF_8);
        byte[] packMessgae = null;
        for(int i=0; i<1024*10*200; i++) {
            packMessgae = packProtocol.binaryPackText(wsgId, connId, message);
        }
        long end = System.currentTimeMillis();
        System.out.println("binary use "  + (end -start)+ " length " + packMessgae.length);

        Map<String,Object> map = null;
        start = System.currentTimeMillis();
        for(int i=0; i<1024*10*200; i++) {
            map = unPackProtocol.binaryUnpack(packMessgae);
        }
        end = System.currentTimeMillis();
        System.out.println("binary unpack use "  + (end -start));
        System.out.println(map);
    }

    public static void testBinaryJson() {

        PackProtocol packProtocol = new PackProtocol();
        UnPackProtocol unPackProtocol = new UnPackProtocol();
        String wsgId = "wsg_88448848322";
        String connId = UUID.randomUUID().toString();
        String message = RandomStringUtils.insecure().nextAlphabetic(1024);
        byte[] bts = message.getBytes(StandardCharsets.UTF_8);
        long start = System.currentTimeMillis();
        byte[] packMessgae = null;
        for(int i=0; i<1024*10*200; i++) {
            packMessgae = packProtocol.jsonPackBinary(wsgId, connId, bts);
        }
        long end = System.currentTimeMillis();
        System.out.println("json pack binary  use "  + (end -start) + " length " + packMessgae.length);
        Map<String,Object> map = null;
        start = System.currentTimeMillis();
        for(int i=0; i<1024*10*200; i++) {
            map = unPackProtocol.jsonUnPack(packMessgae);
        }
        end = System.currentTimeMillis();
        System.out.println("json unpack binary  use "  + (end -start));
        System.out.println(map);
    }

    public static void testBinaryBinary() {
        long start = System.currentTimeMillis();
        PackProtocol packProtocol = new PackProtocol();
        UnPackProtocol unPackProtocol = new UnPackProtocol();
        String wsgId = "wsg_88448848322";
        String connId = UUID.randomUUID().toString();
        String message = RandomStringUtils.insecure().nextAlphabetic(1024);
        byte[] bts = message.getBytes(StandardCharsets.UTF_8);
        byte[] packMessgae = null;
        for(int i=0; i<1024*10*200; i++) {
            packMessgae = packProtocol.binaryPackBinary(wsgId, connId, bts);
        }
        long end = System.currentTimeMillis();
        System.out.println("binary binary use "  + (end -start)+ " length " + packMessgae.length);

        Map<String,Object> map = null;
        start = System.currentTimeMillis();
        for(int i=0; i<1024*10*200; i++) {
            map = unPackProtocol.binaryUnpack(packMessgae);
        }
        end = System.currentTimeMillis();
        System.out.println("binary unpack binary use "  + (end -start));
        System.out.println(map);
    }
}