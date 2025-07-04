package com.wsg.protocol.binary;

import java.nio.charset.StandardCharsets;

public final class BinaryView {
    private final byte[] buffer;
    private final int position;
    private final int length;

    public BinaryView(byte[] buffer, int position, int length) {
        this.buffer = buffer;
        this.position = position;
        this.length = length;
    }

    public String toStringUtf8() {
        return new String(buffer, position, length, StandardCharsets.UTF_8);
    }

    public byte[] toBytes() {
        byte[] bts = new byte[length];
        System.arraycopy(buffer, position, bts, 0, length);
        return bts;
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
