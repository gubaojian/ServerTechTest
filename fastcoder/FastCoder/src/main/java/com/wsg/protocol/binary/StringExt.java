package com.wsg.protocol.binary;

import java.nio.charset.StandardCharsets;

public class StringExt {
    public static void writeTinyString(Output output, String str) {
        byte[] bts = str.getBytes(StandardCharsets.UTF_8);
        output.writeTinyBinary(bts);
    }

    public static void writeLargeString(Output output, String str) {
        byte[] bts = str.getBytes(StandardCharsets.UTF_8);
        output.writeLargeBinary(bts);
    }

    public static String readTinyString(Input input) {
        return input.readTinyBinary().toStringUtf8();
    }

    public static String readLargeString(Input input) {
        return input.readLargeBinary().toStringUtf8();
    }

}
