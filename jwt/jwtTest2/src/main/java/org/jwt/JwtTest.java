package org.jwt;

import io.jsonwebtoken.Jwts;
import io.jsonwebtoken.SignatureAlgorithm;

import javax.crypto.Cipher;
import javax.crypto.Mac;
import javax.crypto.spec.SecretKeySpec;
import java.nio.charset.StandardCharsets;
import java.security.*;
import java.util.*;

/**
 * 保存测试
 * */
public class JwtTest {
    public static void main(String[] args) throws Exception {

       testJwtSpeed();

        testHMACSpeed();

        testHMACSpeedForCplus();

        testRSASignSpeed();

        testRSAEncryptionSpeed();

    }

    public static void testJwtSpeed() {
        String secretKey = "i8zIHoTLy2t4uMIztIUi3vA129xYVKAE"; //RandomStringUtils.randomAlphanumeric(32); //32最小
        Key key = new SecretKeySpec(secretKey.getBytes(StandardCharsets.UTF_8), SignatureAlgorithm.HS256.getJcaName());
        String connId = UUID.randomUUID().toString();
        String verifyToken = null;
        {
            long start = System.currentTimeMillis();
            for (int i=0; i<10000*10; i++) {
                String token = generateToken(
                        connId, 1000*60*30,  key);
                verifyToken = token;
            }
            long end = System.currentTimeMillis();

            System.out.println("generate token used " + (end - start));
        }

        System.out.println(verifyToken);
    }

    public static void testHMACSpeed() throws NoSuchAlgorithmException, InvalidKeyException {
        String key = "i8zIHoTLy2t4uMIztIUi3vA129xYVKAE"; //RandomStringUtils.randomAlphanumeric(32); //32最小


        String data = "{\"alg\":\"HS256\",\"typ\":\"JWT\"}.{\"connId\":\"abc123\",\"iat\":1620000000,\"exp\":1620000000+1800}";
        byte[] dataBytes = data.getBytes(StandardCharsets.UTF_8);

        String sign = null;
        long start = System.currentTimeMillis();
        for (int i = 0; i < 10000*100; i++) {
            Mac mac = Mac.getInstance("HmacSHA256");
            SecretKeySpec secretKey = new SecretKeySpec(key.getBytes(StandardCharsets.UTF_8), "HmacSHA256");
            mac.init(secretKey);
            byte[] bts = mac.doFinal(dataBytes); // 仅计算 HMAC-SHA256
            sign = Base64.getUrlEncoder().encodeToString(bts);
        }
        long end = System.currentTimeMillis();

        System.out.println("hmac used " + (end - start));
        System.out.println("hmac sign " +  sign);
    }

    public static void testHMACSpeedForCplus() throws NoSuchAlgorithmException, InvalidKeyException {
        String key = "hello world"; //RandomStringUtils.randomAlphanumeric(32); //32最小
        Mac mac = Mac.getInstance("HmacSHA256");
        SecretKeySpec secretKey = new SecretKeySpec(key.getBytes(StandardCharsets.UTF_8), "HmacSHA256");
        mac.init(secretKey);

        String data = "test sign";
        byte[] dataBytes = data.getBytes(StandardCharsets.UTF_8);

        String sign = null;
        byte[] bts = mac.doFinal(dataBytes); // 仅计算 HMAC-SHA256
        sign = Base64.getEncoder().encodeToString(bts);

        System.out.println("hmac sign result " + sign);
    }

