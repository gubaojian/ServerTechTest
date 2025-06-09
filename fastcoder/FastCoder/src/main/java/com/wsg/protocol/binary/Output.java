package com.wsg.protocol.binary;

import java.nio.charset.StandardCharsets;

/*
* 暂时不用var int，降低提升代码多端移值难度。
* 针对场景进行精简定制。降低难度。后续可以根据版本号进行扩展
* */
public class Output {
    private byte[] buffer;
    private int position;

    public Output(byte[] buffer, int position) {
        this.buffer = buffer;
        this.position = position;
    }

    public final byte[] toBytes() {
        byte[] bts = new byte[position];
        System.arraycopy(buffer, 0, bts, 0, position);
        return bts;
    }

    public final void writeByte(byte type) {
        buffer[position++] = type;
    }

    public final void writeBytes(byte[] bts) {
        System.arraycopy(bts, 0, buffer, position, bts.length);
        position += bts.length;
    }

    public final void writeShortStringUtf8(String str) {
        byte[] bts = str.getBytes(StandardCharsets.UTF_8);
        if (bts.length > 96) {
            throw new IllegalArgumentException("short string should be less than 96 byte");
        }
        writeByte((byte) bts.length);
        System.arraycopy(bts, 0, buffer, position, bts.length);
        position += bts.length;
    }

    public final void writeLongStringUtf8(String str) {
        byte[] bts = str.getBytes(StandardCharsets.UTF_8);
        writeInt(bts.length);
        System.arraycopy(bts, 0, buffer, position, bts.length);
        position += bts.length;
    }

    public final void writeBinary(byte[] bts) {
        writeInt(bts.length);
        System.arraycopy(bts, 0, buffer, position, bts.length);
        position += bts.length;
    }

    public final void writeInt(int val) {
        buffer[position + 3] = (byte) (val);
        buffer[position + 2] = (byte) (val >>> 8);
        buffer[position + 1] = (byte) (val >>> 16);
        buffer[position] = (byte) (val >>> 24);
        position += 4;
    }
}
