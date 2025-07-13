package org.wsg.admin.manager.inner.protocol;

import com.alibaba.fastjson2.JSON;
import org.wsg.admin.manager.ProtocolConst;
import org.wsg.admin.manager.inner.protocol.io.BinaryView;
import org.wsg.admin.manager.inner.protocol.io.Input;
import org.wsg.admin.manager.inner.protocol.io.StringExt;

import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;
import java.util.Base64;
import java.util.LinkedHashMap;
import java.util.Map;

public class UnPackerProtocol {

    public Map<String, Object> jsonUnPack(String packMessage) {
        Map<String, Object> source = JSON.parseObject(packMessage);
        String action = (String)source.get(ProtocolConst.ACTION);
        String message = (String) source.get(ProtocolConst.MSG);
        if (ProtocolConst.ACTION_BINARY_MSG.equals(action)) {
            byte[] binaryMessage = Base64.getDecoder().decode(message.getBytes(StandardCharsets.UTF_8));
            source.put(ProtocolConst.MSG, binaryMessage);
        }
        return source;
    }

    public Map<String, Object> autoUnPack(ByteBuffer buffer) {
        byte[] bts = buffer.array();
        if (bts[0] == 'b') { //binary
            return binaryKVUnpack(bts);
        } else {
            return jsonUnPack(bts);
        }
    }


    public Map<String, Object> jsonUnPack(byte[] bts) {
        Map<String, Object> source = JSON.parseObject(bts);
        String action = (String)source.get(ProtocolConst.ACTION);
        String message = (String) source.get(ProtocolConst.MSG);
        if (ProtocolConst.ACTION_BINARY_MSG.equals(action)) {
            byte[] binaryMessage = Base64.getDecoder().decode(message.getBytes(StandardCharsets.UTF_8));
            source.put(ProtocolConst.MSG, binaryMessage);
        }
        return source;
    }

    public Map<String, Object> binaryKVUnpack(byte[] bts) {
        Map<String, Object> map = new LinkedHashMap<>();
        if (bts[0] != 'b') {
            throw new IllegalArgumentException("binary protocol not right");
        }
        //unpack skip version check
        byte version = bts[1];
        Input input = new Input(bts, 2);
        BinaryView msg = null;
        String action = null;
        while (input.hasNext()) {
            byte k = input.readByte();
            switch (k) {
                case 'w' :
                    map.put(ProtocolConst.WSGID, StringExt.readString(input));
                    break;
                case 'c' :
                    map.put(ProtocolConst.CONNID, StringExt.readString(input));
                    break;
                case 'a' :
                {
                    String v = StringExt.readString(input);
                    //针对text和binary msg特殊优化。
                    if ("t".equals(v)) {
                        v = ProtocolConst.ACTION_TEXT_MSG;
                    } else if ("b".equals(v)) {
                        v =  ProtocolConst.ACTION_BINARY_MSG;
                    }
                    action = v;
                    map.put(ProtocolConst.ACTION, action);
                }
                break;
                case 'm' :
                    msg = input.readBinary();
                    break;
                default:
                    input.skipBinary();
                    break;
            }
        }
        if (ProtocolConst.ACTION_BINARY_MSG.equals(action)) {
            if (msg != null) {
                map.put(ProtocolConst.MSG, msg.toBytes());
            }
        } else {
            if (msg != null) {
                map.put(ProtocolConst.MSG, msg.toStringUtf8());
            }
        }
        return map;
    }

}
