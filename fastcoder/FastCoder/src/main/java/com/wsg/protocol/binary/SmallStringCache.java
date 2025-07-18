package com.wsg.protocol.binary;

import java.nio.charset.StandardCharsets;

/**
 * small string less 128 byte cache to reduce utf8 code and decode utf8 time
 * */
public class SmallStringCache {

    private final String value;
    private final byte[] bts;

    private SmallStringCache(String value) {
        this.value = value;
        this.bts = value.getBytes(StandardCharsets.UTF_8);
    }

    /**
     * cache utf8 byte for small string
     * */
    public static byte[] utf8Bytes(String target) {
        if (target.length() > 128)  {
            return target.getBytes(StandardCharsets.UTF_8);
        }
        int index = SmallStringCache.getTableIndex(target);
        SmallStringCache cacheEntry = stringStable[index];
        if (SmallStringCache.isCacheEntryMiss(cacheEntry, target)) {
            cacheEntry = new SmallStringCache(target);
            stringStable[index] = cacheEntry;
            stringStable[getTableIndex(cacheEntry.bts)] = cacheEntry;
        }
        return cacheEntry.bts;
    }

    public static String stringFromUtf8(byte[] bts) {
        return stringFromUtf8(bts, 0,  bts.length);
    }

    public static String stringFromUtf8(byte[] bts, int offset, int length) {
        if (length > 128) {
            return new String(bts, offset, length, StandardCharsets.UTF_8);
        }
        int index = SmallStringCache.getTableIndex(bts, offset, length);
        SmallStringCache cacheEntry = stringStable[index];
        if (SmallStringCache.isCacheEntryMiss(cacheEntry, bts, offset, length)) {
            String target = new String(bts, offset, length, StandardCharsets.UTF_8);
            cacheEntry = new SmallStringCache(target);
            stringStable[index] = cacheEntry;
            stringStable[getTableIndex(target)] = cacheEntry;
        }
        return cacheEntry.value;
    }

    private static boolean isCacheEntryMiss(SmallStringCache entry, String target) {
        return entry == null || !entry.value.equals(target);
    }

    private static boolean isCacheEntryMiss(SmallStringCache entry, byte[] bts, int offset, int length) {
        if (entry == null || entry.bts.length != length) {
            return true;
        }
        for(int i=0; i<length; i++) {
            if (entry.bts[i] != bts[offset + i]) {
                return true;
            }
        }
        return false;
    }

    private static int getTableIndex(String target) {
        return hashIndex(target.hashCode()) & (tableSize - 1);
    }

    private static int getTableIndex(byte[] bts) {
        return getTableIndex(bts, 0, bts.length);
    }

    private static int getTableIndex(byte[] bts, int offset, int length) {
        return hashIndex(hashCode(bts, offset, length)) & (tableSize - 1);
    }

    private static int hashIndex(int hashCode) {
        return (hashCode ^ (hashCode >>> 16)) & 0x7FFFFFFF; // 确保结果为正数
    }

    //hash code from string
    private static int hashCode(byte[] value, int offset, int length) {
        int h = 0;
        for(int i=offset; i<offset + length; i++) {
            byte v = value[i];
            h = 31 * h + (v & 0xff);
        }
        return h;
    }
    private static final int tableSize = 4096*2; //must be 2 power
    private static final SmallStringCache[] stringStable = new SmallStringCache[tableSize];
}
