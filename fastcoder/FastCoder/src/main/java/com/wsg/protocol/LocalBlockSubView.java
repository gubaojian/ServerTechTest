package com.wsg.protocol;

public class LocalBlockSubView {
    private final byte[] block;
    private final int offset;
    private final int length;

    public LocalBlockSubView(byte[] block, int offset, int length) {
        this.block = block;
        this.offset = offset;
        this.length = length;
    }

    public byte[] getBlock() {
        return block;
    }

    public int getOffset() {
        return offset;
    }

    public int getLength() {
        return length;
    }
}
