package org.example;

import org.openjdk.jmh.annotations.*;
import org.openjdk.jmh.infra.Blackhole;

import java.nio.ByteBuffer;
import java.util.concurrent.TimeUnit;


@BenchmarkMode(Mode.AverageTime)
@OutputTimeUnit(TimeUnit.NANOSECONDS)
@Warmup(iterations = 5, time = 1, timeUnit = TimeUnit.SECONDS)
@Measurement(iterations = 10, time = 1, timeUnit = TimeUnit.SECONDS)
@Fork(1)
@State(Scope.Thread)
public class VarIntTest {

    // 测试数据：1字节、2字节、3字节、4字节、5字节的Varint
    private static final byte[] VARINT_1B = new byte[]{0x7F}; // 127
    private static final byte[] VARINT_2B = new byte[]{(byte) 0x80, 0x01}; // 128
    private static final byte[] VARINT_3B = new byte[]{(byte) 0x80, (byte) 0x80, 0x01}; // 16384
    private static final byte[] VARINT_4B = new byte[]{(byte) 0x80, (byte) 0x80, (byte) 0x80, 0x01}; // 2097152
    private static final byte[] VARINT_5B = new byte[]{(byte) 0x80, (byte) 0x80, (byte) 0x80, (byte) 0x80, 0x01}; // 268435456

    private byte[] buffer;
    private int position;
    private ByteBuffer byteBuffer;

    @Setup
    public void setup() {
        // 选择测试数据（可修改此处测试不同长度）
        buffer = VARINT_1B.clone();
        byteBuffer = ByteBuffer.wrap(buffer);

        // 为多线程测试重置位置
        position = 0;
    }

    // 1. 原始循环实现
    @Benchmark
    public void baseline(Blackhole bh) {
        position = 0;
        bh.consume(baselineReadVarInt());
    }

    private int baselineReadVarInt() {
        int result = 0;
        int shift = 0;
        while (true) {
            byte b = buffer[position++];
            result |= (b & 0x7F) << shift;
            if (b >= 0) break;
            shift += 7;
        }
        return result;
    }

    // 2. 短路条件判断（用户当前实现）
    @Benchmark
    public void ifElseImplementation(Blackhole bh) {
        position = 0;
        bh.consume(ifElseReadVarInt());
    }

    private int ifElseReadVarInt() {
        if (buffer[position] >= 0) {
            return buffer[position++];
        } else if (buffer[position + 1] >= 0) {
            return (buffer[position++] & 0x7F) | ((buffer[position++] & 0x7F) << 7);
        } else if (buffer[position + 2] >= 0) {
            return (buffer[position++] & 0x7F) | ((buffer[position++] & 0x7F) << 7) | ((buffer[position++] & 0x7F) << 14);
        } else if (buffer[position + 3] >= 0) {
            return (buffer[position++] & 0x7F) | ((buffer[position++] & 0x7F) << 7) | ((buffer[position++] & 0x7F) << 14) | ((buffer[position++] & 0x7F) << 21);
        } else if (buffer[position + 4] >= 0) {
            if ((buffer[position + 4] & 0x7F) > 0x0F) {
                throw new IllegalArgumentException("VarInt exceeds 32-bit maximum value");
            }
            return (buffer[position++] & 0x7F) | ((buffer[position++] & 0x7F) << 7) | ((buffer[position++] & 0x7F) << 14) | ((buffer[position++] & 0x7F) << 21) | ((buffer[position++] & 0x7F) << 28);
        }
        throw new IllegalArgumentException("VarInt too big (exceeds 5 bytes)");
    }

    // 3. 循环展开优化
    @Benchmark
    public void unrolledLoopImplementation(Blackhole bh) {
        position = 0;
        bh.consume(unrolledLoopReadVarInt());
    }

    private int unrolledLoopReadVarInt() {
        byte b1 = buffer[position++];
        int result = b1 & 0x7F;
        if (b1 >= 0) return result;

        byte b2 = buffer[position++];
        result |= (b2 & 0x7F) << 7;
        if (b2 >= 0) return result;

        byte b3 = buffer[position++];
        result |= (b3 & 0x7F) << 14;
        if (b3 >= 0) return result;

        byte b4 = buffer[position++];
        result |= (b4 & 0x7F) << 21;
        if (b4 >= 0) return result;

        byte b5 = buffer[position++];
        if ((b5 & 0x7F) > 0x0F) {
            throw new IllegalArgumentException("VarInt exceeds 32-bit maximum value");
        }
        result |= (b5 & 0x7F) << 28;
        return result;
    }

    // 4. 批量读取优化
    @Benchmark
    public void bulkReadImplementation(Blackhole bh) {
        byteBuffer.position(0);
        bh.consume(bulkReadVarInt());
    }

    private int bulkReadVarInt() {
        if (byteBuffer.get(0) >= 0) {
            return byteBuffer.get();
        }

        int result = 0;
        int shift = 0;
        while (true) {
            byte b = byteBuffer.get();
            result |= (b & 0x7F) << shift;
            if (b >= 0) break;
            shift += 7;
        }
        return result;
    }

    public static void main(String[] args) throws Exception {
        org.openjdk.jmh.Main.main(args);
    }
}
