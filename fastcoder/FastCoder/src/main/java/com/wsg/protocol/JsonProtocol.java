package com.wsg.protocol;

import com.alibaba.fastjson2.JSON;

import java.nio.charset.StandardCharsets;
import java.util.Base64;
import java.util.LinkedHashMap;
import java.util.Map;

public class JsonProtocol implements  Protocol {
    @Override
    public byte[] packText(String wsgId, String connId, String message) {
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

    @Override
    public byte[] packBinary(String wsgId, String connId, byte[] message) {
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

    @Override
    public Map<String, Object> unpackText(String message) {
        Map<String, Object> map = JSON.parseObject(message);
        String msg = (String) map.get(ProtocolConstants.MSG);
        if (ProtocolConstants.ACTION_BINARY_MSG.equals(map.get(ProtocolConstants.ACTION))){
              map.put(ProtocolConstants.MSG, Base64.getDecoder().decode(msg));
        }
        return map;
    }

    @Override
    public Map<String, Object> unpackBinary(byte[] bts) {
        Map<String, Object> map = JSON.parseObject(bts);
        String msg = (String) map.get(ProtocolConstants.MSG);
        if (ProtocolConstants.ACTION_BINARY_MSG.equals(map.get(ProtocolConstants.ACTION))){
            map.put(ProtocolConstants.MSG, Base64.getDecoder().decode(msg));
        }
        return map;
    }


}
