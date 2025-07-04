package com.wsg.protocol;

import com.alibaba.fastjson2.JSON;
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
        if (bts[1] != 0) {
            throw new IllegalArgumentException("binary protocol version not right");
        }
        Input input = new Input(bts, 2);
        map.put(ProtocolConstants.WSG_ID, StringExt.readTinyString(input));
        map.put(ProtocolConstants.CONN_ID, StringExt.readTinyString(input));
        String action = StringExt.readTinyString(input);
        map.put(ProtocolConstants.ACTION, action);
        if (ProtocolConstants.ACTION_BINARY_MSG.equals(action)) {
            map.put(ProtocolConstants.MSG, input.readLargeBinary().toBytes());
        } else {
            map.put(ProtocolConstants.MSG, StringExt.readLargeString(input));
        }
        return map;
    }
}
