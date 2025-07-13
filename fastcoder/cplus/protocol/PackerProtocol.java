package org.wsg.admin.manager.inner.protocol;

import com.alibaba.fastjson2.JSON;
import org.wsg.admin.manager.ProtocolConst;
import org.wsg.admin.manager.inner.protocol.io.Output;
import org.wsg.admin.manager.inner.protocol.io.StringExt;

import java.nio.charset.StandardCharsets;
import java.util.Base64;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.function.Supplier;

public class PackerProtocol {

    public static boolean enableBinaryKV = true;

    public byte[] packText(String wsgId, String connId, String message) {
        if (enableBinaryKV) {
            return binaryKVPackText(wsgId,connId, message);
        }
        return packJsonText(wsgId,connId, message);
    }

    public byte[] packText(String wsgId, String connId, byte[] utf8) {
        if (enableBinaryKV) {
            return binaryKVPackText(wsgId,connId, utf8);
        }
        return packJsonText(wsgId,connId, new String(utf8, StandardCharsets.UTF_8));
    }

    public byte[] packBinary(String wsgId, String connId, byte[] message) {
        if (enableBinaryKV) {
            return binaryKVPackBinary(wsgId, connId, message);
        }
        return packJsonBinary(wsgId, connId, message);
    }

    public byte[] packJsonClose(String wsgId, String connId, String reason) {
        //wsgId放到前面，提升网关解析转发效率和可读性。
        Map<String,String> source = new LinkedHashMap<>();
        source.put(ProtocolConst.WSGID, wsgId);
        source.put(ProtocolConst.CONNID, connId);
        source.put(ProtocolConst.ACTION, ProtocolConst.ACTION_CLOSE);
        source.put(ProtocolConst.MSG, reason);
        checkSource(source);
        String packMessage = JSON.toJSONString(source);
        return  packMessage.getBytes(StandardCharsets.UTF_8);
    }

    public  byte[] binaryKVPackText(String wsgId, String connId, String message) {
        byte[] utf8 = message.getBytes(StandardCharsets.UTF_8);
        return binaryKVPackText(wsgId, connId, utf8);
    }

    public byte[] binaryKVPackText(String wsgId, String connId, byte[] utf8) {
        byte[] buffer = kvLocalBuffer.get();
        Output output = new Output(buffer, 0);
        //协议头部及版本号
        output.writeByte((byte) 'b');
        output.writeByte((byte) 0);
        //按kv顺序写入内容, 注意k不要重复，unpack中增加相应处理方法。
        output.writeByte((byte) 'w');
        StringExt.writeString(output, wsgId);
        output.writeByte((byte) 'c');
        StringExt.writeString(output, connId);
        output.writeByte((byte) 'a');
        StringExt.writeString(output, "t");
        output.writeByte((byte) 'm');
        output.writeBinary(utf8);
        return output.toBytes();
    }


    public byte[] binaryKVPackBinary(String wsgId, String connId, byte[] bts) {
        byte[] buffer = kvLocalBuffer.get();
        Output output = new Output(buffer, 0);
        //协议头部及版本号
        output.writeByte((byte) 'b');
        output.writeByte((byte) 0);
        //按kv顺序写入内容, 注意k不要重复，unpack中增加相应处理方法。
        output.writeByte((byte) 'w');
        StringExt.writeString(output, wsgId);
        output.writeByte((byte) 'c');
        StringExt.writeString(output, connId);
        output.writeByte((byte) 'a');
        StringExt.writeString(output, "b");
        output.writeByte((byte) 'm');
        output.writeBinary(bts);
        return output.toBytes();
    }


    public byte[] packJsonText(String wsgId, String connId, String message) {
        //wsgId放到前面，提升网关解析转发效率和可读性。
        Map<String,String> source = new LinkedHashMap<>();
        source.put(ProtocolConst.WSGID, wsgId);
        source.put(ProtocolConst.CONNID, connId);
        source.put(ProtocolConst.ACTION, ProtocolConst.ACTION_TEXT_MSG);
        source.put(ProtocolConst.MSG, message);
        checkSource(source);
        String packMessage = JSON.toJSONString(source);
        return  packMessage.getBytes(StandardCharsets.UTF_8);
    }

    private byte[] packJsonBinary(String wsgId, String connId, byte[] message) {
        String base64Message = Base64.getEncoder().encodeToString(message);
        //wsgId放到前面，提升网关解析转发效率和可读性。
        Map<String,String> source = new LinkedHashMap<>();
        source.put(ProtocolConst.WSGID, wsgId);
        source.put(ProtocolConst.CONNID, connId);
        source.put(ProtocolConst.ACTION, ProtocolConst.ACTION_TEXT_MSG);
        source.put(ProtocolConst.MSG, base64Message);
        checkSource(source);
        String packMessage = JSON.toJSONString(source);
        return  packMessage.getBytes(StandardCharsets.UTF_8);
    }

    private void checkSource(Map<String,String> source) {
        if (source.get(ProtocolConst.WSGID) == null) {
            throw new IllegalArgumentException("wsgId should be in source map");
        }
        if (source.get(ProtocolConst.CONNID) == null) {
            throw new IllegalArgumentException("connId should be in source map");
        }
        if (source.get(ProtocolConst.ACTION) == null) {
            throw new IllegalArgumentException("action should be in source map");
        }
        if (source.get(ProtocolConst.MSG) == null) {
            throw new IllegalArgumentException("msg should be in source map");
        }
    }

    private static final ThreadLocal<byte[]> kvLocalBuffer = ThreadLocal.withInitial(new Supplier<byte[]>() {
        @Override
        public byte[] get() {
            return new byte[96*1024]; //消息最大64kb
        }
    });

}
