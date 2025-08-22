package org.jwt;

import org.bouncycastle.crypto.digests.SHA256Digest;
import org.bouncycastle.crypto.digests.SHA384Digest;
import org.bouncycastle.crypto.digests.SHA512Digest;
import org.bouncycastle.crypto.generators.HKDFBytesGenerator;
import org.bouncycastle.crypto.params.HKDFParameters;

import java.nio.charset.StandardCharsets;
import java.security.*;
import java.security.spec.ECGenParameterSpec;
import javax.crypto.KeyAgreement;
import java.util.Arrays;
import java.util.Base64;

public class ECDHExample {
    public static void main(String[] args) throws Exception {
        // 1. 初始化椭圆曲线密钥对生成器（使用 secp256r1 曲线）
        KeyPairGenerator keyPairGenerator = KeyPairGenerator.getInstance("EC");
        ECGenParameterSpec curveSpec = new ECGenParameterSpec("secp256r1");
        keyPairGenerator.initialize(curveSpec);

        // 2. 生成 Alice 和 Bob 的密钥对（模拟通信双方）
        KeyPair aliceKeyPair = keyPairGenerator.generateKeyPair();
        KeyPair bobKeyPair = keyPairGenerator.generateKeyPair();

        System.out.println(Base64.getEncoder().encodeToString(aliceKeyPair.getPrivate().getEncoded()));
        System.out.println(Base64.getEncoder().encodeToString(aliceKeyPair.getPublic().getEncoded()));
        System.out.println(Base64.getEncoder().encodeToString(bobKeyPair.getPrivate().getEncoded()));
        System.out.println(Base64.getEncoder().encodeToString(bobKeyPair.getPublic().getEncoded()));

        // 3. 初始化 ECDH 密钥协商对象
        KeyAgreement aliceAgreement = KeyAgreement.getInstance("ECDH");
        KeyAgreement bobAgreement = KeyAgreement.getInstance("ECDH");

        // 4. 各自用私钥初始化协商器
        aliceAgreement.init(aliceKeyPair.getPrivate());
        bobAgreement.init(bobKeyPair.getPrivate());


        // 5. 交换公钥，执行密钥协商（`true` 表示这是最后一步协商）
        aliceAgreement.doPhase(bobKeyPair.getPublic(), true);
        bobAgreement.doPhase(aliceKeyPair.getPublic(), true);

        // 6. 生成共享密钥（原始二进制数据）
        byte[] aliceSharedKey = aliceAgreement.generateSecret();
        byte[] bobSharedKey = bobAgreement.generateSecret();


        // 验证：双方共享密钥应完全一致
        System.out.println("Alice 共享密钥长度: " + aliceSharedKey.length + " 字节");
        System.out.println("Bob 共享密钥长度: " + bobSharedKey.length + " 字节");
        System.out.println("密钥是否一致: " + Arrays.equals(aliceSharedKey, bobSharedKey));
        System.out.println(Base64.getEncoder().encodeToString(aliceSharedKey));
        System.out.println(Base64.getEncoder().encodeToString(bobSharedKey));

        {
            String hashAlg = "SHA-256"; // 公开哈希算法
            int keyLen = 32; // 派生密钥长度（32字节=256位）
            byte[] salt = "hkdf-salt".getBytes(StandardCharsets.UTF_8); // 随机盐（公开API生成）
            byte[] info = "standard-hkdf-example".getBytes(); // 上下文信息
            byte[]  secret = "test hkdf 2".getBytes(StandardCharsets.UTF_8);
            // 4. 使用BouncyCastle公开API（HKDFBytesGenerator）派生密钥
            byte[] aliceDerived = hkdf(secret, salt, info, hashAlg, keyLen);
            System.out.println(Base64.getEncoder().encodeToString(aliceDerived));
        }
    }




    /**
     * HKDF 派生实现（兼容 BouncyCastle 最新版本）
     */
    /**
     * 基于BouncyCastle公开API实现HKDF
     * 使用HKDFBytesGenerator（公开类）和HKDFParameters（公开类）
     */
    private static byte[] hkdf(byte[] ikm, byte[] salt, byte[] info, String hashAlg, int keyLen) {
        // 1. 选择公开的哈希算法实现（BouncyCastle公开类）
        org.bouncycastle.crypto.Digest digest;
        switch (hashAlg.toUpperCase()) {
            case "SHA-256":
                digest = new SHA256Digest(); // 公开类
                break;
            case "SHA-384":
                digest = new SHA384Digest(); // 公开类
                break;
            case "SHA-512":
                digest = new SHA512Digest(); // 公开类
                break;
            default:
                throw new IllegalArgumentException("不支持的哈希算法: " + hashAlg);
        }

        // 2. 初始化HKDF生成器（公开API）
        HKDFBytesGenerator hkdf = new HKDFBytesGenerator(digest); // 公开类
        // 初始化参数（公开类HKDFParameters）
        hkdf.init(new HKDFParameters(ikm, salt, info)); // 公开类

        // 3. 派生密钥（公开API方法）
        byte[] derived = new byte[keyLen];
        hkdf.generateBytes(derived, 0, keyLen); // 公开方法

        return derived;
    }

    /**
     * 生成随机盐值（使用JDK公开的SecureRandom）
     */
    private static byte[] generateSalt(int length) {
        byte[] salt = new byte[length];
        new SecureRandom().nextBytes(salt); // JDK公开API
        return salt;
    }
}