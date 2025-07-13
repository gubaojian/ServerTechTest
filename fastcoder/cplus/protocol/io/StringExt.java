package org.wsg.admin.manager.inner.protocol.io;

import java.nio.charset.StandardCharsets;

public class StringExt {
    public static void writeString(Output output, String str) {
        byte[] bts = str.getBytes(StandardCharsets.UTF_8);
        output.writeBinary(bts);
    }


    public static String readString(Input input) {
        return input.readBinary().toStringUtf8();
    }

}
