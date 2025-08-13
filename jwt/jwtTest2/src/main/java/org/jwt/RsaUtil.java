package org.jwt;


import javax.crypto.Cipher;
import java.nio.charset.StandardCharsets;
import java.security.*;
import java.security.spec.MGF1ParameterSpec;
import java.security.spec.PKCS8EncodedKeySpec;
import java.security.spec.PSSParameterSpec;
import java.security.spec.X509EncodedKeySpec;
import java.util.Base64;

public class RsaUtil {
    private static final String algorithm = "RSA";

    private static final int keySize = 2048;

    public static final String PUBLIC_KEY = "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQCd82hP7GCN+0zLiMadevD1ZWhRmyOfaQcQ3s69tRVDqHw7Bm7PD0IARwUKvJ0Zb2qKbWAX5tDdDSZGqIlG7i5ph7qN7jxKvzDhajXxZZaA9vc4PBNpLAxbw1pARPEc+nuyuM5dUKM446hlHtc0Tlu2qcVva77nhOPZwHphVB6cbQIDAQAB";

    /**
     * 生成RSA密钥对
     * @return 密钥对
     * @throws NoSuchAlgorithmException 算法不存在异常
     */
    public static KeyPair generateKeyPair() throws NoSuchAlgorithmException {
        KeyPairGenerator keyPairGenerator = KeyPairGenerator.getInstance(algorithm);
        keyPairGenerator.initialize(keySize);
        return keyPairGenerator.generateKeyPair();
    }

    /**
     * 获取公钥字符串
     * @param keyPair 密钥对
     * @return 公钥Base64编码字符串
     */
    public static String getPublicKeyStr(KeyPair keyPair) {
        PublicKey publicKey = keyPair.getPublic();
        return Base64.getEncoder().encodeToString(publicKey.getEncoded());
    }

    /**
     * 获取私钥字符串
     * @param keyPair 密钥对
     * @return 私钥Base64编码字符串
     */
    public static String getPrivateKeyStr(KeyPair keyPair) {
        PrivateKey privateKey = keyPair.getPrivate();
        return Base64.getEncoder().encodeToString(privateKey.getEncoded());
    }

    /**
     * rsa加密
     * @param plainText 明文
     * @param publicKeyStr 公钥字符串
     * @return 密文Base64编码字符串
     * @throws Exception 加密异常
     */
    public static String encrypt(String plainText, String publicKeyStr) throws Exception {
        byte[] publicKeyBytes = Base64.getDecoder().decode(publicKeyStr);
        X509EncodedKeySpec keySpec = new X509EncodedKeySpec(publicKeyBytes);
        KeyFactory keyFactory = KeyFactory.getInstance(algorithm);
        PublicKey publicKey = keyFactory.generatePublic(keySpec);
        Cipher cipher = Cipher.getInstance("RSA/ECB/PKCS1Padding");
        cipher.init(Cipher.ENCRYPT_MODE, publicKey);
        byte[] encryptedBytes = cipher.doFinal(plainText.getBytes(StandardCharsets.UTF_8));
        return Base64.getEncoder().encodeToString(encryptedBytes);
    }

    /**
     * rsa解密
     * @param encryptedText 密文Base64编码字符串
     * @param privateKeyStr 私钥字符串
     * @return 明文
     * @throws Exception 解密异常
     */
    public static String decrypt(String encryptedText, String privateKeyStr) throws Exception {
        byte[] privateKeyBytes = Base64.getDecoder().decode(privateKeyStr);
        PKCS8EncodedKeySpec keySpec = new PKCS8EncodedKeySpec(privateKeyBytes);
        KeyFactory keyFactory = KeyFactory.getInstance(algorithm);
        PrivateKey privateKey = keyFactory.generatePrivate(keySpec);
        Cipher cipher = Cipher.getInstance("RSA/ECB/PKCS1Padding");
        cipher.init(Cipher.DECRYPT_MODE, privateKey);
        byte[] decodedBytes = Base64.getDecoder().decode(encryptedText);
        byte[] decryptedBytes = cipher.doFinal(decodedBytes);
        return new String(decryptedBytes, StandardCharsets.UTF_8);
    }

    public static String sign(String data, String privateKeyStr) throws Exception {
        byte[] privateKeyBytes = Base64.getDecoder().decode(privateKeyStr);
        PKCS8EncodedKeySpec keySpec = new PKCS8EncodedKeySpec(privateKeyBytes);
        KeyFactory keyFactory = KeyFactory.getInstance(algorithm);
        PrivateKey privateKey = keyFactory.generatePrivate(keySpec);

        Signature signature = Signature.getInstance("RSASSA-PSS");
        PSSParameterSpec pssParams = new PSSParameterSpec(
                "SHA-256",
                "MGF1",
                MGF1ParameterSpec.SHA256,
                32,  // 盐长度
                1    //  trailers field
        );

        signature.initSign(privateKey);
        signature.setParameter(pssParams);

        signature.update(data.getBytes(StandardCharsets.UTF_8));
        byte[] sign = signature.sign();
        return Base64.getEncoder().encodeToString(sign);
    }

    public static boolean verify(String sign, String plainText, String publicKeyStr) throws Exception {
        byte[] publicKeyBytes = Base64.getDecoder().decode(publicKeyStr);
        X509EncodedKeySpec keySpec = new X509EncodedKeySpec(publicKeyBytes);
        KeyFactory keyFactory = KeyFactory.getInstance(algorithm);
        PublicKey publicKey = keyFactory.generatePublic(keySpec);
        Signature signature = Signature.getInstance("RSASSA-PSS");
        PSSParameterSpec pssParams = new PSSParameterSpec(
                "SHA-256",
                "MGF1",
                MGF1ParameterSpec.SHA256,
                32,  // 盐长度
                1    //  trailers field
        );

        signature.initVerify(publicKey);
        signature.setParameter(pssParams);

        signature.update(plainText.getBytes(StandardCharsets.UTF_8));

       return signature.verify(Base64.getDecoder().decode(sign.getBytes(StandardCharsets.UTF_8)));
    }


}

