package org.jwt;

import java.security.KeyPair;

// https://developer.android.com/reference/javax/crypto/Cipher
//
public class CplusAesTest {

    public static void main(String[] args) throws Exception {
        {
            String plainText = "hello world rsa";
            System.out.println(AesEncryptUtil.encrypt(plainText));
            System.out.println(AesEncryptUtil.generateKey(128));
        }
        {
            String key128 = "E7BpQCZlD1hNJYeDUk4RBw==";
            String plainText = "hello world rsa";
            String encryptText = AesEncryptUtil.encrypt(plainText, key128);
            System.out.println(encryptText);
            System.out.println(AesEncryptUtil.decrypt(encryptText, key128));
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
            String plainText = "hello world rsa";
            System.out.println("-------------ecb----------------");
            String encryptText = AesEncryptUtil.encryptECB(plainText, key128);
            System.out.println(encryptText);
            System.out.println(AesEncryptUtil.decryptECB(encryptText, key128));
        }

        {
            String key128 = "E7BpQCZlD1hNJYeDUk4RBw==";
            String plainText = "hello world rsa";
            System.out.println("-------------CBC ----------------");
            String encryptText = AesEncryptUtil.encryptCBC(plainText, key128);
            System.out.println(encryptText);
            System.out.println(AesEncryptUtil.decryptCBC(encryptText, key128));
        }

        {
            String key128 = "E7BpQCZlD1hNJYeDUk4RBw==";
            String plainText = "hello world rsa";
            System.out.println("-------------CFB ----------------");
            String encryptText = AesEncryptUtil.encryptCFB(plainText, key128);
            System.out.println(encryptText);
            System.out.println(plainText.length());
            System.out.println(AesEncryptUtil.decryptCFB(encryptText, key128));
        }

        {
            String key128 = "E7BpQCZlD1hNJYeDUk4RBw==";
            String plainText = "hello world rsa CTS CTS CTS"; //要求长度大于16
            System.out.println("------------- CTS ----------------");
            String encryptText = AesEncryptUtil.encryptCTS(plainText, key128);
            System.out.println(encryptText);
            System.out.println(plainText.length());
            System.out.println(AesEncryptUtil.decryptCTS(encryptText, key128));
        }

        {
            String key128 = "E7BpQCZlD1hNJYeDUk4RBw==";
            String plainText = "hello world rsa";
            System.out.println("------------- OFB ----------------");
            String encryptText = AesEncryptUtil.encryptOFB(plainText, key128);
            System.out.println(encryptText);
            System.out.println(plainText.length());
            System.out.println(AesEncryptUtil.decryptOFB(encryptText, key128));
        }

        {
            String key128 = "E7BpQCZlD1hNJYeDUk4RBw==";
            String plainText = "hello world rsa";
            System.out.println("------------- CTR ----------------");
            String encryptText = AesEncryptUtil.encryptCTR(plainText, key128);
            System.out.println(encryptText);
            System.out.println(plainText.length());
            System.out.println(AesEncryptUtil.decryptCTR(encryptText, key128));
        }



        {
            String key128 = "E7BpQCZlD1hNJYeDUk4RBw==";
            String plainText = "hello world rsa";
            System.out.println("-------------GCM-AAD----------------");
            String encryptText = AesEncryptUtil.encryptGCMAAD(plainText, key128);
            System.out.println(encryptText);
            System.out.println(AesEncryptUtil.decryptGCMADD(encryptText, key128));
        }

    }
}
