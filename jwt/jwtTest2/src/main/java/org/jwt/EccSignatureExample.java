package org.jwt;


import org.bouncycastle.jce.provider.BouncyCastleProvider;

import java.security.*;
import java.security.spec.ECGenParameterSpec;
import java.security.spec.ECParameterSpec;
import java.util.Base64;

public class EccSignatureExample {
    // 配置：使用secp256r1椭圆曲线（NIST P-256）
    private static final String ECC_CURVE = "secp256r1";
    private static final String SIGNATURE_ALGORITHM = "SHA256withECDSA"; // ECC签名算法
    private static final String BC_PROVIDER = "BC";

    static {
        // 注册BouncyCastle提供者（ECC支持依赖此库）
        if (Security.getProvider(BC_PROVIDER) == null) {
            Security.addProvider(new BouncyCastleProvider());
        }
    }

    /**
     * 生成ECC密钥对（私钥用于签名，公钥用于验证）
     */
    public static KeyPair generateEccKeyPair() throws Exception {
        // 获取secp256r1曲线参数
        AlgorithmParameters ecSpec = AlgorithmParameters.getInstance("EC");
        ecSpec.init(new ECGenParameterSpec(ECC_CURVE));
        ECParameterSpec ecParameterSpec = ecSpec.getParameterSpec(ECParameterSpec.class);

        // 生成密钥对
        KeyPairGenerator keyPairGenerator = KeyPairGenerator.getInstance("EC", BC_PROVIDER);
        keyPairGenerator.initialize(ecParameterSpec, new SecureRandom());
        return keyPairGenerator.generateKeyPair();
    }

    /**
     * 使用ECC私钥对数据进行签名
     */
    public static byte[] sign(byte[] data, PrivateKey privateKey) throws Exception {
        // 获取签名实例
        Signature signature = Signature.getInstance(SIGNATURE_ALGORITHM, BC_PROVIDER);
        // 初始化签名，传入私钥
        signature.initSign(privateKey);
        // 更新要签名的数据
        signature.update(data);
        // 执行签名
        return signature.sign();
    }

    /**
     * 使用ECC公钥验证签名
     */
    public static boolean verifySignature(byte[] data, byte[] signature, PublicKey publicKey) throws Exception {
        // 获取签名实例
        Signature verifier = Signature.getInstance(SIGNATURE_ALGORITHM, BC_PROVIDER);
        // 初始验证器，传入公钥
        verifier.initVerify(publicKey);
        // 更新要验证的数据
        verifier.update(data);
        // 验证签名
        return verifier.verify(signature);
    }

    public static void main(String[] args) {
        try {
            // 1. 生成ECC密钥对
            KeyPair keyPair = generateEccKeyPair();
            PublicKey publicKey = keyPair.getPublic();
            PrivateKey privateKey = keyPair.getPrivate();

            System.out.println("ECC公钥（Base64）：" + Base64.getEncoder().encodeToString(publicKey.getEncoded()));
            System.out.println("ECC私钥（Base64）：" + Base64.getEncoder().encodeToString(privateKey.getEncoded()));

            // 2. 待签名的原始数据
            String originalData = "Hello, ECC Signature!";
            System.out.println("\n原始数据：" + originalData);
            byte[] data = originalData.getBytes("UTF-8");

            // 3. 用私钥对数据进行签名
            byte[] signature = sign(data, privateKey);
            System.out.println("签名结果（Base64）：" + Base64.getEncoder().encodeToString(signature));

            // 4. 用公钥验证签名
            boolean isValid = verifySignature(data, signature, publicKey);
            System.out.println("签名验证结果：" + (isValid ? "有效" : "无效"));

            // 测试篡改数据后的验证结果
            String tamperedData = "Hello, ECC Signature! Tampered";
            byte[] tamperedBytes = tamperedData.getBytes("UTF-8");
            boolean isTamperedValid = verifySignature(tamperedBytes, signature, publicKey);
            System.out.println("篡改数据后的签名验证结果：" + (isTamperedValid ? "有效" : "无效"));

        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
