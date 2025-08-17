package org.jwt;

import org.bouncycastle.crypto.digests.CSHAKEDigest;
import org.bouncycastle.crypto.digests.SHAKEDigest;
import org.bouncycastle.jce.provider.BouncyCastleProvider;
import org.bouncycastle.util.encoders.Hex;

import java.nio.charset.StandardCharsets;
import java.security.Security;

// https://developer.android.com/reference/javax/crypto/Cipher
//
public class CplusCShakeTest {

    public static void main(String[] args) throws Exception {
        Security.addProvider(new BouncyCastleProvider());
        String n =  "";
        String c = "hello";
        {
            String plainText = "hello world rsa";
            System.out.println("------------- cshake256Hex ----------------");
            System.out.println(shake256Hex(plainText, n, c,64));
        }

        {
            String plainText = "hello world rsa";
            System.out.println("------------- cshake128Hex ----------------");
            System.out.println(shake128Hex(plainText, n, c,32));
        }

        {
            String plainText = "hello world rsa";
            System.out.println("------------- cshake128Hex ----------------");
            System.out.println(shake128Hex(plainText, n, c, 256));
        }
    }

    public static String shake256Hex(String input, String n, String c, int outputLength) {
        // 创建 SHAKE256 实例
        CSHAKEDigest digest = new CSHAKEDigest(256, n.getBytes(StandardCharsets.UTF_8), c.getBytes(StandardCharsets.UTF_8)); // 256 表示 SHAKE256
        // 处理输入数据
        byte[] inputBytes = input.getBytes(StandardCharsets.UTF_8);
        digest.update(inputBytes, 0, inputBytes.length);

        // 生成指定长度的哈希值
        byte[] result = new byte[outputLength];
        digest.doFinal(result, 0, result.length);

        // 转换为十六进制字符串
        return Hex.toHexString(result);
    }


    public static String shake128Hex(String input, String n, String c, int outputLength) {
        // 创建 SHAKE256 实例
        CSHAKEDigest digest = new CSHAKEDigest(128, n.getBytes(StandardCharsets.UTF_8), c.getBytes(StandardCharsets.UTF_8)); // 256 表示 SHAKE256
        // 处理输入数据
        byte[] inputBytes = input.getBytes(StandardCharsets.UTF_8);
        digest.update(inputBytes, 0, inputBytes.length);

        // 生成指定长度的哈希值
        byte[] result = new byte[outputLength];
        digest.doFinal(result, 0, result.length);

        // 转换为十六进制字符串
        return Hex.toHexString(result);
    }
}
