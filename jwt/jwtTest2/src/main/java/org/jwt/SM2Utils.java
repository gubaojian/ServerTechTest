package org.jwt;

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
    public static String sign(ECPrivateKeyParameters privateKey, byte[] data) throws CryptoException {
        SM2Signer signer = new SM2Signer();
        signer.init(true, new ParametersWithRandom(privateKey, new SecureRandom()));
        signer.update(data, 0, data.length);
        byte[] signature = signer.generateSignature();
        return Hex.toHexString(signature);
    }

    /**
     * 验证签名
     * @param publicKey 公钥
     * @param data 原始数据
     * @param signature 签名（十六进制）
     * @return 验证结果
     */
    public static boolean verify(ECPublicKeyParameters publicKey, byte[] data, String signature) {
        SM2Signer signer = new SM2Signer();
        signer.init(false, publicKey);
        signer.update(data, 0, data.length);
        return signer.verifySignature(Hex.decode(signature));
    }

    // 测试方法
    public static void main(String[] args) {
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
            /**
            // 签名
            String signature = sign(privateKey, originalData.getBytes("UTF-8"));
            System.out.println("签名: " + signature);

            // 验证签名
            boolean verifyResult = verify(publicKey, originalData.getBytes("UTF-8"), signature);
            System.out.println("签名验证结果: " + verifyResult);*/

        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
