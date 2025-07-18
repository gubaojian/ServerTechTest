package com.wsg.protocol;

public class ThreadLocalSharedBuffer {
    private final byte[] sharedBuffer;
    private final int offset;
    private final int length;

    public ThreadLocalSharedBuffer(byte[] buffer, int offset, int length) {
        this.sharedBuffer = buffer;
        this.offset = offset;
        this.length = length;
    }

    public byte[] getSharedBufferView() {
        return sharedBuffer;
    }

    public int getOffset() {
        return offset;
    }

    public int getLength() {
        return length;
    }
}
