package com.wsg.protocol.binary;

import java.nio.charset.StandardCharsets;
import java.util.HashMap;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.atomic.AtomicInteger;

public class StringExt {
    public static void writeString(Output output, String str) {
        byte[] bts = str.getBytes(StandardCharsets.UTF_8);
        output.writeBinary(bts);
    }

    public static void writeStringWithCachePool(Output output, String str) {
        byte[] bts = fastStringCachePool.computeIfAbsent(str, s -> s.getBytes(StandardCharsets.UTF_8));
        output.writeBinary(bts);
        int overLimit = fastStringCachePool.size() - 4096*2; //wsgId max number most is bellow 1000
        if (overLimit > 0) {
            AtomicInteger remaining = new AtomicInteger(4096 + overLimit);
            fastStringCachePool.keySet().removeIf(key -> remaining.decrementAndGet() >= 0);
        }
    }
    private static final Map<String, byte[]> fastStringCachePool = new ConcurrentHashMap<>();


    public static String readString(Input input) {
        return input.readBinary().toStringUtf8();
    }



}
