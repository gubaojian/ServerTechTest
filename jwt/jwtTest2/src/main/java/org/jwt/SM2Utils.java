package org.jwt;

import com.alipay.api.AlipayApiException;
import com.alipay.api.internal.util.asymmetric.SM2Encryptor;
import org.bouncycastle.asn1.gm.GMNamedCurves;
import org.bouncycastle.asn1.x9.X9ECParameters;
import org.bouncycastle.crypto.AsymmetricCipherKeyPair;
import org.bouncycastle.crypto.CryptoException;
import org.bouncycastle.crypto.InvalidCipherTextException;
import org.bouncycastle.crypto.engines.SM2Engine;
import org.bouncycastle.crypto.params.*;
import org.bouncycastle.crypto.params.ECDomainParameters;
import org.bouncycastle.crypto.params.ECPrivateKeyParameters;
import org.bouncycastle.crypto.params.ECPublicKeyParameters;
import org.bouncycastle.crypto.params.ParametersWithRandom;
import org.bouncycastle.crypto.signers.SM2Signer;
import org.bouncycastle.jcajce.spec.SM2ParameterSpec;
import org.bouncycastle.jce.provider.BouncyCastleProvider;
import org.bouncycastle.math.ec.ECPoint;
import org.bouncycastle.util.encoders.Hex;

import javax.crypto.BadPaddingException;
import javax.crypto.Cipher;
import javax.crypto.IllegalBlockSizeException;
import javax.crypto.NoSuchPaddingException;
import java.nio.charset.StandardCharsets;
import java.security.*;
import java.security.spec.ECGenParameterSpec;
import java.security.spec.ECPublicKeySpec;
import java.security.spec.InvalidKeySpecException;
import java.security.spec.X509EncodedKeySpec;
import java.util.Base64;

public class SM2Utils {
    // 静态代码块，添加BouncyCastleProvider支持
    static {
        Security.addProvider(new BouncyCastleProvider());
    }

    // 获取SM2曲线参数
    private static X9ECParameters x9ECParameters = GMNamedCurves.getByName("sm2p256v1");
    private static ECDomainParameters domainParameters = new ECDomainParameters(
            x9ECParameters.getCurve(),
            x9ECParameters.getG(),
            x9ECParameters.getN()
    );

    /**
     * 生成SM2密钥对
     *
     * @return 密钥对
     */
    public static KeyPair generateKeyPair() throws NoSuchAlgorithmException, InvalidAlgorithmParameterException {
        // 指定椭圆曲线算法
        KeyPairGenerator keyPairGenerator = KeyPairGenerator.getInstance("SM2");

        // 指定椭圆曲线类型（如secp256r1，即NIST P-256）
        ECGenParameterSpec ecSpec = new ECGenParameterSpec("sm2p256v1");

        // 初始化生成器，指定曲线参数和随机数生成器
        keyPairGenerator.initialize(ecSpec, new SecureRandom());

        // 生成并返回密钥对
        return keyPairGenerator.generateKeyPair();
    }

    /**
     * 将公钥转换为十六进制字符串
     * @param publicKey 公钥
     * @return 十六进制字符串
     */
    public static String publicKeyToHex(PublicKey publicKey) {
        return Hex.toHexString(publicKey.getEncoded());
    }

    /**
     * 将私钥转换为十六进制字符串
     * @param privateKey 私钥
     * @return 十六进制字符串
     */
    public static String privateKeyToHex(PrivateKey privateKey) {
        return Hex.toHexString(privateKey.getEncoded());
    }

    /**
     * 从十六进制字符串恢复公钥
     * @param hex 公钥十六进制字符串
     * @return 公钥对象
     */
    public static ECPublicKeyParameters hexToPublicKey(String hex) {
        byte[] keyBytes = Hex.decode(hex);
        ECPoint point = x9ECParameters.getCurve().decodePoint(keyBytes);
        return new ECPublicKeyParameters(point, domainParameters);
    }

    /**
     * 从十六进制字符串恢复私钥
     * @param hex 私钥十六进制字符串
     * @return 私钥对象
     */
    public static ECPrivateKeyParameters hexToPrivateKey(String hex) {
        byte[] keyBytes = Hex.decode(hex);
        return new ECPrivateKeyParameters(new java.math.BigInteger(1, keyBytes), domainParameters);
    }

    /**
     * SM2加密
     * @param publicKey 公钥
     * @param data 待加密数据
     * @return 加密后的数据（Base64编码）
     */
    public static String encrypt(PublicKey publicKey, byte[] data) throws NoSuchPaddingException, NoSuchAlgorithmException, InvalidKeyException, IllegalBlockSizeException, BadPaddingException {
        Cipher cipher = Cipher.getInstance("SM2");
        cipher.init(Cipher.ENCRYPT_MODE, publicKey);
        byte[] encryptedBytes = cipher.doFinal(data);
        System.out.println(encryptedBytes.length);
        return Base64.getEncoder().encodeToString(encryptedBytes);
    }

