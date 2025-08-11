package com.hku.common.core.demo;



import javax.crypto.Cipher;
import java.nio.charset.StandardCharsets;
import java.security.*;
import java.security.spec.PKCS8EncodedKeySpec;
import java.security.spec.X509EncodedKeySpec;
import java.util.Base64;

/**
 * @author pengzhengfa
 */
public class EncryptUtil {

    private static final String algorithm = "RSA";

    private static final int keySize = 1024;

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

    public static void main(String[] args) {
        try {
            // 生成密钥对
            KeyPair keyPair = generateKeyPair();
            String publicKeyStr = getPublicKeyStr(keyPair);
            String privateKeyStr = getPrivateKeyStr(keyPair);
            System.out.println("公钥: " + publicKeyStr);
            System.out.println("私钥: " + privateKeyStr);
            // 待加密的文本
            String plainText = "Hello, RSA Encryption!";
            System.out.println("明文: " + plainText);
            // 加密
            String encryptedText = encrypt(plainText, publicKeyStr);
            System.out.println("密文: " + encryptedText);
            // 解密
            String decryptedText = decrypt(encryptedText, privateKeyStr);
            System.out.println("解密后: " + decryptedText);


           final String PUBLIC_KEY = "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQCd82hP7GCN+0zLiMadevD1ZWhRmyOfaQcQ3s69tRVDqHw7Bm7PD0IARwUKvJ0Zb2qKbWAX5tDdDSZGqIlG7i5ph7qN7jxKvzDhajXxZZaA9vc4PBNpLAxbw1pARPEc+nuyuM5dUKM446hlHtc0Tlu2qcVva77nhOPZwHphVB6cbQIDAQAB";

            final String PRIVATE_KEY = "MIICdgIBADANBgkqhkiG9w0BAQEFAASCAmAwggJcAgEAAoGBAJ3zaE/sYI37TMuIxp168PVlaFGbI59pBxDezr21FUOofDsGbs8PQgBHBQq8nRlvaoptYBfm0N0NJkaoiUbuLmmHuo3uPEq/MOFqNfFlloD29zg8E2ksDFvDWkBE8Rz6e7K4zl1QozjjqGUe1zROW7apxW9rvueE49nAemFUHpxtAgMBAAECgYBF2/RU9R03buBmXRZLbKbKQpHDFjTJOrCvW3xhF4hSJmDb0lqMJhlw+fhW6VkqeMbMdte7epECU+CL+9rLjmjj/FyASBJdMtiXqVmUWFw3EWYtPzMqNs0/rT+CrWOQKD9rUBswQ3XKqFpqiVCrOaPK2ttQSXTDdJuQQLCqg6SSGQJBAObMwV7uY1agkdrGHbMy6JfrE9jhMdpDHguBkP+9ZYJgs1BOfLFkx57qF0LEHmUHkG4W8Obb6EcHxbKMK7lAcusCQQCvMmYxjq7kPy1LET2S1jPMLjEp8ci6M3LdJESt+yBODUtm0SmaTC8LyT9uSLUzMHbcuFcoVlnBiJeTekWbBGgHAkEAovY8Kw1jcs0Zb0GnwtnVRT8k2mPXUWdGlZU8jnZqcC5bLf7dKFwiXuCm3+ueEnPO1Qy5uCfZ0B16x+EVFk6dVQJAfNUzffJF6bqeYlOphPv9smnjfBicxxgrqyjRiwpZ7WOJq3wz4Y6RIxvFh6Zk/lF1EDaFW+eTUtQp9Np1yEvudwJASd77d8Q1RmWGQKq0Mt0CDxL9PqoaL7SbbquizyV8FSzaZ1HZiZweBi1wmayyPWzMINQY8IFhFH2fUYCvZ09cZA==";

            String decryptedText2 = decrypt("lM4m5FC1K4r69uLy9S++JaF9TP9R/WHHmtVS950eRfEwufpP/apnmsvlnLnZwnmLL7yPPF284OzCfMaaeRb8hDqHlmJUAr+LnCyOldV4BzzA6jxMwlO/nPiUTq/d3BZ2WZ0uz4xt16l4JVw7ACsXwde1dUSk2uF2txmViFHHDhk=", PRIVATE_KEY);
            System.out.println("解密后: " + decryptedText2);



        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}