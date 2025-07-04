package com.wsg.protocol.binary;

import java.nio.charset.StandardCharsets;

public class Input {
    private int position;
    private final byte[] buffer;

    public Input(byte[] buffer, int start) {
        this.buffer = buffer;
        this.position = start;
    }

    public byte[] getBuffer() {
        return buffer;
    }

    public int getPosition() {
        return position;
    }

    public void setPosition(int pos) {
        position = pos;
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

    public final void skip(int length) {
        position += length;
    }

    public final BinaryView readTinyBinary() {
        int  length = readByte();
        BinaryView view = new BinaryView(buffer, position, length);
        position += length;
        return view;
    }

    public final BinaryView readLargeBinary() {
        int  length = readInt();
        BinaryView view = new BinaryView(buffer, position, length);
        position += length;
        return view;
    }

    public final boolean hasNext() {
        return position < buffer.length;
    }
}
