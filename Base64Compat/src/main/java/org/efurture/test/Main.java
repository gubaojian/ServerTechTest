package org.efurture.test;

import jdkx.compat.util.Base64;
import jdkx.compat.util.Compat;
import jdkx.compat.util.HexFormat;
import okio.ByteString;

import java.nio.charset.StandardCharsets;

/**
 * aGVsbG8g5Lit5Zu95ZWK5biI5YKFMw==
 * aGVsbG8g5Lit5Zu9ODg=
 */
public class Main {
    public static void main(String[] args) {
        Compat.Math.addExact(0, 0);
        System.out.println(Base64.getEncoder().encodeToString("hello 中国啊师傅3".getBytes(StandardCharsets.UTF_8)));
        System.out.println(Base64.getUrlEncoder().encodeToString("hello 中国88".getBytes(StandardCharsets.UTF_8)));

        System.out.println(new String(Base64.getDecoder().decode("aGVsbG8g5Lit5Zu9ODg")));


        testBase64();

        testHex();
    }

    /**
     * jdk base64 used 73
     * jdkx.bouncycastle.util.Base64 used 378
     * */
    private static void testBase64() {
        String text = "hello 中国啊师傅3hello 中国啊师傅3hello 中国啊师傅3hello 中国啊师傅3hello 中国啊师傅3";
        byte[] bts = text.getBytes(StandardCharsets.UTF_8);
        {
            long start = System.currentTimeMillis();
            for(int i=0; i<10000*100; i++) {
                Base64.getEncoder().encodeToString(bts);
            }
            long end = System.currentTimeMillis();
            System.out.println("jdk base64 used " + (end -start ));
        }
        {
            long start = System.currentTimeMillis();
            for(int i=0; i<10000*100; i++) {
                jdkx.bouncycastle.util.Base64.toBase64String(bts);
            }
            long end = System.currentTimeMillis();
            System.out.println("jdkx.bouncycastle.util.Base64 used " + (end -start ));
        }
        {
            long start = System.currentTimeMillis();
            for(int i=0; i<10000*100; i++) {
                kotlin.io.encoding.Base64.Default.encode(bts, 0, bts.length);
            }
            long end = System.currentTimeMillis();
            System.out.println("kotlin.io.encoding.Base64 used " + (end -start ));
        }
        {
            long start = System.currentTimeMillis();
            for(int i=0; i<10000*100; i++) {
                jdkx.okio.Base64.encode(bts);
            }
            long end = System.currentTimeMillis();
            System.out.println("jdkx.okio.Base64 used " + (end -start ));
        }
        System.out.println(Base64.getEncoder().encodeToString(bts));
        System.out.println(jdkx.bouncycastle.util.Base64.toBase64String(bts));
        System.out.println(kotlin.io.encoding.Base64.Default.encode(bts, 0, bts.length));
        System.out.println(jdkx.okio.Base64.encode(bts));
    }
    /**
     * jdk HexFormat used 80
     * jdkx.bouncycastle.util.hex.Hex used 551
     * okio Hex used 80
     * */
    private static void testHex() {
        String text = "hello 中国啊师傅3hello 中国啊师傅3hello 中国啊师傅3hello 中国啊师傅3hello 中国啊师傅3";
        byte[] bts = text.getBytes(StandardCharsets.UTF_8);
        {
            long start = System.currentTimeMillis();
            for(int i=0; i<10000*100; i++) {
                HexFormat.of().formatHex(bts);
            }
            long end = System.currentTimeMillis();
            System.out.println("jdk HexFormat used " + (end -start ));
        }
        {
            long start = System.currentTimeMillis();
            for(int i=0; i<10000*100; i++) {
                jdkx.bouncycastle.util.hex.Hex.toHexString(bts);
            }
            long end = System.currentTimeMillis();
            System.out.println("jdkx.bouncycastle.util.hex.Hex used " + (end -start ));
        }
        {
            long start = System.currentTimeMillis();
            for(int i=0; i<10000*100; i++) {
                ByteString.of(bts).hex();
            }
            long end = System.currentTimeMillis();
            System.out.println("okio Hex used " + (end -start ));
        }
        System.out.println(HexFormat.of().formatHex(bts));
        System.out.println(jdkx.bouncycastle.util.hex.Hex.toHexString(bts));
        System.out.println(HexFormat.of().withUpperCase().formatHex(bts));
        System.out.println(ByteString.of(bts).hex());
    }
}