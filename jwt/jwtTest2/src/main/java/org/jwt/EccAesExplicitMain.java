package org.jwt;

import javax.crypto.*;
import javax.crypto.spec.GCMParameterSpec;
import javax.crypto.spec.SecretKeySpec;
import java.security.*;
import java.security.spec.ECGenParameterSpec;
import java.security.spec.ECParameterSpec;
import java.security.spec.InvalidParameterSpecException;
import java.util.Base64;

// BouncyCastle 依赖（需引入 bcprov-jdk15on-1.70+ jar 包）
import org.bouncycastle.jce.provider.BouncyCastleProvider;
import org.bouncycastle.jce.spec.IESParameterSpec;
import org.bouncycastle.util.Arrays;

/**
 * ECIES + AES-256-GCM 混合加密实现
 * 关键特性：与 C 端 OpenSSL 完全兼容，参数显式配置（无默认值依赖）
 */
public class EccAesExplicitMain {
    // -------------------------- 核心配置（与 C 端 OpenSSL 一一对应）--------------------------
    private static final String ECC_CURVE = "secp256r1";          // C端：NID_X9_62_prime256v1
    private static final String ECIES_ALGORITHM = "ECIES"; // C端：SHA256 + AES-256-CBC
    private static final String AES_ALGORITHM = "AES/GCM/NoPadding";    // C端：AES-256-GCM
    private static final int AES_KEY_BITS = 256;                  // AES-256 密钥长度（32字节）
    private static final int GCM_IV_BYTES = 12;                   // GCM 推荐 IV 长度（与 C 端一致）
    private static final int GCM_TAG_BYTES = 16;                  // GCM 认证标签长度（与 C 端一致）
    private static final String BC_PROVIDER = "BC";               // BouncyCastle 提供者名称

    // 静态加载 BouncyCastle 提供者（确保优先生效）
    static {
        if (Security.getProvider(BC_PROVIDER) == null) {
            Security.addProvider(new BouncyCastleProvider());
            System.out.println("[初始化] 已添加 BouncyCastle 安全提供者");
        } else {
            System.out.println("[初始化] BouncyCastle 提供者已存在");
        }
    }

    public static void main(String[] args) {
        try {
            // -------------------------- 步骤1：生成 ECC 密钥对（基于标准曲线参数）--------------------------
            System.out.println("\n===== 步骤1：生成 ECC 密钥对（secp256r1） =====");
            ECParameterSpec ecDomainSpec = getSecp256r1ParameterSpec();
            KeyPair eccKeyPair = generateEccKeyPair(ecDomainSpec);
            PublicKey eccPublicKey = eccKeyPair.getPublic();
            PrivateKey eccPrivateKey = eccKeyPair.getPrivate();

            System.out.println("ECC 公钥（Base64）：" + Base64.getEncoder().encodeToString(eccPublicKey.getEncoded()));
            System.out.println("ECC 私钥（Base64）：" + Base64.getEncoder().encodeToString(eccPrivateKey.getEncoded()));

            // -------------------------- 步骤2：生成临时 AES-256 密钥 --------------------------
            System.out.println("\n===== 步骤2：生成临时 AES-256 密钥 =====");
            SecretKey aesKey = generateAes256Key();
            System.out.println("AES-256 密钥（Base64）：" + Base64.getEncoder().encodeToString(aesKey.getEncoded()));

            // -------------------------- 步骤3：准备待加密的原始数据 --------------------------
            String originalData = "ECIES+AES-256-GCM 跨语言加密测试（Java ↔ C/OpenSSL）";
            System.out.println("\n===== 步骤3：原始数据 =====");
            System.out.println("原始内容：" + originalData);
            byte[] dataBytes = originalData.getBytes("UTF-8"); // 统一 UTF-8 编码，避免中文乱码

            // -------------------------- 步骤4：AES-256-GCM 加密实际数据 --------------------------
            System.out.println("\n===== 步骤4：AES-256-GCM 加密数据 =====");
            byte[] encryptedData = aes256GcmEncrypt(dataBytes, aesKey);
            System.out.println("加密后数据（Base64）：" + Base64.getEncoder().encodeToString(encryptedData));

            // -------------------------- 步骤5：ECIES 加密 AES 密钥 --------------------------
            System.out.println("\n===== 步骤5：ECIES 加密 AES 密钥 =====");
            IESParameterSpec iesParams = createIesParameterSpec();
            byte[] encryptedAesKey = eciesEncrypt(aesKey.getEncoded(), eccPublicKey, iesParams);
            System.out.println("加密后 AES 密钥（Base64）：" + Base64.getEncoder().encodeToString(encryptedAesKey));

            // -------------------------- 步骤6：接收方解密流程（模拟跨端传输）--------------------------
            System.out.println("\n===== 步骤6：接收方解密流程 =====");
            // 6.1 ECIES 解密 AES 密钥
            byte[] decryptedAesKeyBytes = eciesDecrypt(encryptedAesKey, eccPrivateKey, iesParams);
            SecretKey decryptedAesKey = new SecretKeySpec(decryptedAesKeyBytes, "AES");
            System.out.println("解密后 AES 密钥（Base64）：" + Base64.getEncoder().encodeToString(decryptedAesKey.getEncoded()));

            // 6.2 AES-256-GCM 解密实际数据
            byte[] decryptedData = aes256GcmDecrypt(encryptedData, decryptedAesKey);
            String decryptedStr = new String(decryptedData, "UTF-8");
            System.out.println("解密后数据：" + decryptedStr);
            System.out.println("解密验证结果：" + (originalData.equals(decryptedStr) ? "✅ 数据一致" : "❌ 数据不一致"));

        } catch (Exception e) {
            System.err.println("\n[错误] 执行失败：" + e.getMessage());
            e.printStackTrace();
        }
    }

