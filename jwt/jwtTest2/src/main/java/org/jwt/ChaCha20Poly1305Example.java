package org.jwt;

import org.bouncycastle.jce.provider.BouncyCastleProvider;
import javax.crypto.Cipher;
import javax.crypto.SecretKey;
import javax.crypto.spec.GCMParameterSpec;
import javax.crypto.spec.SecretKeySpec;
import java.nio.charset.StandardCharsets;
import java.security.SecureRandom;
import java.security.Security;
import java.util.Base64;

/**
 * ChaCha20-Poly1305 加密解密示例（遵循 RFC 7539）
 * 核心参数：256位密钥（32字节）、12字节nonce、128位认证标签（16字节）
 * 密文格式：[nonce(12B)] + [密文] + [认证标签(16B)]
 */
public class ChaCha20Poly1305Example {
    // RFC 7539 规定：ChaCha20-Poly1305 密钥256位（32字节）、nonce12字节、标签128位（16字节）
    private static final int KEY_SIZE = 32;    // 密钥长度（字节）
    private static final int NONCE_SIZE = 12;  // Nonce长度（字节）
    private static final int TAG_SIZE = 128;   // 认证标签长度（bits）
    private static final String ALGORITHM = "ChaCha20-Poly1305"; // BouncyCastle 算法标识

    static {
        // 注册 BouncyCastle 安全提供者
        Security.addProvider(new BouncyCastleProvider());
    }

    /**
     * 生成 256位 ChaCha20-Poly1305 密钥
     */
    public static SecretKey generateKey() {
        SecureRandom random = new SecureRandom();
        byte[] keyBytes = new byte[KEY_SIZE];
        random.nextBytes(keyBytes); // 安全随机生成32字节密钥
        return new SecretKeySpec(keyBytes, ALGORITHM);
    }

    /**
     * 生成 12字节 Nonce（同一密钥下必须唯一，避免重复）
     */
    public static byte[] generateNonce() {
        SecureRandom random = new SecureRandom();
        byte[] nonce = new byte[NONCE_SIZE];
        random.nextBytes(nonce);
        return nonce;
    }

    /**
     * ChaCha20-Poly1305 加密（支持 AAD 附加认证数据）
     * @param plainText 明文（字符串）
     * @param key 256位密钥
     * @param nonce 12字节随机Nonce
     * @param aad 附加认证数据（可选，如无则传null）
     * @return 加密后的数据（Base64编码：nonce + 密文 + 标签）
     */
    public static String encrypt(
            String plainText,
            SecretKey key,
            byte[] nonce,
            byte[] aad
    ) throws Exception {
        // 1. 校验参数长度
        if (nonce.length != NONCE_SIZE) {
            throw new IllegalArgumentException("Nonce必须为 " + NONCE_SIZE + " 字节");
        }
        if (key.getEncoded().length != KEY_SIZE) {
            throw new IllegalArgumentException("密钥必须为 " + KEY_SIZE + " 字节（256位）");
        }

        // 2. 初始化 Cipher（AEAD 加密模式）
        Cipher cipher = Cipher.getInstance(ALGORITHM, "BC");
        GCMParameterSpec gcmParamSpec = new GCMParameterSpec(TAG_SIZE, nonce); // 标签长度+Nonce
        cipher.init(Cipher.ENCRYPT_MODE, key, gcmParamSpec);

        // 3. （可选）添加 AAD 附加认证数据（不参与加密但需验证完整性）
        if (aad != null && aad.length > 0) {
            cipher.updateAAD(aad);
        }

        // 4. 执行加密（输出：密文 + 16字节认证标签）
        byte[] cipherTextWithTag = cipher.doFinal(plainText.getBytes(StandardCharsets.UTF_8));

        // 5. 拼接格式：nonce(12B) + 密文 + 标签(16B)（方便解密时提取）
        byte[] encryptedData = new byte[NONCE_SIZE + cipherTextWithTag.length];
        System.arraycopy(nonce, 0, encryptedData, 0, NONCE_SIZE);
        System.arraycopy(cipherTextWithTag, 0, encryptedData, NONCE_SIZE, cipherTextWithTag.length);

        // 6. Base64编码（便于传输/存储）
        return Base64.getEncoder().encodeToString(encryptedData);
    }