    public static void testRSASignSpeed() throws Exception {
        // 生成RSA2048密钥对（实际应用中只需生成一次）
        KeyPair keyPair = generateRSAKeyPair();
        PrivateKey privateKey = keyPair.getPrivate();

        // 待签名的数据（与HMAC测试保持一致）
        String data = "{\"alg\":\"RS256\",\"typ\":\"JWT\"}.{\"connId\":\"abc123\",\"iat\":1620000000,\"exp\":1620000000+1800}";
        byte[] dataBytes = data.getBytes(StandardCharsets.UTF_8);

        // 初始化签名器


        String sign = null;
        long start = System.currentTimeMillis();
        for (int i = 0; i < 10000; i++) {
            Signature signature = Signature.getInstance("SHA256withRSA");
            signature.initSign(privateKey);
            // 每次签名都需要重新传入数据（RSA签名器不支持重复使用相同数据）
            signature.update(dataBytes);
            byte[] signBytes = signature.sign();
            sign = Base64.getUrlEncoder().encodeToString(signBytes);

        }
        long end = System.currentTimeMillis();

        System.out.println("RSA2048 耗时: " + (end - start) + "ms");
        System.out.println("RSA2048 签名结果: " + sign);
    }


    public static void testRSAEncryptionSpeed() throws Exception {
        KeyPair keyPair = generateRSAKeyPair();
        PublicKey publicKey = keyPair.getPublic();
        PrivateKey privateKey = keyPair.getPrivate();
        String text = "hello world中国";
        byte[] encrptedData = null;
        {
            byte[] data = text.getBytes(StandardCharsets.UTF_8);
            Cipher cipher = Cipher.getInstance("RSA/ECB/PKCS1Padding");
            cipher.init(Cipher.ENCRYPT_MODE, publicKey);

            long start = System.currentTimeMillis();
            int iterations = 1000; // 加密次数

            for (int i = 0; i < iterations; i++) {
                byte[] encrypted = cipher.doFinal(data);
                encrptedData = encrypted;
            }

            long end = System.currentTimeMillis();
            long totalTime = end - start;

            System.out.println("RSA加密测试:");
            System.out.println("执行次数: " + iterations);
            System.out.println("总耗时: " + totalTime + "ms");
            System.out.println("平均每次耗时: " + (totalTime * 1.0 / iterations) + "ms");
            System.out.println("每秒可执行: " + (iterations * 1000.0 / totalTime) + "次");
        }

        {
            Cipher cipher = Cipher.getInstance("RSA/ECB/PKCS1Padding");
            cipher.init(Cipher.DECRYPT_MODE, privateKey);

            long start = System.currentTimeMillis();
            int iterations = 1000; // 解密次数

            for (int i = 0; i < iterations; i++) {
                // 执行解密
                byte[] decrypted = cipher.doFinal(encrptedData);
                // 可以在这里将字节数组转换为字符串模拟实际使用场景
                // String decryptedStr = new String(decrypted, StandardCharsets.UTF_8);
            }

            long end = System.currentTimeMillis();
            long totalTime = end - start;

            System.out.println("\nRSA解密测试:");
            System.out.println("执行次数: " + iterations);
            System.out.println("总耗时: " + totalTime + "ms");
            System.out.println("平均每次耗时: " + (totalTime * 1.0 / iterations) + "ms");
            System.out.println("每秒可执行: " + (iterations * 1000.0 / totalTime) + "次");
        }

    }

    public static String generateToken(String connId, long expireTime, Key key) {
        Map<String,String> claims = new HashMap<>();
        claims.put("connId", connId);
        if (expireTime <= 0) {
            return Jwts.builder()
                    .setClaims(claims)
                    .setIssuedAt(new Date(System.currentTimeMillis()))
                    .signWith(key, SignatureAlgorithm.HS256)
                    .compact();
        }
        return Jwts.builder()
                .setClaims(claims)
                .setIssuedAt(new Date(System.currentTimeMillis()))
                .setExpiration(new Date(System.currentTimeMillis() + expireTime))
                .signWith(key, SignatureAlgorithm.HS256)
                .compact();
    }

    private static KeyPair generateRSAKeyPair() throws Exception {
        KeyPairGenerator keyPairGenerator = KeyPairGenerator.getInstance("RSA");
        keyPairGenerator.initialize(2048); // 2048位密钥
        return keyPairGenerator.generateKeyPair();
    }
}
