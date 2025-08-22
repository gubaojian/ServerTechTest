package org.jwt;


import org.bouncycastle.jce.provider.BouncyCastleProvider;
import java.security.*;
import java.security.spec.InvalidKeySpecException;
import java.security.spec.NamedParameterSpec;
import java.security.spec.PKCS8EncodedKeySpec;
import java.security.spec.X509EncodedKeySpec;
import java.util.Base64;

public class EdDSASignatureExample {
    // 支持的EdDSA曲线
    public static final String ED25519_CURVE = "Ed25519";
    public static final String ED448_CURVE = "Ed448";
    private static final String SIGNATURE_ALGORITHM = "EdDSA"; // EdDSA签名算法
    private static final String BC_PROVIDER = "BC";

    static {
        // 注册BouncyCastle提供者
        if (Security.getProvider(BC_PROVIDER) == null) {
            Security.addProvider(new BouncyCastleProvider());
        }
    }

    /**
     * 生成EdDSA密钥对（兼容新老版本BouncyCastle）
     */
    public static KeyPair generateEdDSAKeyPair(String curve) throws Exception {
        KeyPairGenerator keyPairGenerator;
        try {
            // 尝试标准方式（适用于BouncyCastle 1.70+）
            keyPairGenerator = KeyPairGenerator.getInstance("EdDSA", BC_PROVIDER);
            // 使用NamedParameterSpec指定曲线（替代EdDSAParameterSpec）
            keyPairGenerator.initialize(new NamedParameterSpec(curve), new SecureRandom());
        } catch (Exception e) {
            throw  new RuntimeException(e);
        }
        return keyPairGenerator.generateKeyPair();
    }

    /**
     * 使用EdDSA私钥签名
     */
    public static byte[] sign(byte[] data, PrivateKey privateKey) throws Exception {
        Signature signature = Signature.getInstance(SIGNATURE_ALGORITHM, BC_PROVIDER);
        signature.initSign(privateKey);
        signature.update(data);
        return signature.sign();
    }

    /**
     * 使用EdDSA公钥验证签名
     */
    public static boolean verifySignature(byte[] data, byte[] signature, PublicKey publicKey) throws Exception {
        Signature verifier = Signature.getInstance(SIGNATURE_ALGORITHM, BC_PROVIDER);
        verifier.initVerify(publicKey);
        verifier.update(data);
        return verifier.verify(signature);
    }

    /**
     * 从Base64恢复公钥
     */
    public static PublicKey restorePublicKey(String base64PublicKey) throws NoSuchAlgorithmException, InvalidKeySpecException, NoSuchProviderException {
        byte[] keyBytes = Base64.getDecoder().decode(base64PublicKey);
        X509EncodedKeySpec keySpec = new X509EncodedKeySpec(keyBytes);
        KeyFactory keyFactory = KeyFactory.getInstance("EdDSA", BC_PROVIDER);
        return keyFactory.generatePublic(keySpec);
    }

    /**
     * 从Base64恢复私钥
     */
    public static PrivateKey restorePrivateKey(String base64PrivateKey) throws NoSuchAlgorithmException, InvalidKeySpecException, NoSuchProviderException {
        byte[] keyBytes = Base64.getDecoder().decode(base64PrivateKey);
        PKCS8EncodedKeySpec keySpec = new PKCS8EncodedKeySpec(keyBytes);
        KeyFactory keyFactory = KeyFactory.getInstance("EdDSA", BC_PROVIDER);
        return keyFactory.generatePrivate(keySpec);
    }

    public static void main(String[] args) {
        try {
            // 测试Ed25519
            System.out.println("=== 测试 Ed25519 签名 ===");
            testEdDSA(ED25519_CURVE);

            // 测试Ed448
            System.out.println("\n=== 测试 Ed448 签名 ===");
            testEdDSA(ED448_CURVE);

        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private static void testEdDSA(String curve) throws Exception {
        KeyPair keyPair = generateEdDSAKeyPair(curve);
        PublicKey publicKey = keyPair.getPublic();
        PrivateKey privateKey = keyPair.getPrivate();

        System.out.println("公钥（Base64）：" + Base64.getEncoder().encodeToString(publicKey.getEncoded()));
        System.out.println("私钥（Base64）：" + Base64.getEncoder().encodeToString(privateKey.getEncoded()));

        {
            StringBuilder sb = new StringBuilder();
            sb.append("std::string publicKey =\"");
            sb.append(Base64.getEncoder().encodeToString(publicKey.getEncoded()));
            sb.append("\";\n\n");

            sb.append("std::string privateKey = \"");
            sb.append(Base64.getEncoder().encodeToString(privateKey.getEncoded()));
            sb.append("\";\n\n");
            System.out.println(sb.toString());
        }

        String originalData = "Hello EdDSA Signature with " + curve;
        System.out.println("\n原始数据：" + originalData);
        byte[] data = originalData.getBytes("UTF-8");

        byte[] signature = sign(data, privateKey);
        System.out.println("签名结果（长度）：" + signature.length);
        System.out.println("签名结果（Base64）：" + Base64.getEncoder().encodeToString(signature));

        boolean isValid = verifySignature(data, signature, publicKey);
        System.out.println("签名验证结果：" + (isValid ? "有效" : "无效"));

        // 测试篡改数据
        String tamperedData = originalData + " [Tampered]";
        boolean isTamperedValid = verifySignature(tamperedData.getBytes("UTF-8"), signature, publicKey);
        System.out.println("篡改数据验证结果：" + (isTamperedValid ? "有效" : "无效"));

        // 测试恢复的密钥
        boolean isRestoredValid = verifySignature(data, signature,
                restorePublicKey(Base64.getEncoder().encodeToString(publicKey.getEncoded())));
        System.out.println("恢复公钥验证结果：" + (isRestoredValid ? "有效" : "无效"));
    }
}


