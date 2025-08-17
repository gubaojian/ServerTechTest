package org.jwt;

import org.apache.commons.codec.digest.DigestUtils;

// https://developer.android.com/reference/javax/crypto/Cipher
//
public class CplusDigestTest {

    public static void main(String[] args) throws Exception {
        {
            String plainText = "hello world rsa";
            System.out.println("------------- MD5 ----------------");
            System.out.println(DigestUtils.md5Hex(plainText));
        }

        {
            String plainText = "hello world rsa";
            System.out.println("------------- SHA1 ----------------");
            System.out.println(DigestUtils.sha1Hex(plainText));
        }

        {
            String plainText = "hello world rsa";
            System.out.println("------------- sha256 ----------------");
            System.out.println(DigestUtils.sha256Hex(plainText));
        }

        {
            String plainText = "hello world rsa";
            System.out.println("------------- sha384 ----------------");
            System.out.println(DigestUtils.sha384Hex(plainText));
        }

        {
            String plainText = "hello world rsa";
            System.out.println("------------- sha512 ----------------");
            System.out.println(DigestUtils.sha512Hex(plainText));
        }

        {
            String plainText = "hello world rsa";
            System.out.println("------------- sha3_256  ----------------");
            System.out.println(DigestUtils.sha3_256Hex(plainText));
        }

        {
            String plainText = "hello world rsa";
            System.out.println("------------- sha3_384 ----------------");
            System.out.println(DigestUtils.sha3_384Hex(plainText));
        }

        {
            String plainText = "hello world rsa";
            System.out.println("------------- sha3-512 ----------------");
            System.out.println(DigestUtils.sha3_512Hex(plainText));
        }

        {
            String plainText = "hello world rsa";
            System.out.println("------------- sha512_224 ----------------");
            System.out.println(DigestUtils.sha512_224Hex(plainText));
        }

        {

            String plainText = "hello world rsa";
            System.out.println("------------- sha512_256 ----------------");
            System.out.println(DigestUtils.sha512_256Hex(plainText));
        }





    }
}
