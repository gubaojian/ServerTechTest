package org.example;

import com.wsg.protocol.binary.SmallStringCache;

import java.io.IOException;
import java.nio.charset.StandardCharsets;

public class StringCacheTest {

    public static void main(String[] args) throws IOException, InterruptedException {
        testStringPool();
    }

    public static void testStringPool() {
        String wsgId = "wsg_8844884832266";
        long start = System.currentTimeMillis();
        for(int i=0; i<1024*10*200; i++) {
            byte[] bts = wsgId.getBytes(StandardCharsets.UTF_8);
            wsgId = new String(bts, StandardCharsets.UTF_8);
        }
        long end = System.currentTimeMillis();
        System.out.println("string without cache used "  + (end -start) + "ms");

        start = System.currentTimeMillis();
        for(int i=0; i<1024*10*200; i++) {
            byte[] bts = SmallStringCache.utf8Bytes(wsgId);
            wsgId = SmallStringCache.stringFromUtf8(bts);
        }
        end = System.currentTimeMillis();
        System.out.println("string with cache used "  + (end -start) + "ms");

    }
}
