package org.jwt;

import org.bouncycastle.jce.provider.BouncyCastleProvider;

import java.security.Security;

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
            System.out.println("-------------GCM----------------");
            String encryptText = AesEncryptUtil.encryptGCM(plainText, key128);
            System.out.println(encryptText);
            System.out.println(AesEncryptUtil.decryptGCM(encryptText, key128));
        }

    }
}
