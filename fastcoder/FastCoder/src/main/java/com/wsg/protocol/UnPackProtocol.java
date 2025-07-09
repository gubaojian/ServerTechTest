package com.wsg.protocol;

import com.alibaba.fastjson2.JSON;
import com.wsg.protocol.binary.BinaryView;
import com.wsg.protocol.binary.Input;
import com.wsg.protocol.binary.StringExt;

import java.util.Base64;
import java.util.LinkedHashMap;
import java.util.Map;

public class UnPackProtocol {


    public Map<String, Object> jsonUnPack(String message) {
        Map<String, Object> map = JSON.parseObject(message);
        String msg = (String) map.get(ProtocolConstants.MSG);
        if (ProtocolConstants.ACTION_BINARY_MSG.equals(map.get(ProtocolConstants.ACTION))){
            map.put(ProtocolConstants.MSG, Base64.getDecoder().decode(msg));
        }
        return map;
    }

    //auto unpack data
    public Map<String, Object> unPack(byte[] bts) {
        if (bts[0] == 'b') { //binary
            return binaryUnpack(bts);
        } else {
            return jsonUnPack(bts);
        }
    }

    public Map<String, Object> jsonUnPack(byte[] bts) {
        Map<String, Object> map = JSON.parseObject(bts);
        String msg = (String) map.get(ProtocolConstants.MSG);
        if (ProtocolConstants.ACTION_BINARY_MSG.equals(map.get(ProtocolConstants.ACTION))){
            map.put(ProtocolConstants.MSG, Base64.getDecoder().decode(msg));
        }
        return map;
    }

    public Map<String, Object> binaryUnpack(byte[] bts) {
        Map<String, Object> map = new LinkedHashMap<>();
        if (bts[0] != 'b') {
            throw new IllegalArgumentException("binary protocol not right");
        }
        //unpack skip version check
        byte version = bts[1];
        Input input = new Input(bts, 2);
        map.put(ProtocolConstants.WSG_ID, StringExt.readString(input));
        map.put(ProtocolConstants.CONN_ID, StringExt.readString(input));
        String action = StringExt.readString(input);
        map.put(ProtocolConstants.ACTION, action);
        if (ProtocolConstants.ACTION_BINARY_MSG.equals(action)) {
            map.put(ProtocolConstants.MSG, input.readBinary().toBytes());
        } else {
            map.put(ProtocolConstants.MSG, StringExt.readString(input));
        }
        return map;
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
                    map.put(ProtocolConstants.WSG_ID, StringExt.readString(input));
                    break;
                case 'c' :
                    map.put(ProtocolConstants.CONN_ID, StringExt.readString(input));
                    break;
                case 'a' :
                    {
                         String v = StringExt.readString(input);
                         //针对text和binary msg特殊优化。
                         if ("t".equals(v)) {
                            v = ProtocolConstants.ACTION_TEXT_MSG;
                         } else if ("b".equals(v)) {
                            v = ProtocolConstants.ACTION_BINARY_MSG;
                         }
                         action = v;
                         map.put(ProtocolConstants.ACTION, action);
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
        if (ProtocolConstants.ACTION_BINARY_MSG.equals(action)) {
            if (msg != null) {
                map.put(ProtocolConstants.MSG, msg.toBytes());
            }
        } else {
            if (msg != null) {
                map.put(ProtocolConstants.MSG, msg.toStringUtf8());
            }
        }
        return map;
    }

}
