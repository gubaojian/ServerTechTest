package org.jwt;
import org.bouncycastle.crypto.digests.SHAKEDigest;
import org.bouncycastle.jce.provider.BouncyCastleProvider;
import org.bouncycastle.util.encoders.Hex;
import java.nio.charset.StandardCharsets;
import java.security.Security;

public class ShakeHashTest {

    public static String shake128Hex(String input, int outputLength) {
        if (outputLength <= 0) {
            throw new IllegalArgumentException("输出长度必须大于 0");
        }
        SHAKEDigest digest = new SHAKEDigest(128);
        byte[] inputBytes = input.getBytes(StandardCharsets.UTF_8);
        digest.update(inputBytes, 0, inputBytes.length);

        byte[] result = new byte[outputLength];
        digest.doFinal(result, 0);

        return Hex.toHexString(result);
    }

    public static void main(String[] args) {
        Security.addProvider(new BouncyCastleProvider());

        // 确保删除或重命名了冲突的SHAKEDigest.java文件
        String input = "test-long-shake128";

        int len128 = 128;
        int len256 = 256;

        String hash128 = shake128Hex(input, len128);
        String hash256 = shake128Hex(input, len256);

        System.out.println("SHAKE128 (" + len128 + "字节) 长度: " + hash128);
        System.out.println("SHAKE128 (" + len128 + "字节): " + hash128.substring(0, 64) + "...");

        System.out.println("\nSHAKE128 (" + len256 + "字节) 长度: " + hash256);
        System.out.println("SHAKE128 (" + len256 + "字节): " + hash256.substring(0, 64) + "...");

        System.out.println("\nSHAKE128 (" + len256 + "字节) 尾部16字节: " +
                hash256.substring(hash256.length() - 32));
    }
}