    /**
     * SM2解密
     * @param privateKey 私钥
     * @param encryptedData 加密后的数据（Base64编码）
     * @return 解密后的数据
     */
    public static byte[] decrypt(PrivateKey privateKey, String encryptedData) throws NoSuchPaddingException, NoSuchAlgorithmException, InvalidKeyException, IllegalBlockSizeException, BadPaddingException {

            byte[] data = Base64.getDecoder().decode(encryptedData);
            Cipher cipher = Cipher.getInstance("SM2");
            cipher.init(Cipher.DECRYPT_MODE, privateKey);
            byte[] encryptedBytes = cipher.doFinal(data);
            return encryptedBytes;

    }

    /**
     * 签名
     * @param privateKey 私钥
     * @param data 待签名数据
     * @return 签名结果（十六进制）
     */
    public static String sign(PrivateKey privateKey, byte[] data) throws CryptoException, NoSuchAlgorithmException, NoSuchProviderException, InvalidKeyException, SignatureException, InvalidAlgorithmParameterException {
        // 1. 获取标准Signature实例，算法为"SM3withSM2"（SM2签名必须搭配SM3哈希）
        Signature signature = Signature.getInstance("SM3withSM2", "BC");
        // 2. 初始化签名模式：传入私钥和随机数（SM2签名需随机性）
        signature.initSign(privateKey, new SecureRandom());
        signature.setParameter(new SM2ParameterSpec(USER_ID.getBytes()));
        // 3. 更新待签名数据
        signature.update(data);
        // 4. 生成签名字节数组，转为十六进制返回
        byte[] signBytes = signature.sign();
        return Hex.toHexString(signBytes);
    }

    /**
     * 验证签名
     * @param publicKey 公钥
     * @param data 原始数据
     * @param signatureHex 签名（十六进制）
     * @return 验证结果
     */
    public static boolean verify(PublicKey publicKey, byte[] data, String signatureHex) throws NoSuchAlgorithmException, NoSuchProviderException, InvalidKeyException, SignatureException, InvalidAlgorithmParameterException {
        // 1. 获取标准Signature实例，算法与签名一致
        Signature signature = Signature.getInstance("SM3withSM2", "BC");
        // 2. 初始化为验证模式：传入公钥
        signature.initVerify(publicKey);
        signature.setParameter(new SM2ParameterSpec(USER_ID.getBytes()));
        // 3. 更新原始数据（需与签名时的原始数据完全一致）
        signature.update(data);
        // 4. 解码十六进制签名，验证结果
        byte[] signBytes = Hex.decode(signatureHex);
        return signature.verify(signBytes);
    }

    // 测试方法
    public static void main(String[] args) throws AlipayApiException {
        try {
            // 生成密钥对
            KeyPair keyPair = generateKeyPair();
            PrivateKey privateKey =  keyPair.getPrivate();
            PublicKey publicKey =  keyPair.getPublic();

            // 打印密钥
            System.out.println("私钥: " + privateKeyToHex(privateKey));
            System.out.println("公钥: " + publicKeyToHex(publicKey));

            // 原始数据
            String originalData = "hello world sm2";
            System.out.println("原始数据: " + originalData);

            // 加密
            String encryptedData = encrypt(publicKey, originalData.getBytes("UTF-8"));
            System.out.println("加密后: " + encryptedData);


            // 解密
            byte[] decryptedData = decrypt(privateKey, encryptedData);
            System.out.println("解密后: " + new String(decryptedData, "UTF-8"));

            // 签名
            String signature = sign(privateKey, originalData.getBytes("UTF-8"));
            System.out.println("签名: " + signature);

            // 验证签名
            boolean verifyResult = verify(publicKey, originalData.getBytes("UTF-8"), signature);
            System.out.println("签名验证结果: " + verifyResult);

        } catch (Exception e) {
            e.printStackTrace();
        }

        {
            SM2Encryptor sm2 = new SM2Encryptor();
            String privateKey = "MIGHAgEAMBMGByqGSM49AgEGCCqBHM9VAYItBG0wawIBAQQgTKwFtNzANfbXyLjMZsfQVKdRJniH29LrYlsMPpP4JQGhRANCAASfROCTf3PbQ4sHNMwOAyMUYrQ5hGdXQWcy9Gp4qjF/77XgiAVq8jUNk/7+YeH/fLf7epFX63qNDEy1HcuPSV9l";
            String publicKey = "MFkwEwYHKoZIzj0CAQYIKoEcz1UBgi0DQgAEn0Tgk39z20OLBzTMDgMjFGK0OYRnV0FnMvRqeKoxf++14IgFavI1DZP+/mHh/3y3+3qRV+t6jQxMtR3Lj0lfZQ==";
            String originalData = "hello world sm2";
            String sign = sm2.sign(originalData, "UTF-8", privateKey);

            System.out.println(sign);
            System.out.println("验证结果");

            System.out.println(sm2.verify(originalData, "UTF-8", publicKey, sign));

        }
    }

    private static final String USER_ID = "1234567812345678";
}