    /**
     * 1. 获取 secp256r1 标准曲线参数（JDK 原生 ECParameterSpec）
     * 作用：明确曲线方程、基点、阶、余因子，与 C 端 secp256r1 完全对齐
     */
    private static ECParameterSpec getSecp256r1ParameterSpec() throws NoSuchAlgorithmException, InvalidAlgorithmParameterException, NoSuchProviderException, InvalidParameterSpecException {
        AlgorithmParameters ecParams = AlgorithmParameters.getInstance("EC", BC_PROVIDER);
        ecParams.init(new ECGenParameterSpec(ECC_CURVE)); // 加载 secp256r1 预定义参数
        return ecParams.getParameterSpec(ECParameterSpec.class);
    }

    /**
     * 2. 生成 ECC 密钥对（基于标准曲线参数）
     */
    private static KeyPair generateEccKeyPair(ECParameterSpec ecSpec) throws NoSuchAlgorithmException, InvalidAlgorithmParameterException, NoSuchProviderException {
        KeyPairGenerator kpg = KeyPairGenerator.getInstance("EC", BC_PROVIDER);
        kpg.initialize(ecSpec, new SecureRandom()); // 显式传入曲线参数 + 强随机数
        return kpg.generateKeyPair();
    }

    /**
     * 3. 生成 AES-256 密钥（与 C 端 generate_aes_key 对应）
     */
    private static SecretKey generateAes256Key() throws NoSuchAlgorithmException, NoSuchProviderException {
        KeyGenerator kg = KeyGenerator.getInstance("AES", BC_PROVIDER);
        kg.init(AES_KEY_BITS, new SecureRandom()); // 256 位密钥 + 强随机数初始化
        return kg.generateKey();
    }

    /**
     * 4. 构造 IES 参数（与 BouncyCastle IESParameterSpec 定义完全匹配）
     * 对应 C 端配置：shared_info 为空、禁用 MAC、AES-256、禁用点压缩
     */
    private static IESParameterSpec createIesParameterSpec() {
        byte[] derivation = new byte[0];   // C端：shared_info[] = ""（共享信息1）
        byte[] encoding = new byte[0];     // C端：shared_info[] = ""（共享信息2）
        int macKeySize = 0;                // 禁用 MAC（与 C 端 ECIES 配置一致）
        int cipherKeySize = AES_KEY_BITS;  // AES-256 密钥长度（与 C 端对应）
        byte[] nonce = null;               // 不使用额外 nonce
        boolean pointCompression = false;  // 禁用 EC 点压缩（避免 C 端解析错误）


        /**
         * // 3. 显式配置 ECIES 核心参数：SHA-256（KDF）+ AES-256-CBC（对称加密）
         *     BcEciesKeyEncapsulation eciesKem = new BcEciesKeyEncapsulation(
         *         new SHA256Digest(),          // 关键：KDF 哈希算法 = SHA-256（与 C 端对齐）
         *         new AESEngine(),             // 对称加密引擎 = AES
         *         new CBCBlockCipher(new AESEngine()),  // 对称加密模式 = CBC
         *         null,                        // 禁用 MAC（与 C 端一致）
         *         new byte[0],                 // 共享信息1（空，与 C 端 shared_info 对齐）
         *         new byte[0]                  // 共享信息2（空）
         *     );
         * */


        // 使用 6 参数构造函数（完全匹配用户提供的 BouncyCastle IESParameterSpec 定义）
        return new IESParameterSpec(derivation, encoding, macKeySize, cipherKeySize, nonce, pointCompression);
    }

