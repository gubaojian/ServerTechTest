package com.efurture.cbor;

import co.nstant.in.cbor.CborBuilder;
import co.nstant.in.cbor.CborEncoder;
import co.nstant.in.cbor.CborException;
import com.alibaba.fastjson.JSON;
import org.junit.Test;

import java.io.ByteArrayOutputStream;
import java.util.HashMap;
import java.util.Map;
import java.util.UUID;

public class CborTest {

    @Test
    public void testCbor() throws CborException {
            ByteArrayOutputStream baos = new ByteArrayOutputStream();
            new CborEncoder(baos).encode(new CborBuilder()
                    .addMap().put("id", System.currentTimeMillis())
                    .put("uuid", UUID.randomUUID().toString())
                    .put("owner", UUID.randomUUID().toString())
                    .put("preMessages", UUID.randomUUID().toString())
                    .put("nextMessages", UUID.randomUUID().toString())
                    .end()
                    .build());
            byte[] encodedBytes = baos.toByteArray();
            System.out.println("length " +encodedBytes.length);

            Map<String, Object> map = new HashMap<String, Object>();
            map.put("id", System.currentTimeMillis());
            map.put("uuid", UUID.randomUUID().toString());
            map.put("owner", UUID.randomUUID().toString());
            map.put("preMessages", UUID.randomUUID().toString());
            map.put("nextMessages", UUID.randomUUID().toString());
            System.out.println("length " + JSON.toJSONString(map).getBytes().length);
    }



    @Test
    public void testCborPerformance() throws CborException {
        long start = System.currentTimeMillis();
        for(int i=0; i<10000; i++) {
            ByteArrayOutputStream baos = new ByteArrayOutputStream();
            new CborEncoder(baos).encode(new CborBuilder()
                    .addMap().put("id", System.currentTimeMillis())
                    .put("uuid", UUID.randomUUID().toString())
                    .put("owner", UUID.randomUUID().toString())
                    .put("preMessages", UUID.randomUUID().toString())
                    .put("nextMessages", UUID.randomUUID().toString())
                    .end()
                    .build());
            byte[] encodedBytes = baos.toByteArray();
        }
        System.out.println("used " + (System.currentTimeMillis() - start));
        start = System.currentTimeMillis();
        for(int i=0; i<10000; i++) {
            Map<String, Object> map = new HashMap<String, Object>();
            map.put("id", System.currentTimeMillis());
            map.put("uuid", UUID.randomUUID().toString());
            map.put("owner", UUID.randomUUID().toString());
            map.put("preMessages", UUID.randomUUID().toString());
            map.put("nextMessages", UUID.randomUUID().toString());
            JSON.toJSONString(map).getBytes();
        }

        System.out.println("used " + (System.currentTimeMillis() - start));

        //System.out.println(JSON.toJSONString(map).getBytes().length);

        start = System.currentTimeMillis();
        for(int i=0; i<10000; i++) {
            ByteArrayOutputStream baos = new ByteArrayOutputStream();
            new CborEncoder(baos).encode(new CborBuilder()
                    .addMap().put("id", System.currentTimeMillis())
                    .put("uuid", UUID.randomUUID().toString())
                    .put("owner", UUID.randomUUID().toString())
                    .put("preMessages", UUID.randomUUID().toString())
                    .put("nextMessages", UUID.randomUUID().toString())
                    .end()
                    .build());
            byte[] encodedBytes = baos.toByteArray();
        }
        System.out.println("used " + (System.currentTimeMillis() - start));

        start = System.currentTimeMillis();
        for(int i=0; i<10000; i++) {
            Map<String, Object> map = new HashMap<String, Object>();
            map.put("id", System.currentTimeMillis());
            map.put("uuid", UUID.randomUUID().toString());
            map.put("owner", UUID.randomUUID().toString());
            map.put("preMessages", UUID.randomUUID().toString());
            map.put("nextMessages", UUID.randomUUID().toString());
            JSON.toJSONString(map).getBytes();
        }

        System.out.println("used " + (System.currentTimeMillis() - start));

    }
}
