package com.wsg.protocol.binary;

import java.nio.charset.StandardCharsets;

public final class BinaryView {
    private final byte[] buffer;
    private final int position;
    private final int length;
    private String stringCache;
    private byte[] bytesCache;

    public BinaryView(byte[] buffer, int position, int length) {
        this.buffer = buffer;
        this.position = position;
        this.length = length;
    }

    public String toStringUtf8() {
        if (stringCache == null) {
            stringCache = new String(buffer, position, length, StandardCharsets.UTF_8);
        }
        return stringCache;
    }

    public byte[] toBytes() {
        if (bytesCache == null) {
            byte[] bts = new byte[length];
            System.arraycopy(buffer, position, bts, 0, length);
            bytesCache = bts;
        }
        return bytesCache;
    }

    public int getLength() {
        return length;
    }

    public int getPosition() {
        return position;
    }

    public byte[] getBuffer() {
        return buffer;
    }
}
