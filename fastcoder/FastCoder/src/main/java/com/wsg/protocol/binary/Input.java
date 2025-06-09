package com.wsg.protocol.binary;

import java.nio.charset.StandardCharsets;

public class Input {
    private int position;
    private byte[] buffer;
    private int start;

    public Input(byte[] buffer, int start) {
        this.buffer = buffer;
        this.position = start;
        this.start = start;
    }

    public final byte readByte() {
        byte type = buffer[position];
        position++;
        return type;
    }

    public final int readInt() {
        int number = (((buffer[position + 3])) +
                ((buffer[position + 2]) << 8) +
                ((buffer[position + 1]) << 16) +
                ((buffer[position]) << 24));
        position += 4;
        return number;
    }

    public final String readShortStringUtf8() {
        int  length = readByte();
        String str = new String(buffer, position, length, StandardCharsets.UTF_8);
        position += length;
        return str;
    }

    public final String readLongStringUtf8() {
        int  length = readInt();
        String str = new String(buffer, position, length, StandardCharsets.UTF_8);
        position += length;
        return str;
    }

    public final byte[] readBinary() {
        int  length = readInt();
        byte[] bts = new byte[length];
        System.arraycopy(buffer, position, bts, 0, length);
        position += length;
        return bts;
    }
}
