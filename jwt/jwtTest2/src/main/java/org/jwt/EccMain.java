package org.jwt;

import javax.crypto.Cipher;
import java.security.*;
import java.security.spec.ECGenParameterSpec;
import java.util.Base64;

// 引入BouncyCastle相关类
import org.bouncycastle.jce.provider.BouncyCastleProvider;
import org.bouncycastle.util.encoders.Hex;

public class EccMain {
    // 椭圆曲线算法名称
    private static final String ELLIPTIC_CURVE_ALGORITHM = "EC";
    // 使用BouncyCastle支持的椭圆曲线加密算法
    private static final String ECIES_ALGORITHM = "ECIES/NONE/NoPadding";
    // 使用的椭圆曲线
    private static final String CURVE_NAME = "secp256r1"; // 256位的椭圆曲线
    // BouncyCastle提供者名称
    private static final String PROVIDER = null;

    static {
        // 添加BouncyCastle作为安全提供者
        if (Security.getProvider(PROVIDER) == null) {
            Security.addProvider(new BouncyCastleProvider());
            System.out.println("已添加BouncyCastle安全提供者");
        } else {
            System.out.println("BouncyCastle安全提供者已存在");
        }
    }

    public static void main(String[] args) {
        try {
            // 1. 生成ECC密钥对
            KeyPair keyPair = generateKeyPair();
            PublicKey publicKey = keyPair.getPublic();
            PrivateKey privateKey = keyPair.getPrivate();

            System.out.println("生成的公钥: " + Base64.getEncoder().encodeToString(publicKey.getEncoded()));
            System.out.println("生成的私钥: " + Base64.getEncoder().encodeToString(privateKey.getEncoded()));

            // 2. 要加密的数据
            String originalData = "这是一个椭圆曲线加密的示例";
            System.out.println("原始数据: " + originalData);

            // 3. 使用公钥加密
            byte[] encryptedData = encrypt(originalData.getBytes(), publicKey);
            System.out.println("加密后的数据: " + Base64.getEncoder().encodeToString(encryptedData));

            // 4. 使用私钥解密
            byte[] decryptedData = decrypt(encryptedData, privateKey);
            System.out.println("解密后的数据: " + new String(decryptedData));

        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    /**
     * 生成ECC密钥对
     */
    public static KeyPair generateKeyPair() throws Exception {
        // 创建密钥对生成器，指定使用BouncyCastle提供者
        KeyPairGenerator keyPairGenerator = KeyPairGenerator.getInstance(ELLIPTIC_CURVE_ALGORITHM);

        // 指定椭圆曲线
        ECGenParameterSpec ecSpec = new ECGenParameterSpec(CURVE_NAME);
        keyPairGenerator.initialize(ecSpec, new SecureRandom());

        // 生成密钥对
        return keyPairGenerator.generateKeyPair();
    }

    /**
     * 使用公钥加密数据
     */
    public static byte[] encrypt(byte[] data, PublicKey publicKey) throws Exception {
        // 使用BouncyCastle提供者的ECIES算法
        Cipher cipher = Cipher.getInstance(ECIES_ALGORITHM);
        cipher.init(Cipher.ENCRYPT_MODE, publicKey);
        return cipher.doFinal(data);
    }

    /**
     * 使用私钥解密数据
     */
    public static byte[] decrypt(byte[] encryptedData, PrivateKey privateKey) throws Exception {
        // 使用BouncyCastle提供者的ECIES算法
        Cipher cipher = Cipher.getInstance(ECIES_ALGORITHM);
        cipher.init(Cipher.DECRYPT_MODE, privateKey);
        return cipher.doFinal(encryptedData);
    }
}

