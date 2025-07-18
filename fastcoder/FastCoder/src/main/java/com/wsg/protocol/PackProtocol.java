package com.wsg.protocol;

import com.alibaba.fastjson2.JSON;
import com.wsg.protocol.binary.Output;
import com.wsg.protocol.binary.StringExt;

import java.nio.charset.StandardCharsets;
import java.util.Base64;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.function.Supplier;

/**
 * 约定不管是json还是二进制，按约定顺序提高解析效率，添加字段统一放到后面.
 * 这样能提升解析效率，增加兼容性。老版本只是忽略对应的字段
 * */
public class PackProtocol {

    public byte[] jsonPackText(String wsgId, String connId, String message) {
        //wsgId放到最前面, msg放最后，增加可读性，提升解析性能，方便解析。
        // 比如wsgbus直接start_with然后根据长度进行截取,可以避免json解析，提升效率。
        Map<String,String> map = new LinkedHashMap<>();
        map.put(ProtocolConstants.WSG_ID, wsgId);
        map.put(ProtocolConstants.CONN_ID, connId);
        map.put(ProtocolConstants.ACTION, ProtocolConstants.ACTION_TEXT_MSG);
        map.put(ProtocolConstants.MSG, message);
        String json = JSON.toJSONString(map);
        return json.getBytes(StandardCharsets.UTF_8); //check length
    }

    public byte[] jsonPackBinary(String wsgId, String connId, byte[] message) {
        //wsgId放到最前面, msg放最后，增加可读性，提升解析性能，方便解析。
        //比如wsgbus直接start_with然后根据长度进行截取,可以避免json解析，提升效率。
        Map<String,String> map = new LinkedHashMap<>();
        map.put(ProtocolConstants.WSG_ID, wsgId);
        map.put(ProtocolConstants.CONN_ID, connId);
        map.put(ProtocolConstants.ACTION, ProtocolConstants.ACTION_BINARY_MSG);
        String base64Message = Base64.getEncoder().encodeToString(message);
        map.put(ProtocolConstants.MSG, base64Message);
        String json = JSON.toJSONString(map);
        return json.getBytes(StandardCharsets.UTF_8);
    }

    public byte[] binaryPackText(String wsgId, String connId, String message) {
        byte[] buffer = kvLocalBuffer.get();
        Output output = new Output(buffer, 0);
        //协议头部及版本号
        output.writeByte((byte) 'b');
        output.writeByte((byte) 0);
        //按顺序写入内容
        StringExt.writeString(output, wsgId);
        StringExt.writeString(output, connId);
        StringExt.writeString(output, ProtocolConstants.ACTION_TEXT_MSG);
        StringExt.writeString(output, message);
        return output.toBytes();
    }

    public byte[] binaryKVPackText(String wsgId, String connId, String message) {
        byte[] bts = message.getBytes(StandardCharsets.UTF_8);
        return binaryKVPackText(wsgId, connId, message);
    }

    public byte[] binaryKVPackText(String wsgId, String connId, byte[] utf8) {
        byte[] buffer = kvLocalBuffer.get();
        Output output = new Output(buffer, 0);
        //协议头部及版本号
        output.writeByte((byte) 'b');
        output.writeByte((byte) 0);
        //按kv顺序写入内容, 注意k不要重复，unpack中增加相应处理方法。
        output.writeByte((byte) 'w');
        StringExt.writeStringWithCachePool(output, wsgId);
        output.writeByte((byte) 'c');
        StringExt.writeString(output, connId);
        output.writeByte((byte) 'a');
        StringExt.writeString(output, "t");
        output.writeByte((byte) 'm');
        output.writeBinary(utf8);
        return output.toBytes();
    }

    public byte[] binaryPackBinary(String wsgId, String connId, byte[] message) {
        byte[] buffer = kvLocalBuffer.get();
        Output output = new Output(buffer, 0);
        //协议头部及版本号
        output.writeByte((byte) 'b');
        output.writeByte((byte) 0);
        //按顺序写入内容
        StringExt.writeString(output, wsgId);
        StringExt.writeString(output, connId);
        StringExt.writeString(output, ProtocolConstants.ACTION_BINARY_MSG);
        output.writeBinary(message);
        return output.toBytes();
    }

    public ThreadLocalSharedBuffer binaryPackBinaryView(String wsgId, String connId, byte[] message) {
        byte[] buffer = kvLocalBuffer.get();
        Output output = new Output(buffer, 0);
        //协议头部及版本号
        output.writeByte((byte) 'b');
        output.writeByte((byte) 0);
        //按顺序写入内容
        StringExt.writeStringWithCachePool(output, wsgId);
        StringExt.writeString(output, connId);
        StringExt.writeString(output, ProtocolConstants.ACTION_BINARY_MSG);
        output.writeBinary(message);
        return new ThreadLocalSharedBuffer(buffer, 0, output.getPosition());
    }


    private static final ThreadLocal<byte[]> kvLocalBuffer = ThreadLocal.withInitial(new Supplier<byte[]>() {
        @Override
        public byte[] get() {
            return new byte[96*1024]; //消息最大64kb
        }
    });

}
