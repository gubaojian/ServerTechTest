package com.wsg.protocol.binary;

/*
 * 暂时不用var int，降低提升代码多端移值难度。
 * bool，string等字符串编码处理独立出去，可以方便字符串扩展，
 * 库仅支持短长短二进制和自定义操作，方便移值。
 * 针对场景进行精简定制。降低难度。后续可以根据版本号进行扩展
 * */
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

    public int readVarInt() {
        if (buffer[position] >= 0) {
            return buffer[position++];
        } else if (buffer[position + 1] >= 0) {
            return (buffer[position++] & 0x7F)
                    | ((buffer[position++] & 0x7F) << 7);
        } else if (buffer[position + 2] >= 0) {
            return (buffer[position++] & 0x7F)
                    | ((buffer[position++] & 0x7F) << 7)
                    | ((buffer[position++] & 0x7F) << 14);
        } else if (buffer[position + 3] >= 0) {
            return (buffer[position++] & 0x7F)
                    | ((buffer[position++] & 0x7F) << 7)
                    | ((buffer[position++] & 0x7F) << 14)
                    | ((buffer[position++] & 0x7F) << 21);
        } else if (buffer[position + 4] >= 0) {
            if ((buffer[position + 4] & 0x7F) > 0x0F) {
                throw new IllegalArgumentException("VarInt exceeds 32-bit maximum value");
            }
            return (buffer[position++] & 0x7F)
                    | ((buffer[position++] & 0x7F) << 7)
                    | ((buffer[position++] & 0x7F) << 14)
                    | ((buffer[position++] & 0x7F) << 21)
                    | ((buffer[position++] & 0x7F) << 28);
        }
        throw new IllegalArgumentException("VarInt too big (exceeds 5 bytes)");
    }

    public final BinaryView readBinary() {
        int  length = readVarInt();
        BinaryView view = new BinaryView(buffer, position, length);
        position += length;
        return view;
    }

    public final void skipBinary() {
        int  length = readVarInt();
        position += length;
    }

    public final boolean hasNext() {
        return position < buffer.length;
    }
}
