import javax.crypto.Cipher;
import javax.crypto.KeyGenerator;
import javax.crypto.SecretKey;
import javax.crypto.spec.IvParameterSpec;
import javax.crypto.spec.SecretKeySpec;
import java.nio.charset.StandardCharsets;
import java.security.SecureRandom;
import java.security.Security;
import java.util.Arrays;
import java.util.Base64;

import org.bouncycastle.jce.provider.BouncyCastleProvider;

public class ChaCha20CipherExample {
    // ChaCha20密钥长度为256位(32字节)
    private static final int KEY_SIZE = 256;
    // 非ce长度为96位(12字节) - 符合IETF标准
    private static final int NONCE_SIZE = 12;
    // 算法名称
    private static final String ALGORITHM = "ChaCha20";

    static {
        // 添加BouncyCastle作为安全提供者
        Security.addProvider(new BouncyCastleProvider());
    }

    /**
     * 生成ChaCha20密钥
     * @return 生成的密钥
     * @throws Exception 可能的异常
     */
    public static SecretKey generateKey() throws Exception {
        KeyGenerator keyGenerator = KeyGenerator.getInstance(ALGORITHM, "BC");
        keyGenerator.init(KEY_SIZE);
        return keyGenerator.generateKey();
    }

    /**
     * 生成随机非ce
     * @return 生成的非ce
     */
    public static byte[] generateNonce() {
        SecureRandom random = new SecureRandom();
        byte[] nonce = new byte[NONCE_SIZE];
        random.nextBytes(nonce);
        return nonce;
    }

    /**
     * 使用ChaCha20算法加密数据
     * @param plaintext 明文数据
     * @param key 密钥
     * @param nonce 非ce
     * @return 加密后的密文
     * @throws Exception 可能的异常
     */
    public static byte[] encrypt(byte[] plaintext, SecretKey key, byte[] nonce) throws Exception {
        if (nonce.length != NONCE_SIZE) {
            throw new IllegalArgumentException("非ce长度必须为" + NONCE_SIZE + "字节");
        }

        // 获取Cipher实例，使用BouncyCastle提供的ChaCha20实现
        Cipher cipher = Cipher.getInstance(ALGORITHM, "BC");
        // 初始化加密模式，传入密钥和非ce
        IvParameterSpec ivSpec = new IvParameterSpec(nonce);
        cipher.init(Cipher.ENCRYPT_MODE, key, ivSpec);

        // 执行加密
        return cipher.doFinal(plaintext);
    }

    /**
     * 使用ChaCha20算法解密数据
     * @param ciphertext 密文数据
     * @param key 密钥
     * @param nonce 非ce
     * @return 解密后的明文
     * @throws Exception 可能的异常
     */
    public static byte[] decrypt(byte[] ciphertext, SecretKey key, byte[] nonce) throws Exception {
        if (nonce.length != NONCE_SIZE) {
            throw new IllegalArgumentException("非ce长度必须为" + NONCE_SIZE + "字节");
        }

        // 获取Cipher实例，使用BouncyCastle提供的ChaCha20实现
        Cipher cipher = Cipher.getInstance(ALGORITHM, "BC");
        // 初始化解密模式，传入密钥和非ce
        IvParameterSpec ivSpec = new IvParameterSpec(nonce);
        cipher.init(Cipher.DECRYPT_MODE, key, ivSpec);

        // 执行解密
        return cipher.doFinal(ciphertext);
    }

    public static void main(String[] args) {
        try {
            // 1. 生成密钥并以Base64格式输出
            SecretKey key = generateKey();
            String keyBase64 = Base64.getEncoder().encodeToString(key.getEncoded());
            System.out.println("生成的ChaCha20密钥(Base64): " + keyBase64);

            // 从Base64字符串恢复密钥（演示如何从存储的密钥恢复）
            SecretKey restoredKey = new SecretKeySpec(
                    Base64.getDecoder().decode(keyBase64),
                    ALGORITHM
            );

            // 2. 生成非ce
            byte[] nonce = generateNonce();
            String nonceBase64 = Base64.getEncoder().encodeToString(nonce);
            System.out.println("生成的非ce(Base64): " + nonceBase64);

            // 3. 原始数据
            String originalText = "这是一个使用标准Cipher API的ChaCha20加密解密示例";
            System.out.println("原始文本: " + originalText);

            // 4. 加密
            byte[] ciphertext = encrypt(originalText.getBytes(StandardCharsets.UTF_8), restoredKey, nonce);
            String ciphertextBase64 = Base64.getEncoder().encodeToString(ciphertext);
            System.out.println("加密后的文本(Base64): " + ciphertextBase64);
            System.out.println("加密后的文本带once(Base64): " + Base64.getEncoder().encodeToString(
                    org.bouncycastle.util.Arrays.concatenate(nonce, ciphertext)
            ));

            // 5. 解密
            byte[] decryptedData = decrypt(Base64.getDecoder().decode(ciphertextBase64), restoredKey, nonce);
            String decryptedText = new String(decryptedData, StandardCharsets.UTF_8);
            System.out.println("解密后的文本: " + decryptedText);

            // 验证解密结果
            if (originalText.equals(decryptedText)) {
                System.out.println("加密解密成功，结果一致！");
            } else {
                System.out.println("加密解密失败，结果不一致！");
            }

        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
    