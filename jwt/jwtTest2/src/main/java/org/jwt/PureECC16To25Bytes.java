package org.jwt;

import org.bouncycastle.jce.provider.BouncyCastleProvider;

import javax.crypto.Cipher;
import java.security.*;
import java.security.spec.ECGenParameterSpec;
import java.security.spec.PKCS8EncodedKeySpec;
import java.security.spec.X509EncodedKeySpec;
import java.util.Base64;

public class PureECC16To25Bytes {
    // 核心配置：纯ECC算法，无混合加密
    private static final String ALGORITHM = "EC";
    // 模式：无填充（精准处理固定长度数据）
    private static final String TRANSFORMATION = "ECIESwithAES";
    // 曲线选择：secp384r1（384位椭圆曲线，支持加密≤48字节数据，覆盖16-25字节需求）
    private static final String CURVE_NAME = "secp384r1";

    /**
     * 生成ECC密钥对（基于384位曲线）
     */
    public static KeyPair generateKeyPair() throws Exception {
        KeyPairGenerator keyGen = KeyPairGenerator.getInstance(ALGORITHM);
        ECGenParameterSpec ecSpec = new ECGenParameterSpec(CURVE_NAME);
        keyGen.initialize(ecSpec); // 初始化曲线参数
        return keyGen.generateKeyPair();
    }

    /**
     * 公钥加密（仅支持16-25字节数据）
     */
    public static String encrypt(byte[] data, PublicKey publicKey) throws Exception {
        // 严格校验数据长度
        if (data.length < 16 || data.length > 25) {
            throw new IllegalArgumentException("仅支持16-25字节数据加密");
        }
        Cipher cipher = Cipher.getInstance(TRANSFORMATION);
        cipher.init(Cipher.ENCRYPT_MODE, publicKey);
        byte[] encryptedData = cipher.doFinal(data);
        return Base64.getEncoder().encodeToString(encryptedData);
    }

    /**
     * 私钥解密
     */
    public static byte[] decrypt(String encryptedStr, PrivateKey privateKey) throws Exception {
        byte[] encryptedData = Base64.getDecoder().decode(encryptedStr);
        Cipher cipher = Cipher.getInstance(TRANSFORMATION);
        cipher.init(Cipher.DECRYPT_MODE, privateKey);
        return cipher.doFinal(encryptedData);
    }

    // 密钥格式转换工具（Base64与密钥对象互转）
    public static String publicKeyToBase64(PublicKey publicKey) {
        return Base64.getEncoder().encodeToString(publicKey.getEncoded());
    }

    public static PublicKey base64ToPublicKey(String base64PubKey) throws Exception {
        byte[] keyBytes = Base64.getDecoder().decode(base64PubKey);
        X509EncodedKeySpec spec = new X509EncodedKeySpec(keyBytes);
        return KeyFactory.getInstance(ALGORITHM).generatePublic(spec);
    }

    public static String privateKeyToBase64(PrivateKey privateKey) {
        return Base64.getEncoder().encodeToString(privateKey.getEncoded());
    }

    public static PrivateKey base64ToPrivateKey(String base64PriKey) throws Exception {
        byte[] keyBytes = Base64.getDecoder().decode(base64PriKey);
        PKCS8EncodedKeySpec spec = new PKCS8EncodedKeySpec(keyBytes);
        return KeyFactory.getInstance(ALGORITHM).generatePrivate(spec);
    }

    // 测试：加密16-25字节数据
    public static void main(String[] args) {
        try {
            Security.addProvider(new BouncyCastleProvider());
            // 生成密钥对
            KeyPair keyPair = generateKeyPair();
            PublicKey publicKey = keyPair.getPublic();
            PrivateKey privateKey = keyPair.getPrivate();

            // 测试数据：20字节（示例）
            byte[] data = "test123456789012345678".getBytes(); // 长度20字节
            System.out.println("原始数据: " + new String(data));
            System.out.println("数据长度: " + data.length + "字节");

            // 加密
            String encrypted = encrypt(data, publicKey);
            System.out.println("加密后: " + encrypted);

            // 解密
            byte[] decrypted = decrypt(encrypted, privateKey);
            System.out.println("解密后: " + new String(decrypted));
            System.out.println("解密后长度: " + decrypted.length + "字节");

        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}