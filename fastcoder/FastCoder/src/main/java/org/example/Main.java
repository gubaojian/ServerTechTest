package org.example;

import com.wsg.protocol.BinaryProtocol;
import com.wsg.protocol.JsonProtocol;
import com.wsg.protocol.Protocol;
import org.apache.commons.lang3.RandomStringUtils;
import org.apache.commons.lang3.RandomUtils;

import java.util.Map;
import java.util.UUID;

//TIP 要<b>运行</b>代码，请按 <shortcut actionId="Run"/> 或
// 点击装订区域中的 <icon src="AllIcons.Actions.Execute"/> 图标。
public class Main {
    public static void main(String[] args) {
          testJson();
          testBinary();
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