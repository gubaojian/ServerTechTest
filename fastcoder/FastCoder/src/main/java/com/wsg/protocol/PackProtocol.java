package com.wsg.protocol;

import com.alibaba.fastjson2.JSON;
import com.wsg.protocol.binary.Output;
import com.wsg.protocol.binary.StringExt;

import java.nio.charset.StandardCharsets;
import java.util.Base64;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.function.Supplier;

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
        byte[] buffer = localBuffer.get();
        Output output = new Output(buffer, 0);
        //协议头部及版本号
        output.writeByte((byte) 'b');
        output.writeByte((byte) 0);
        //按顺序写入内容
        StringExt.writeTinyString(output, wsgId);
        StringExt.writeTinyString(output, connId);
        StringExt.writeTinyString(output, ProtocolConstants.ACTION_TEXT_MSG);
        StringExt.writeLargeString(output, message);
        return output.toBytes();
    }

    public byte[] binaryPackBinary(String wsgId, String connId, byte[] message) {
        byte[] buffer = localBuffer.get();
        Output output = new Output(buffer, 0);
        //协议头部及版本号
        output.writeByte((byte) 'b');
        output.writeByte((byte) 0);
        //按顺序写入内容
        StringExt.writeTinyString(output, wsgId);
        StringExt.writeTinyString(output, connId);
        StringExt.writeTinyString(output, ProtocolConstants.ACTION_BINARY_MSG);
        output.writeLargeBinary(message);
        return output.toBytes();
    }


    private static final ThreadLocal<byte[]> localBuffer = ThreadLocal.withInitial(new Supplier<byte[]>() {
        @Override
        public byte[] get() {
            return new byte[96*1024]; //消息最大64kb
        }
    });

}