    /**
     * ChaCha20-Poly1305 解密（支持验证 AAD）
     * @param encryptedBase64 加密后的数据（Base64编码：nonce + 密文 + 标签）
     * @param key 256位密钥（需与加密时一致）
     * @param aad 附加认证数据（需与加密时完全一致，无则传null）
     * @return 解密后的明文
     */
    public static String decrypt(
            String encryptedBase64,
            SecretKey key,
            byte[] aad
    ) throws Exception {
        // 1. Base64解码，获取原始加密数据（nonce + 密文 + 标签）
        byte[] encryptedData = Base64.getDecoder().decode(encryptedBase64);

        // 2. 拆分数据：前12字节是nonce，剩余是「密文+标签」
        if (encryptedData.length < NONCE_SIZE + (TAG_SIZE / 8)) {
            throw new IllegalArgumentException("加密数据格式错误（长度不足）");
        }
        byte[] nonce = new byte[NONCE_SIZE];
        byte[] cipherTextWithTag = new byte[encryptedData.length - NONCE_SIZE];
        System.arraycopy(encryptedData, 0, nonce, 0, NONCE_SIZE);
        System.arraycopy(encryptedData, NONCE_SIZE, cipherTextWithTag, 0, cipherTextWithTag.length);

        // 3. 初始化 Cipher（AEAD 解密模式，自动验证标签）
        Cipher cipher = Cipher.getInstance(ALGORITHM, "BC");
        GCMParameterSpec gcmParamSpec = new GCMParameterSpec(TAG_SIZE, nonce);
        cipher.init(Cipher.DECRYPT_MODE, key, gcmParamSpec);

        // 4. （可选）添加 AAD 验证（不一致会抛异常）
        if (aad != null && aad.length > 0) {
            cipher.updateAAD(aad);
        }

        // 5. 执行解密（标签验证失败会抛 InvalidKeyException）
        byte[] plainBytes = cipher.doFinal(cipherTextWithTag);
        return new String(plainBytes, StandardCharsets.UTF_8);
    }

    // 示例入口
    public static void main(String[] args) {
        try {
            // 1. 生成密钥（实际场景中需安全存储，如密钥管理系统）
            SecretKey key = generateKey();
            String keyBase64 = Base64.getEncoder().encodeToString(key.getEncoded());
            System.out.println("生成的256位密钥（Base64）：" + keyBase64);

            // 2. 生成Nonce（每次加密都需重新生成，不可重复）
            byte[] nonce = generateNonce();
            System.out.println("生成的12字节Nonce（Base64）：" + Base64.getEncoder().encodeToString(nonce));

            // 3. 原始数据与AAD（AAD可选，如可传用户ID、时间戳等上下文信息）
            String plainText = "ChaCha20-Poly1305 test";
            {
                String aadStr = "user:123;timestamp:20240821"; // 附加认证数据
                byte[] aad = aadStr.getBytes(StandardCharsets.UTF_8);
                System.out.println("\n原始明文：" + plainText);
                System.out.println("附加认证数据（AAD）：" + aadStr);

                // 4. 加密（带AAD）
                String encryptedBase64 = encrypt(plainText, key, nonce, aad);
                System.out.println("\n加密后数据（Base64）：" + encryptedBase64);

                // 5. 解密（需传入相同密钥和AAD）
                String decryptedText = decrypt(encryptedBase64, key, aad);
                System.out.println("\n解密后明文：" + decryptedText);

                // 6. 验证结果
                if (plainText.equals(decryptedText)) {
                    System.out.println("\n✅ 加密解密成功，结果一致！");
                } else {
                    System.out.println("\n❌ 加密解密失败，结果不一致！");
                }

            }

            {
                String aadStr = ""; // 附加认证数据
                byte[] aad = aadStr.getBytes(StandardCharsets.UTF_8);
                System.out.println("\n原始明文：" + plainText);
                System.out.println("附加认证数据无（AAD）：" + aadStr);

                // 4. 加密（带AAD）
                String encryptedBase64 = encrypt(plainText, key, nonce, aad);
                System.out.println("\n加密后数据（Base64）：" + encryptedBase64);

                // 5. 解密（需传入相同密钥和AAD）
                String decryptedText = decrypt(encryptedBase64, key, aad);
                System.out.println("\n解密后明文：" + decryptedText);

                // 6. 验证结果
                if (plainText.equals(decryptedText)) {
                    System.out.println("\n✅ 加密解密成功，结果一致！");
                } else {
                    System.out.println("\n❌ 加密解密失败，结果不一致！");
                }

            }

            // ------------------- 测试：AAD不一致的情况（会抛异常）-------------------
            // String wrongAad = "user:456;timestamp:20240821";
            // decrypt(encryptedBase64, key, wrongAad.getBytes(StandardCharsets.UTF_8));

        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
