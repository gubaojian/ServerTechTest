package com.efurture;

import com.efurture.uri.URIProcessor;

import javax.crypto.Mac;
import javax.crypto.spec.SecretKeySpec;
import java.net.URI;
import java.nio.charset.StandardCharsets;
import java.util.*;

public class UrlHMacSignUtil {


    public static URI signUrl(String uri, String appSecret) {
        URIProcessor processor =  URIProcessor.fromURL(uri);
        Map<String, List<String>> queryMap = processor.getQueryMap();
        if (appSecret == null) {
            throw new IllegalArgumentException("appSecret should not be null");
        }
        byte[] secret = appSecret.getBytes(StandardCharsets.UTF_8);
        if (secret.length < 32) {
            throw new IllegalArgumentException("appSecret should be 32 byte");
        }

        if (processor.getQueryParameter("port") == null) {
            throw new IllegalArgumentException("uri should be contains port");
        }

        TreeMap<String, String> signParameters = new TreeMap<>();
        Set<Map.Entry<String, List<String>>> entries = queryMap.entrySet();
        for(Map.Entry<String, List<String>> entry : entries) {
            List<String> values = entry.getValue();
            if (values != null && values.size() > 1) {
                throw new IllegalArgumentException("url one only can't contains two same parameter, key contains two values " + entry.getKey());
            }
            String value = "";
            if (values != null && values.size() == 1) {
                value = values.get(0);
            }
            signParameters.put(entry.getKey(), value);
        }
        signParameters.put("signTime", String.valueOf(System.currentTimeMillis()));
        signParameters.put("salt", UUID.randomUUID().toString());

        Set<Map.Entry<String, String>> signEntries = signParameters.entrySet();
        StringBuilder sb = new StringBuilder();
        for(Map.Entry<String, String> entry : signEntries) {
            sb.append(entry.getKey());
            sb.append("=");
            sb.append(entry.getValue());
            sb.append("&");
        }
        sb.deleteCharAt(sb.length()-1);
        String data = sb.toString();
        try {
            Mac mac = Mac.getInstance("HmacSHA256");
            SecretKeySpec secretKey = new SecretKeySpec(secret, "HmacSHA256");
            mac.init(secretKey);
            byte[] bts = mac.doFinal(data.getBytes(StandardCharsets.UTF_8)); // 仅计算 HMAC-SHA256
            jdkx.compat.util.HexFormat hexFormat = jdkx.compat.util.HexFormat.of().withLowerCase();
            String sign = hexFormat.formatHex(bts);
            signParameters.put("sign", sign);
            processor.clearAllParameters();
            processor.putAllParameter(signParameters);
            return processor.toURI();
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
    }

    public static boolean checkSignUrl(String uri, String appSecret) {
        URIProcessor processor =  URIProcessor.fromURL(uri);
        Map<String, List<String>> queryMap = processor.getQueryMap();
        String expect_sign = processor.getQueryParameter("sign");
        if (expect_sign == null) {
            throw new IllegalArgumentException("parameter sign should not be null");
        }
        if (appSecret == null) {
            throw new IllegalArgumentException("appSecret should not be null");
        }
        byte[] secret = appSecret.getBytes(StandardCharsets.UTF_8);
        if (secret.length < 32) {
            throw new IllegalArgumentException("appSecret should be 32 byte");
        }
        if (processor.getQueryParameter("port") == null) {
            throw new IllegalArgumentException("uri should be contains port parameter");
        }
        processor.removeQueryParameter("sign");
        String signTime = processor.getQueryParameter("signTime");
        long time = Long.parseLong(signTime);
        if (Math.abs(System.currentTimeMillis() - time) > 1000*60) {
            throw new IllegalArgumentException("signTime is expired " + uri);
        }
        TreeMap<String, String> signParameters = new TreeMap<>();
        Set<Map.Entry<String, List<String>>> entries = queryMap.entrySet();
        for(Map.Entry<String, List<String>> entry : entries) {
            List<String> values = entry.getValue();
            if (values != null && values.size() > 1) {
                throw new IllegalArgumentException("url one only can't contains two same parameter, key contains two values " + entry.getKey());
            }
            String value = "";
            if (values != null && values.size() == 1) {
                value = values.get(0);
            }
            signParameters.put(entry.getKey(), value);
        }


        Set<Map.Entry<String, String>> signEntries = signParameters.entrySet();
        StringBuilder sb = new StringBuilder();
        for(Map.Entry<String, String> entry : signEntries) {
            sb.append(entry.getKey());
            sb.append("=");
            sb.append(entry.getValue());
            sb.append("&");
        }
        sb.deleteCharAt(sb.length()-1);
        String data = sb.toString();
        try {
            Mac mac = Mac.getInstance("HmacSHA256");
            SecretKeySpec secretKey = new SecretKeySpec(secret, "HmacSHA256");
            mac.init(secretKey);
            byte[] bts = mac.doFinal(data.getBytes(StandardCharsets.UTF_8)); // 仅计算 HMAC-SHA256
            jdkx.compat.util.HexFormat hexFormat = jdkx.compat.util.HexFormat.of().withLowerCase();
            String sign = hexFormat.formatHex(bts);
            return expect_sign.equals(sign);
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
    }

    public static void main(String[] args) {
        String appSecret = "dssssssssssssssssssss1242142141241242142444444";
        String uri = "http://127.0.0.1:8001?appId=333&role=client&port=8001";
        URI signUri = signUrl(uri, appSecret);
        System.out.println(uri);
        System.out.println(signUri);
        System.out.println(checkSignUrl(signUri.toString(), appSecret));
        System.out.println("after alert uri");
        System.out.println(checkSignUrl(URIProcessor.fromURI(signUri).appendQueryParameter("h", "eee").toURI().toString(), appSecret));

    }
}
