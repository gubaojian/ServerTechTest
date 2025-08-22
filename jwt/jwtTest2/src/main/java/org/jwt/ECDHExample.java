package org.jwt;

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
    }
}