    /**
     * 5. ECIES 加密 AES 密钥（与 C 端 encrypt_aes_key 对应）
     */
    private static byte[] eciesEncrypt(byte[] aesKeyBytes, PublicKey eccPubKey, IESParameterSpec iesParams)
            throws NoSuchAlgorithmException, InvalidKeyException,
            InvalidAlgorithmParameterException, NoSuchPaddingException, NoSuchProviderException, IllegalBlockSizeException, BadPaddingException {
        Cipher cipher = Cipher.getInstance(ECIES_ALGORITHM, BC_PROVIDER);
        cipher.init(Cipher.ENCRYPT_MODE, eccPubKey, iesParams); // 显式传入 IES 参数
        return cipher.doFinal(aesKeyBytes);
    }

    /**
     * 6. ECIES 解密 AES 密钥（与 C 端 decrypt_aes_key 对应）
     */
    private static byte[] eciesDecrypt(byte[] encryptedAesKey, PrivateKey eccPrivKey, IESParameterSpec iesParams)
            throws NoSuchAlgorithmException, NoSuchPaddingException, InvalidKeyException,
            InvalidAlgorithmParameterException, IllegalBlockSizeException, BadPaddingException, NoSuchProviderException {
        Cipher cipher = Cipher.getInstance(ECIES_ALGORITHM, BC_PROVIDER);
        cipher.init(Cipher.DECRYPT_MODE, eccPrivKey, iesParams); // 传入与加密端相同的 IES 参数
        return cipher.doFinal(encryptedAesKey);
    }

    /**
     * 7. AES-256-GCM 加密（格式：IV[12字节] + 密文 + 标签[16字节]，与 C 端一致）
     */
    private static byte[] aes256GcmEncrypt(byte[] data, SecretKey aesKey)
            throws NoSuchAlgorithmException, NoSuchPaddingException, InvalidKeyException,
            InvalidAlgorithmParameterException, IllegalBlockSizeException, BadPaddingException, NoSuchProviderException {
        // 生成 12 字节随机 IV
        byte[] iv = new byte[GCM_IV_BYTES];
        new SecureRandom().nextBytes(iv);

        // 初始化 AES-GCM 加密器
        Cipher cipher = Cipher.getInstance(AES_ALGORITHM, BC_PROVIDER);
        GCMParameterSpec gcmSpec = new GCMParameterSpec(GCM_TAG_BYTES * 8, iv); // 标签长度以“位”为单位
        cipher.init(Cipher.ENCRYPT_MODE, aesKey, gcmSpec);

        // 执行加密（GCM 模式下，doFinal 自动生成认证标签）
        byte[] cipherWithTag = cipher.doFinal(data);

        // 拼接：IV + 密文+标签（C 端需按此格式解析）
        return Arrays.concatenate(iv, cipherWithTag);
    }

    /**
     * 8. AES-256-GCM 解密（解析格式：IV[12字节] + 密文 + 标签[16字节]）
     */
    private static byte[] aes256GcmDecrypt(byte[] encryptedData, SecretKey aesKey)
            throws NoSuchAlgorithmException, NoSuchPaddingException, InvalidKeyException,
            InvalidAlgorithmParameterException, IllegalBlockSizeException, BadPaddingException, NoSuchProviderException {
        // 分离 IV 和“密文+标签”
        byte[] iv = Arrays.copyOfRange(encryptedData, 0, GCM_IV_BYTES);
        byte[] cipherWithTag = Arrays.copyOfRange(encryptedData, GCM_IV_BYTES, encryptedData.length);

        // 初始化解密器（自动验证标签，数据篡改会抛 BadPaddingException）
        Cipher cipher = Cipher.getInstance(AES_ALGORITHM, BC_PROVIDER);
        GCMParameterSpec gcmSpec = new GCMParameterSpec(GCM_TAG_BYTES * 8, iv);
        cipher.init(Cipher.DECRYPT_MODE, aesKey, gcmSpec);

        // 执行解密
        return cipher.doFinal(cipherWithTag);
    }
}