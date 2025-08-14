package org.jwt;

import org.bouncycastle.jce.provider.BouncyCastleProvider;

import java.security.Security;
import java.util.Base64;

// https://developer.android.com/reference/javax/crypto/Cipher
//
public class CplusAesGCMSIVTest {

    public static void main(String[] args) throws Exception {
        Security.addProvider(new BouncyCastleProvider());
        {
            String key128 = "E7BpQCZlD1hNJYeDUk4RBw==";
            String plainText = "hello world rsa";
            System.out.println("-------------GCM-SIV----------------");
            String encryptText = AesEncryptUtil.encryptGCMSIV(plainText, key128);
            System.out.println(encryptText);
            System.out.println(AesEncryptUtil.decryptGCMSIV(encryptText, key128));
        }

        {
            String key128 = "E7BpQCZlD1hNJYeDUk4RBw==";
            String plainText = "hello world rsa";
            System.out.println("-------------CCM----------------");
            String encryptText = AesEncryptUtil.encryptCCM(plainText, key128);
            System.out.println(encryptText);
            System.out.println(AesEncryptUtil.decryptCCM(encryptText, key128));
        }

        {
            String key128 = "E7BpQCZlD1hNJYeDUk4RBw==";
            String plainText = "hello world rsa";
            System.out.println("-------------GCM----------------");
            String encryptText = AesEncryptUtil.encryptGCM(plainText, key128);
            System.out.println(encryptText);
            System.out.println(AesEncryptUtil.decryptGCM(encryptText, key128));
        }


        {
            String key128 = "E7BpQCZlD1hNJYeDUk4RBw==";
            byte[] decode = Base64.getDecoder().decode(key128);
            byte[] sivKey = new byte [16*2];
            System.arraycopy(decode, 0, sivKey, 0, decode.length);
            System.arraycopy(decode, 0, sivKey, decode.length, decode.length);
            String key = Base64.getEncoder().encodeToString(sivKey);
            String plainText = "hello world rsa";
            System.out.println("------------- SIV ----------------");
            System.out.println("key: " + key);
            String encryptText = AesEncryptUtil.encryptSIV(plainText, key);
            System.out.println(encryptText);
            System.out.println(plainText.length());
            System.out.println(AesEncryptUtil.decryptSIV(encryptText, key));
        }


    }
}
