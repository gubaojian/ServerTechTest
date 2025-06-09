package com.wsg.protocol;

import java.nio.ByteBuffer;
import java.util.Map;

public interface Protocol {
    public byte[] packText(String wsgId, String connId, String message);
    public byte[] packBinary(String wsgId, String connId, byte[] message);

    public Map<String,Object> unpackText(String message);
    public Map<String,Object> unpackBinary(byte[] bts);

}
