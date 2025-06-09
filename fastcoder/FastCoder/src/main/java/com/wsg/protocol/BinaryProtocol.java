package com.wsg.protocol;

import com.wsg.protocol.binary.Input;
import com.wsg.protocol.binary.Output;

import java.nio.charset.StandardCharsets;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.function.Supplier;

public class BinaryProtocol implements Protocol {

    private static final ThreadLocal<byte[]> localBuffer = ThreadLocal.withInitial(new Supplier<byte[]>() {
        @Override
        public byte[] get() {
            return new byte[96*1024]; //消息最大64kb
        }
    });

    @Override
    public byte[] packText(String wsgId, String connId, String message) {
        //wsgId放到最前面, msg放最后，增加可读性，提升解析性能，方便解析。
        // 比如wsgbus直接start_with然后根据长度进行截取,可以避免json解析。
        byte[] buffer = localBuffer.get();
        Output output = new Output(buffer, 0);
        //协议头部及版本号
        output.writeByte((byte) 'b');
        output.writeByte((byte) 0);
        //按顺序写入内容
        output.writeShortStringUtf8(wsgId);
        output.writeShortStringUtf8(connId);
        output.writeShortStringUtf8(ProtocolConstants.ACTION_TEXT_MSG);
        output.writeLongStringUtf8(message);
        return output.toBytes();
    }

    @Override
    public byte[] packBinary(String wsgId, String connId, byte[] message) {
        byte[] buffer = localBuffer.get();
        Output output = new Output(buffer, 0);
        //协议头部及版本号
        output.writeByte((byte) 'b');
        output.writeByte((byte) 0);
        //按顺序写入内容
        output.writeShortStringUtf8(wsgId);
        output.writeShortStringUtf8(connId);
        output.writeShortStringUtf8(ProtocolConstants.ACTION_BINARY_MSG);
        output.writeBinary(message);
        return output.toBytes();
    }

    @Override
    public Map<String, Object> unpackText(String message) {
          return unpackBinary(message.getBytes(StandardCharsets.UTF_8));
    }

    @Override
    public Map<String, Object> unpackBinary(byte[] bts) {
        Map<String, Object> map = new LinkedHashMap<>();
        if (bts[0] != 'b') {
            throw new IllegalArgumentException("binary protocol not right");
        }
        if (bts[1] != 0) {
            throw new IllegalArgumentException("binary protocol version not right");
        }
        Input input = new Input(bts, 2);
        map.put(ProtocolConstants.WSG_ID, input.readShortStringUtf8());
        map.put(ProtocolConstants.CONN_ID, input.readShortStringUtf8());
        String action = input.readShortStringUtf8();
        map.put(ProtocolConstants.ACTION, action);
        if (ProtocolConstants.ACTION_BINARY_MSG.equals(action)) {
            map.put(ProtocolConstants.MSG, input.readBinary());
        } else {
            map.put(ProtocolConstants.MSG, input.readLongStringUtf8());
        }
        return map;
    }


}
