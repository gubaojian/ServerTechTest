package com.efurture;

import com.efurture.uri.URIProcessor;
import jdkx.compat.util.threadlocal.ThreadLocalUtil;

import javax.crypto.Mac;
import javax.crypto.spec.SecretKeySpec;
import java.net.URI;
import java.nio.charset.StandardCharsets;
import java.util.*;

public class HMacSignUtil {


    public static  String signUrl(String uri, byte[] appSecret) {
        return innerSignUrl(URIProcessor.fromURI(uri), appSecret);
    }

    public static  String signUrl(URI uri, byte[] appSecret) {
        return innerSignUrl(URIProcessor.fromURI(uri), appSecret);
    }

    public static  String signUrl(URIProcessor processor, byte[] appSecret) {
        URIProcessor innerProcessor = URIProcessor.fromProcessor(processor);
        return innerSignUrl(innerProcessor, appSecret);
    }

    /**
     * 对URIProcessor进行签名（内部会拷贝新实例，不修改原processor参数）
     * @param processor 待签名的URIProcessor
     * @param appSecret 签名密钥（≥32字节）
     * @return 签名后的URI
     */
    private static String innerSignUrl(URIProcessor processor, byte[] appSecret) {
        Map<String, List<String>> queryMap = processor.getQueryMap();
        if (appSecret == null) {
            throw new IllegalArgumentException("appSecret should not be null");
        }
        byte[] secret =  appSecret;
        if (secret == null || secret.length < 32) {
            throw new IllegalArgumentException("appSecret should be at least 32 byte");
        }

        if (processor.getQueryParameter("port") == null) {
            throw new IllegalArgumentException("uri should contain port parameter");
        }

        TreeMap<String, String> signParameters = genSignMap(queryMap);
        signParameters.put("signTime", String.valueOf(System.currentTimeMillis()));
        signParameters.put("salt", String.valueOf(Math.abs(ThreadLocalUtil.getSecureRandom().nextLong())));
        signParameters.put("traceId", ThreadLocalUtil.genTraceId());

        Set<Map.Entry<String, String>> signEntries = signParameters.entrySet();
        StringBuilder sb = new StringBuilder(512);
        for(Map.Entry<String, String> entry : signEntries) {
            sb.append(entry.getKey());
            sb.append("=");
            sb.append(entry.getValue());
            sb.append("&");
        }
        sb.deleteCharAt(sb.length()-1);
        String signData = sb.toString();
        try {
            Mac mac = ThreadLocalUtil.getLocalValueEntry().getHmacSHA256();
            SecretKeySpec secretKey = new SecretKeySpec(secret, "HmacSHA256");
            mac.init(secretKey);
            byte[] bts = mac.doFinal(signData.getBytes(StandardCharsets.UTF_8)); // 仅计算 HMAC-SHA256
            jdkx.compat.util.HexFormat hexFormat = jdkx.compat.util.HexFormat.of().withLowerCase();
            String sign = hexFormat.formatHex(bts);
            signParameters.put("sign", sign);
            processor.clearAllParameters();
            processor.putAllParameter(signParameters);
            return processor.fastToUri();
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
    }

    public static boolean checkSignUrl(String uri, byte[] appSecret) {
        try {
            return innerCheckSignUrl(uri, appSecret);
        }catch (Exception e) {
            e.printStackTrace();
            return false;
        }
    }

    private static boolean innerCheckSignUrl(String uri, byte[] appSecret) {
        URIProcessor processor =  URIProcessor.fromURL(uri);
        Map<String, List<String>> queryMap = processor.getQueryMap();
        String expect_sign = processor.getQueryParameter("sign");
        if (expect_sign == null) {
            throw new IllegalArgumentException("parameter sign should not be null");
        }
        if (appSecret == null) {
            throw new IllegalArgumentException("appSecret should not be null");
        }
        byte[] secret = appSecret;
        if (secret == null || secret.length < 32) {
            throw new IllegalArgumentException("appSecret should be at least 32 byte");
        }
        if (processor.getQueryParameter("port") == null) {
            throw new IllegalArgumentException("uri should contain port parameter");
        }
        processor.removeQueryParameter("sign");
        String signTime = processor.getQueryParameter("signTime");
        if (signTime == null) {
            throw new IllegalArgumentException("parameter signTime should not be null");
        }
        long signTimeNum = 0 ;
        try {
            signTimeNum = Long.parseLong(signTime);
        } catch (Exception e) {
            throw new IllegalArgumentException("parameter signTime is illegal number ", e);
        }
        long abs = Math.abs(System.currentTimeMillis() - signTimeNum);
        long expireTime = 1000*60*2L; //2*60秒过期
        if (abs > expireTime) {
            throw new IllegalArgumentException("signTime is expired, current time " + System.currentTimeMillis() + " abs: " + abs + " sign time " + signTimeNum);
        }
        TreeMap<String, String> signParameters = genSignMap(queryMap);
        Set<Map.Entry<String, String>> signEntries = signParameters.entrySet();
        StringBuilder sb = new StringBuilder(512);
        for(Map.Entry<String, String> entry : signEntries) {
            sb.append(entry.getKey());
            sb.append("=");
            sb.append(entry.getValue());
            sb.append("&");
        }
        sb.deleteCharAt(sb.length()-1);
        String signData = sb.toString();
        try {
            Mac mac = ThreadLocalUtil.getLocalValueEntry().getHmacSHA256();
            SecretKeySpec secretKey = new SecretKeySpec(secret, "HmacSHA256");
            mac.init(secretKey);
            byte[] genSign = mac.doFinal(signData.getBytes(StandardCharsets.UTF_8)); // 仅计算 HMAC-SHA256
            jdkx.compat.util.HexFormat hexFormat = jdkx.compat.util.HexFormat.of().withLowerCase();
            byte[] sign = hexFormat.parseHex(expect_sign);
            return Arrays.equals(genSign, sign);
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
    }

    private static TreeMap<String, String> genSignMap(Map<String, List<String>> queryMap) {
        TreeMap<String, String> signParameters = new TreeMap<>();
        Set<Map.Entry<String, List<String>>> entries = queryMap.entrySet();
        for(Map.Entry<String, List<String>> entry : entries) {
            List<String> values = entry.getValue();
            if (values != null && values.size() > 1) {
                throw new IllegalArgumentException("url one only can't contains multiple values for the same parameter, parameter key contains two values " + entry.getKey());
            }
            String value = "";
            if (values != null && values.size() == 1) {
                value = values.get(0);
            }
            signParameters.put(entry.getKey(), value);
        }
        return signParameters;
    }

    /**
     * sign url used 4183ms. sign count 1000000
     * sign check sign url used 2299ms. sign count 1000000
     * */
    private static void testPerf() {
        String signUri = null;
        {
            long start = System.currentTimeMillis();
            byte[] appSecret = "dssssssssssssssssssss1242142141241242142444444".getBytes(StandardCharsets.UTF_8);
            String uri = "http://127.0.0.1:8001?appId=333&role=client&port=8001";

            int times = 10000*100;
            for (int i = 0; i < times; i++) {
                signUri =  signUrl(uri, appSecret);
            }
            long end = System.currentTimeMillis();

            System.out.println("sign url used " + (end - start) + "ms. sign count " + times);
        }

        System.out.println("sign url " +  signUri);

        {
            long start = System.currentTimeMillis();
            byte[] appSecret = "dssssssssssssssssssss1242142141241242142444444".getBytes(StandardCharsets.UTF_8);
            String uri = signUri;

            int times = 10000*100;
            for (int i = 0; i < times; i++) {
                checkSignUrl(uri, appSecret);
            }
            long end = System.currentTimeMillis();

            System.out.println("sign check sign url used " + (end - start) + "ms. sign count " + times);
        }
        System.out.println("sign url " +  signUri);

    }

    private static void demo() {
        {
            byte[] appSecret = "dssssssssssssssssssss1242142141241242142444444".getBytes(StandardCharsets.UTF_8);
            String uri = "http://127.0.0.1:8001?appId=333&role=client&port=8001#test";
            String signUri = signUrl(uri, appSecret);
            System.out.println(uri);
            System.out.println(signUri);
            System.out.println(checkSignUrl(signUri.toString(), appSecret));
            System.out.println("after alert uri " + System.currentTimeMillis());
            System.out.println(checkSignUrl(URIProcessor.fromURI(signUri).appendQueryParameter("h", "eee").toURI().toString(), appSecret));
        }

        {
            System.out.println(URIProcessor.fromURI("http://example.com?search=java%2520%2526%2520uri&page=1").toURI().toString());
            System.out.println(URIProcessor.fromURI("http://example.com?search=java%2520%2526%2520uri&page=1").fastToUri());


            System.out.println(URI.create("http://example.com?param=%25G2").toString());

            //System.out.println(URIProcessor.fromURI("http://example.com?param=%25G2").fastToUri());


            System.out.println(URIProcessor.fromURI("mailto:test@example.com/test?test=1").toURI().toString());
            System.out.println(URIProcessor.fromURI("mailto:test@example.com?test=1").fastToUri());

        }
    }

    public static void main(String[] args) {
        demo();
        testPerf();
    }
}
