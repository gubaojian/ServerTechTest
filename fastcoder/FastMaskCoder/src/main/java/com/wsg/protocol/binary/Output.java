package com.wsg.protocol.binary;

/*
* 暂时不用var int，降低提升代码多端移值难度。
* bool，string等字符串编码处理独立出去，可以方便字符串扩展，
* 库仅支持短长短二进制和自定义操作，方便移值。
* 针对场景进行精简定制。降低难度。后续可以根据版本号进行扩展
* */
public class Output {
    private final byte[] buffer;
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

    /**
     * max 126 length binary
     * */
    public final void writeTinyBinary(byte[] bts) {
        if (bts.length > 126) {
            throw new IllegalArgumentException("tiny binary should be less than 126 byte");
        }
        writeByte((byte) bts.length);
        System.arraycopy(bts, 0, buffer, position, bts.length);
        position += bts.length;
    }

    /**
     * max 1gb length binary
     * */
    public final void writeLargeBinary(byte[] bts) {
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
