package org.jwt;

import org.bouncycastle.crypto.digests.SHA1Digest;
import org.bouncycastle.crypto.engines.AESEngine;
import org.bouncycastle.crypto.modes.CBCBlockCipher;
import org.bouncycastle.jce.provider.BouncyCastleProvider;
import org.bouncycastle.jce.spec.IESParameterSpec;
import org.bouncycastle.util.Arrays;

import javax.crypto.Cipher;
import javax.crypto.spec.OAEPParameterSpec;
import java.security.*;
import java.security.spec.ECGenParameterSpec;
import java.security.spec.ECParameterSpec;
import java.security.spec.MGF1ParameterSpec;
import java.util.Base64;

public class EccDecryptionExample {
    // 配置：使用secp256r1椭圆曲线（NIST P-256）
    private static final String ECC_CURVE = "secp256r1";
    //private static final String ECC_ALGORITHM = "ECIES"; // ECC加密算法（基础非对称加密）
    private static final String ECC_ALGORITHM = "ECIESwithSHA256"; // ECC加密算法（基础非对称加密）

    //private static final String ECC_ALGORITHM = "ECIESwithSHA3_256"; // ECC加密算法（基础非对称加密）




    private static final String BC_PROVIDER = "BC";

    private static final int SYMMETRIC_KEY_SIZE = 128; // 对应OpenSSL的AES-128
    private static final byte[] SHARED_INFO = new byte[0]; // 对应OpenSSL的shared_info[] = ""
    // 2.1 KDF哈希算法：与OpenSSL的KDF_HASH（EVP_sha1()）对应
    private static final SHA1Digest KDF_DIGEST = new SHA1Digest();
    // 2.2 对称加密算法：与OpenSSL的SYMMETRIC_CIPHER（AES-128-CBC）对应
    private static final AESEngine SYMMETRIC_ENGINE = new AESEngine();
    private static final CBCBlockCipher SYMMETRIC_MODE = new CBCBlockCipher(SYMMETRIC_ENGINE);


    static {
        // 注册BouncyCastle提供者（ECC支持依赖此库）
        if (Security.getProvider(BC_PROVIDER) == null) {
            Security.addProvider(new BouncyCastleProvider());
        }
    }

    /**
     * 生成ECC密钥对（公钥用于加密，私钥用于解密）
     */
    public static KeyPair generateEccKeyPair() throws Exception {
        // 获取secp256r1曲线参数
        AlgorithmParameters ecSpec = AlgorithmParameters.getInstance("EC");
        ecSpec.init(new ECGenParameterSpec(ECC_CURVE));
        ecSpec.getParameterSpec(ECParameterSpec.class);
        ECParameterSpec ecParameterSpec = ecSpec.getParameterSpec(ECParameterSpec.class);

        // 生成密钥对
        KeyPairGenerator keyPairGenerator = KeyPairGenerator.getInstance("EC", BC_PROVIDER);
        keyPairGenerator.initialize(ecParameterSpec, new SecureRandom());
        return keyPairGenerator.generateKeyPair();
    }

    /**
     * 使用ECC公钥加密数据（小数据，非对称加密不适合大文件）
     */
    public static byte[] eccEncrypt(byte[] data, PublicKey publicKey) throws Exception {
        Cipher cipher = Cipher.getInstance(ECC_ALGORITHM, BC_PROVIDER);
        cipher.init(Cipher.ENCRYPT_MODE, publicKey);
        {
            AlgorithmParameters params = cipher.getParameters();
              System.out.println("哈希函数: " + params );
        }
        return cipher.doFinal(data);
    }

    /**
     * 使用ECC私钥解密数据（核心解密方法）
     */
    public static byte[] eccDecrypt(byte[] encryptedData, PrivateKey privateKey) throws Exception {
        Cipher cipher = Cipher.getInstance(ECC_ALGORITHM, BC_PROVIDER);
        // 初始化解密模式，传入私钥
        cipher.init(Cipher.DECRYPT_MODE, privateKey);
        // 执行解密
        return cipher.doFinal(encryptedData);
    }

    /**
     * 核心：显式创建与OpenSSL对齐的IES参数
     * 对应OpenSSL的EVP_PKEY_CTX_set_ecies_kdf_md和EVP_PKEY_CTX_set_ecies_cipher
     */
    private static IESParameterSpec createIesParams() {
        // 配置与OpenSSL完全一致的参数：
        // 1. 共享信息（与OpenSSL的shared_info对应）
        // 2. 对称密钥长度（与AES-128对应）
        // 3. 禁用MAC（与OpenSSL默认行为一致）
        return new IESParameterSpec(
                SHARED_INFO,          // 共享信息1（与OpenSSL的shared_info一致）
                SHARED_INFO,          // 共享信息2（同上）
                0,                    // MAC密钥长度（0表示禁用MAC，与OpenSSL一致）
                SYMMETRIC_KEY_SIZE,   // 对称加密密钥长度（128位，与AES-128对应）
                null,                 // 不使用nonce
                false                 // 禁用点压缩（避免OpenSSL解析问题）
        );
    }

    public static void main(String[] args) {
        try {
            // 1. 生成ECC密钥对
            KeyPair keyPair = generateEccKeyPair();
            PublicKey publicKey = keyPair.getPublic();
            PrivateKey privateKey = keyPair.getPrivate();

            System.out.println("ECC公钥（Base64）：" + Base64.getEncoder().encodeToString(publicKey.getEncoded()));
            System.out.println("ECC私钥（Base64）：" + Base64.getEncoder().encodeToString(privateKey.getEncoded()));

            // 2. 待解密的原始数据（通常是小数据，如对称密钥）
            String originalData = "Hello, ECC Decryption!";
            System.out.println("\n原始数据：" + originalData);

            // 3. 用公钥加密数据（模拟发送方操作）
            byte[] encryptedData = eccEncrypt(originalData.getBytes("UTF-8"), publicKey);
            System.out.println("加密后数据（Base64）：" + Base64.getEncoder().encodeToString(encryptedData));

            // 4. 用私钥解密数据（核心解密步骤）
            byte[] decryptedData = eccDecrypt(encryptedData, privateKey);
            String decryptedStr = new String(decryptedData, "UTF-8");
            System.out.println("解密后数据：" + decryptedStr);

            // 验证解密结果
            System.out.println("解密验证：" + Arrays.areEqual(originalData.getBytes(), decryptedData));

        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}

