package org.jwt;


import org.bouncycastle.jcajce.spec.AEADParameterSpec;
import org.cryptomator.siv.SivMode;

import java.nio.charset.StandardCharsets;
import java.security.SecureRandom;
import java.util.Arrays;
import java.util.Base64;

import javax.crypto.Cipher;
import javax.crypto.KeyGenerator;
import javax.crypto.SecretKey;
import javax.crypto.spec.GCMParameterSpec;
import javax.crypto.spec.IvParameterSpec;
import javax.crypto.spec.SecretKeySpec;

public class AesEncryptUtil {


    private static final String algorithm = "AES";

    /**
     * 32 字节的全小写长数字密钥
     */
    private static final String aesKey = "12345678901234567890123456789012";

    /**
     * 加密方法
     *
     * @param plainText
     * @return
     */
    public static String encrypt(String plainText) {
        try {
            SecretKeySpec secretKeySpec = new SecretKeySpec(aesKey.getBytes(StandardCharsets.UTF_8), algorithm);
            Cipher cipher = Cipher.getInstance("AES/CBC/PKCS5Padding");
            cipher.init(Cipher.ENCRYPT_MODE, secretKeySpec);
            byte[] encryptedBytes = cipher.doFinal(plainText.getBytes(StandardCharsets.UTF_8));
            return Base64.getEncoder().encodeToString(encryptedBytes);
        } catch (Exception e) {
            throw new RuntimeException("AES 加密失败", e);
        }
    }

    /**
     * 解密方法
     *
     * @param encryptedText
     * @return
     */
    public static String decrypt(String encryptedText) {
        try {
            SecretKeySpec secretKeySpec = new SecretKeySpec(aesKey.getBytes(StandardCharsets.UTF_8), algorithm);
            Cipher cipher = Cipher.getInstance("AES/CBC/PKCS5Padding");
            cipher.init(Cipher.DECRYPT_MODE, secretKeySpec);
            byte[] decodedBytes = Base64.getDecoder().decode(encryptedText);
            byte[] decryptedBytes = cipher.doFinal(decodedBytes);
            return new String(decryptedBytes, StandardCharsets.UTF_8);
        } catch (Exception e) {
            throw new RuntimeException("AES 解密失败", e);
        }
    }

    public static String encrypt(String plainText, String base64Key) {
        try {
            SecretKeySpec secretKeySpec = new SecretKeySpec(Base64.getDecoder().decode(base64Key), algorithm);
            Cipher cipher = Cipher.getInstance("AES/CBC/PKCS5Padding");

            byte[] iv = new byte[IV_LENGTH];
            SecureRandom random = new SecureRandom();
            random.nextBytes(iv); // 随机生成IV
            IvParameterSpec ivSpec = new IvParameterSpec(iv);

            cipher.init(Cipher.ENCRYPT_MODE, secretKeySpec, ivSpec);

            byte[] encryptedBytes = cipher.doFinal(plainText.getBytes(StandardCharsets.UTF_8));


            byte[] result = new byte[iv.length + encryptedBytes.length];
            System.arraycopy(iv, 0, result, 0, iv.length);
            System.arraycopy(encryptedBytes, 0, result, iv.length, encryptedBytes.length);

            return Base64.getEncoder().encodeToString(result);
        } catch (Exception e) {
            throw new RuntimeException("AES 加密失败", e);
        }
    }

    /**
     * 解密方法
     *
     * @param encryptedText
     * @return
     */
    public static String decrypt(String encryptedText, String base64Key) {
        try {
            SecretKeySpec secretKeySpec = new SecretKeySpec(Base64.getDecoder().decode(base64Key), algorithm);

            byte[] decoded = Base64.getDecoder().decode(encryptedText);

            // 3. 提取IV（前16字节）和密文（剩余部分）
            byte[] iv = new byte[16];
            System.arraycopy(decoded, 0, iv, 0, iv.length);
            IvParameterSpec ivSpec = new IvParameterSpec(iv);

            Cipher cipher = Cipher.getInstance("AES/CBC/PKCS5Padding");
            cipher.init(Cipher.DECRYPT_MODE, secretKeySpec, ivSpec);

            byte[] encryptedBytes = new byte[decoded.length - IV_LENGTH];
            System.arraycopy(decoded, IV_LENGTH, encryptedBytes, 0, encryptedBytes.length);
            byte[] decryptedBytes = cipher.doFinal(encryptedBytes);
            return new String(decryptedBytes, StandardCharsets.UTF_8);
        } catch (Exception e) {
            throw new RuntimeException("AES 解密失败", e);
        }
    }

    public static final  int IV_LENGTH = 16;


    public static String generateKey(int size) throws Exception {
        // 1. 创建AES密钥生成器
        KeyGenerator keyGenerator = KeyGenerator.getInstance("AES");

        // 2. 设置密钥长度（128、192、256）
        keyGenerator.init(size); // 128位密钥无需额外配置

        // 3. 生成密钥
        SecretKey secretKey = keyGenerator.generateKey();

        // 4. 获取密钥的字节数组，并转为Base64格式（方便存储和传输）
        byte[] keyBytes = secretKey.getEncoded();
        System.out.println("AES密钥（Base64）：" + keyBytes.length);
        String base64Key = Base64.getEncoder().encodeToString(keyBytes);

        System.out.println("AES密钥（Base64）：" + base64Key);
        return  base64Key;
    }



    private static final String algorithm_GCM = "AES/GCM/NoPadding";
    private static final int GCM_IV_LENGTH = 12; // 推荐12字节IV
    private static final int GCM_TAG_LENGTH = 16; // 认证标签长度（128位）


    public static String encryptGCM(String plainText, String base64Key) {
        try {
            SecretKeySpec secretKeySpec = new SecretKeySpec(Base64.getDecoder().decode(base64Key), "AES");
            Cipher cipher = Cipher.getInstance("AES/GCM/NoPadding");

            byte[] iv = new byte[GCM_IV_LENGTH];
            SecureRandom random = new SecureRandom();
            random.nextBytes(iv); // 随机生成IV
            GCMParameterSpec ivSpec = new GCMParameterSpec(128, iv);

            cipher.init(Cipher.ENCRYPT_MODE, secretKeySpec, ivSpec);

            byte[] encryptedBytes = cipher.doFinal(plainText.getBytes(StandardCharsets.UTF_8));


            byte[] result = new byte[iv.length + encryptedBytes.length];
            System.arraycopy(iv, 0, result, 0, iv.length);
            System.arraycopy(encryptedBytes, 0, result, iv.length, encryptedBytes.length);

            return Base64.getEncoder().encodeToString(result);
        } catch (Exception e) {
            throw new RuntimeException("AES 加密失败", e);
        }
    }

    /**
     * 解密方法
     *
     * @param encryptedText
     * @return
     */
    public static String decryptGCM(String encryptedText, String base64Key) {
        try {
            SecretKeySpec secretKeySpec = new SecretKeySpec(Base64.getDecoder().decode(base64Key), "AES");

            byte[] decoded = Base64.getDecoder().decode(encryptedText);

            // 3. 提取IV（前16字节）和密文（剩余部分）
            byte[] iv = new byte[GCM_IV_LENGTH];
            System.arraycopy(decoded, 0, iv, 0, iv.length);
            GCMParameterSpec ivSpec = new GCMParameterSpec(128, iv);

            Cipher cipher = Cipher.getInstance("AES/GCM/NoPadding");
            cipher.init(Cipher.DECRYPT_MODE, secretKeySpec, ivSpec);

            byte[] encryptedBytes = new byte[decoded.length - GCM_IV_LENGTH];
            System.arraycopy(decoded, GCM_IV_LENGTH, encryptedBytes, 0, encryptedBytes.length);

            byte[] decryptedBytes = cipher.doFinal(encryptedBytes);
            return new String(decryptedBytes, StandardCharsets.UTF_8);
        } catch (Exception e) {
            throw new RuntimeException("AES 解密失败", e);
        }
    }


    public static String encryptECB(String plainText, String base64Key) {
        try {
            SecretKeySpec secretKeySpec = new SecretKeySpec(Base64.getDecoder().decode(base64Key), "AES");
            Cipher cipher = Cipher.getInstance("AES/ECB/PKCS5Padding");
            cipher.init(Cipher.ENCRYPT_MODE, secretKeySpec);
            byte[] encryptedBytes = cipher.doFinal(plainText.getBytes(StandardCharsets.UTF_8));
            return Base64.getEncoder().encodeToString(encryptedBytes);
        } catch (Exception e) {
            throw new RuntimeException("AES 加密失败", e);
        }
    }

    /**
     * 解密方法
     *
     * @param encryptedText
     * @return
     */
    public static String decryptECB(String encryptedText, String base64Key) {
        try {
            SecretKeySpec secretKeySpec = new SecretKeySpec(Base64.getDecoder().decode(base64Key), "AES");
            byte[] decoded = Base64.getDecoder().decode(encryptedText);
            Cipher cipher = Cipher.getInstance("AES/ECB/PKCS5Padding");
            cipher.init(Cipher.DECRYPT_MODE, secretKeySpec);
            byte[] decryptedBytes = cipher.doFinal(decoded);
            return new String(decryptedBytes, StandardCharsets.UTF_8);
        } catch (Exception e) {
            throw new RuntimeException("AES 解密失败", e);
        }
    }


    public static String encryptCBC(String plainText, String base64Key) {
        try {
            SecretKeySpec secretKeySpec = new SecretKeySpec(Base64.getDecoder().decode(base64Key), algorithm);
            Cipher cipher = Cipher.getInstance("AES/CBC/PKCS5Padding");

            byte[] iv = new byte[IV_LENGTH];
            SecureRandom random = new SecureRandom();
            random.nextBytes(iv); // 随机生成IV
            IvParameterSpec ivSpec = new IvParameterSpec(iv);

            cipher.init(Cipher.ENCRYPT_MODE, secretKeySpec, ivSpec);

            byte[] encryptedBytes = cipher.doFinal(plainText.getBytes(StandardCharsets.UTF_8));


            byte[] result = new byte[iv.length + encryptedBytes.length];
            System.arraycopy(iv, 0, result, 0, iv.length);
            System.arraycopy(encryptedBytes, 0, result, iv.length, encryptedBytes.length);

            return Base64.getEncoder().encodeToString(result);
        } catch (Exception e) {
            throw new RuntimeException("AES 加密失败", e);
        }
    }

    /**
     * 解密方法
     *
     * @param encryptedText
     * @return
     */
    public static String decryptCBC(String encryptedText, String base64Key) {
        try {
            SecretKeySpec secretKeySpec = new SecretKeySpec(Base64.getDecoder().decode(base64Key), algorithm);

            byte[] decoded = Base64.getDecoder().decode(encryptedText);

            // 3. 提取IV（前16字节）和密文（剩余部分）
            byte[] iv = new byte[16];
            System.arraycopy(decoded, 0, iv, 0, iv.length);
            IvParameterSpec ivSpec = new IvParameterSpec(iv);

            Cipher cipher = Cipher.getInstance("AES/CBC/PKCS5Padding");
            cipher.init(Cipher.DECRYPT_MODE, secretKeySpec, ivSpec);

            byte[] encryptedBytes = new byte[decoded.length - IV_LENGTH];
            System.arraycopy(decoded, IV_LENGTH, encryptedBytes, 0, encryptedBytes.length);
            byte[] decryptedBytes = cipher.doFinal(encryptedBytes);
            return new String(decryptedBytes, StandardCharsets.UTF_8);
        } catch (Exception e) {
            throw new RuntimeException("AES 解密失败", e);
        }
    }

    public static String encryptCFB(String plainText, String base64Key) {
        try {
            SecretKeySpec secretKeySpec = new SecretKeySpec(Base64.getDecoder().decode(base64Key), algorithm);
            Cipher cipher = Cipher.getInstance("AES/CFB/NoPadding");

            byte[] iv = new byte[IV_LENGTH];
            SecureRandom random = new SecureRandom();
            random.nextBytes(iv); // 随机生成IV
            IvParameterSpec ivSpec = new IvParameterSpec(iv);

            cipher.init(Cipher.ENCRYPT_MODE, secretKeySpec, ivSpec);

            byte[] encryptedBytes = cipher.doFinal(plainText.getBytes(StandardCharsets.UTF_8));


            byte[] result = new byte[iv.length + encryptedBytes.length];
            System.arraycopy(iv, 0, result, 0, iv.length);
            System.arraycopy(encryptedBytes, 0, result, iv.length, encryptedBytes.length);

            return Base64.getEncoder().encodeToString(result);
        } catch (Exception e) {
            throw new RuntimeException("AES 加密失败", e);
        }
    }

    /**
     * 解密方法
     *
     * @param encryptedText
     * @return
     */
    public static String decryptCFB(String encryptedText, String base64Key) {
        try {
            SecretKeySpec secretKeySpec = new SecretKeySpec(Base64.getDecoder().decode(base64Key), algorithm);

            byte[] decoded = Base64.getDecoder().decode(encryptedText);

            // 3. 提取IV（前16字节）和密文（剩余部分）
            byte[] iv = new byte[16];
            System.arraycopy(decoded, 0, iv, 0, iv.length);
            IvParameterSpec ivSpec = new IvParameterSpec(iv);

            Cipher cipher = Cipher.getInstance("AES/CFB/NoPadding");
            cipher.init(Cipher.DECRYPT_MODE, secretKeySpec, ivSpec);

            byte[] encryptedBytes = new byte[decoded.length - IV_LENGTH];
            System.arraycopy(decoded, IV_LENGTH, encryptedBytes, 0, encryptedBytes.length);
            byte[] decryptedBytes = cipher.doFinal(encryptedBytes);
            return new String(decryptedBytes, StandardCharsets.UTF_8);
        } catch (Exception e) {
            throw new RuntimeException("AES 解密失败", e);
        }
    }


    public static String encryptCTS(String plainText, String base64Key) {
        try {
            SecretKeySpec secretKeySpec = new SecretKeySpec(Base64.getDecoder().decode(base64Key), algorithm);
            Cipher cipher = Cipher.getInstance("AES/CTS/NoPadding");



            byte[] iv = new byte[IV_LENGTH];
            SecureRandom random = new SecureRandom();
            random.nextBytes(iv); // 随机生成IV
            IvParameterSpec ivSpec = new IvParameterSpec(iv);

            if (plainText.length() < 16) {
                //StringBuilder sb = new StringBuilder();
                //sb.append(plainText);
                //sb.append("ddddd");
                //plainText = sb.toString();
            }

            cipher.init(Cipher.ENCRYPT_MODE, secretKeySpec, ivSpec);

            byte[] encryptedBytes = cipher.doFinal(plainText.getBytes(StandardCharsets.UTF_8));


            byte[] result = new byte[iv.length + encryptedBytes.length];
            System.arraycopy(iv, 0, result, 0, iv.length);
            System.arraycopy(encryptedBytes, 0, result, iv.length, encryptedBytes.length);

            return Base64.getEncoder().encodeToString(result);
        } catch (Exception e) {
            throw new RuntimeException("AES 加密失败", e);
        }
    }

    /**
     * 解密方法
     *
     * @param encryptedText
     * @return
     */
    public static String decryptCTS(String encryptedText, String base64Key) {
        try {
            SecretKeySpec secretKeySpec = new SecretKeySpec(Base64.getDecoder().decode(base64Key), algorithm);

            byte[] decoded = Base64.getDecoder().decode(encryptedText);

            // 3. 提取IV（前16字节）和密文（剩余部分）
            byte[] iv = new byte[16];
            System.arraycopy(decoded, 0, iv, 0, iv.length);
            IvParameterSpec ivSpec = new IvParameterSpec(iv);

            Cipher cipher = Cipher.getInstance("AES/CTS/NoPadding");
            cipher.init(Cipher.DECRYPT_MODE, secretKeySpec, ivSpec);

            byte[] encryptedBytes = new byte[decoded.length - IV_LENGTH];
            System.arraycopy(decoded, IV_LENGTH, encryptedBytes, 0, encryptedBytes.length);
            byte[] decryptedBytes = cipher.doFinal(encryptedBytes);
            return new String(decryptedBytes, StandardCharsets.UTF_8);
        } catch (Exception e) {
            throw new RuntimeException("AES 解密失败", e);
        }
    }


    public static String encryptOFB(String plainText, String base64Key) {
        try {
            SecretKeySpec secretKeySpec = new SecretKeySpec(Base64.getDecoder().decode(base64Key), algorithm);
            Cipher cipher = Cipher.getInstance("AES/OFB/NoPadding");



            byte[] iv = new byte[IV_LENGTH];
            SecureRandom random = new SecureRandom();
            random.nextBytes(iv); // 随机生成IV
            IvParameterSpec ivSpec = new IvParameterSpec(iv);

            cipher.init(Cipher.ENCRYPT_MODE, secretKeySpec, ivSpec);

            byte[] encryptedBytes = cipher.doFinal(plainText.getBytes(StandardCharsets.UTF_8));


            byte[] result = new byte[iv.length + encryptedBytes.length];
            System.arraycopy(iv, 0, result, 0, iv.length);
            System.arraycopy(encryptedBytes, 0, result, iv.length, encryptedBytes.length);

            return Base64.getEncoder().encodeToString(result);
        } catch (Exception e) {
            throw new RuntimeException("AES 加密失败", e);
        }
    }

    /**
     * 解密方法
     *
     * @param encryptedText
     * @return
     */
    public static String decryptOFB(String encryptedText, String base64Key) {
        try {
            SecretKeySpec secretKeySpec = new SecretKeySpec(Base64.getDecoder().decode(base64Key), algorithm);

            byte[] decoded = Base64.getDecoder().decode(encryptedText);

            // 3. 提取IV（前16字节）和密文（剩余部分）
            byte[] iv = new byte[16];
            System.arraycopy(decoded, 0, iv, 0, iv.length);
            IvParameterSpec ivSpec = new IvParameterSpec(iv);

            Cipher cipher = Cipher.getInstance("AES/OFB/NoPadding");
            cipher.init(Cipher.DECRYPT_MODE, secretKeySpec, ivSpec);

            byte[] encryptedBytes = new byte[decoded.length - IV_LENGTH];
            System.arraycopy(decoded, IV_LENGTH, encryptedBytes, 0, encryptedBytes.length);
            byte[] decryptedBytes = cipher.doFinal(encryptedBytes);
            return new String(decryptedBytes, StandardCharsets.UTF_8);
        } catch (Exception e) {
            throw new RuntimeException("AES 解密失败", e);
        }
    }


    public static String encryptCTR(String plainText, String base64Key) {
        try {
            SecretKeySpec secretKeySpec = new SecretKeySpec(Base64.getDecoder().decode(base64Key), algorithm);
            Cipher cipher = Cipher.getInstance("AES/CTR/NoPadding");



            byte[] iv = new byte[IV_LENGTH];
            SecureRandom random = new SecureRandom();
            random.nextBytes(iv); // 随机生成IV
            IvParameterSpec ivSpec = new IvParameterSpec(iv);

            cipher.init(Cipher.ENCRYPT_MODE, secretKeySpec, ivSpec);

            byte[] encryptedBytes = cipher.doFinal(plainText.getBytes(StandardCharsets.UTF_8));


            byte[] result = new byte[iv.length + encryptedBytes.length];
            System.arraycopy(iv, 0, result, 0, iv.length);
            System.arraycopy(encryptedBytes, 0, result, iv.length, encryptedBytes.length);

            return Base64.getEncoder().encodeToString(result);
        } catch (Exception e) {
            throw new RuntimeException("AES 加密失败", e);
        }
    }

    /**
     * 解密方法
     *
     * @param encryptedText
     * @return
     */
    public static String decryptCTR(String encryptedText, String base64Key) {
        try {
            SecretKeySpec secretKeySpec = new SecretKeySpec(Base64.getDecoder().decode(base64Key), algorithm);

            byte[] decoded = Base64.getDecoder().decode(encryptedText);

            // 3. 提取IV（前16字节）和密文（剩余部分）
            byte[] iv = new byte[16];
            System.arraycopy(decoded, 0, iv, 0, iv.length);
            IvParameterSpec ivSpec = new IvParameterSpec(iv);

            Cipher cipher = Cipher.getInstance("AES/OFB/NoPadding");
            cipher.init(Cipher.DECRYPT_MODE, secretKeySpec, ivSpec);

            byte[] encryptedBytes = new byte[decoded.length - IV_LENGTH];
            System.arraycopy(decoded, IV_LENGTH, encryptedBytes, 0, encryptedBytes.length);
            byte[] decryptedBytes = cipher.doFinal(encryptedBytes);
            return new String(decryptedBytes, StandardCharsets.UTF_8);
        } catch (Exception e) {
            throw new RuntimeException("AES 解密失败", e);
        }
    }


    public static String encryptGCMSIV(String plainText, String base64Key) {
        try {
            SecretKeySpec secretKeySpec = new SecretKeySpec(Base64.getDecoder().decode(base64Key), "AES");
            Cipher cipher = Cipher.getInstance("AES/GCM-SIV/NoPadding");

            byte[] iv = new byte[GCM_IV_LENGTH];
            SecureRandom random = new SecureRandom();
            random.nextBytes(iv); // 随机生成IV
            GCMParameterSpec ivSpec = new GCMParameterSpec(128, iv);

            cipher.init(Cipher.ENCRYPT_MODE, secretKeySpec, ivSpec);

            byte[] encryptedBytes = cipher.doFinal(plainText.getBytes(StandardCharsets.UTF_8));


            byte[] result = new byte[iv.length + encryptedBytes.length];
            System.arraycopy(iv, 0, result, 0, iv.length);
            System.arraycopy(encryptedBytes, 0, result, iv.length, encryptedBytes.length);



            return Base64.getEncoder().encodeToString(result);
        } catch (Exception e) {
            throw new RuntimeException("AES 加密失败", e);
        }
    }

    /**
     * 解密方法
     *
     * @param encryptedText
     * @return
     */
    public static String decryptGCMSIV(String encryptedText, String base64Key) {
        try {
            SecretKeySpec secretKeySpec = new SecretKeySpec(Base64.getDecoder().decode(base64Key), "AES");

            byte[] decoded = Base64.getDecoder().decode(encryptedText);

            // 3. 提取IV（前16字节）和密文（剩余部分）
            byte[] iv = new byte[GCM_IV_LENGTH];
            System.arraycopy(decoded, 0, iv, 0, iv.length);
            GCMParameterSpec ivSpec = new GCMParameterSpec(128, iv);

            Cipher cipher = Cipher.getInstance("AES/GCM-SIV/NoPadding");
            cipher.init(Cipher.DECRYPT_MODE, secretKeySpec, ivSpec);

            byte[] encryptedBytes = new byte[decoded.length - GCM_IV_LENGTH];
            System.arraycopy(decoded, GCM_IV_LENGTH, encryptedBytes, 0, encryptedBytes.length);

            byte[] decryptedBytes = cipher.doFinal(encryptedBytes);
            return new String(decryptedBytes, StandardCharsets.UTF_8);
        } catch (Exception e) {
            throw new RuntimeException("AES 解密失败", e);
        }
    }



    public static String encryptGCMAAD(String plainText, String base64Key) {
        try {
            SecretKeySpec secretKeySpec = new SecretKeySpec(Base64.getDecoder().decode(base64Key), "AES");
            Cipher cipher = Cipher.getInstance("AES/GCM/NoPadding");

            byte[] iv = new byte[GCM_IV_LENGTH];
            SecureRandom random = new SecureRandom();
            random.nextBytes(iv); // 随机生成IV
            GCMParameterSpec ivSpec = new GCMParameterSpec(128, iv);

            cipher.init(Cipher.ENCRYPT_MODE, secretKeySpec, ivSpec);
            cipher.updateAAD("Hello World".getBytes(StandardCharsets.UTF_8));

            byte[] encryptedBytes = cipher.doFinal(plainText.getBytes(StandardCharsets.UTF_8));


            byte[] result = new byte[iv.length + encryptedBytes.length];
            System.arraycopy(iv, 0, result, 0, iv.length);
            System.arraycopy(encryptedBytes, 0, result, iv.length, encryptedBytes.length);

            return Base64.getEncoder().encodeToString(result);
        } catch (Exception e) {
            throw new RuntimeException("AES 加密失败", e);
        }
    }

    /**
     * 解密方法
     *
     * @param encryptedText
     * @return
     */
    public static String decryptGCMADD(String encryptedText, String base64Key) {
        try {
            SecretKeySpec secretKeySpec = new SecretKeySpec(Base64.getDecoder().decode(base64Key), "AES");

            byte[] decoded = Base64.getDecoder().decode(encryptedText);

            // 3. 提取IV（前16字节）和密文（剩余部分）
            byte[] iv = new byte[GCM_IV_LENGTH];
            System.arraycopy(decoded, 0, iv, 0, iv.length);
            GCMParameterSpec ivSpec = new GCMParameterSpec(128, iv);

            Cipher cipher = Cipher.getInstance("AES/GCM/NoPadding");
            cipher.init(Cipher.DECRYPT_MODE, secretKeySpec, ivSpec);
            cipher.updateAAD("Hello World".getBytes(StandardCharsets.UTF_8));

            byte[] encryptedBytes = new byte[decoded.length - GCM_IV_LENGTH];
            System.arraycopy(decoded, GCM_IV_LENGTH, encryptedBytes, 0, encryptedBytes.length);

            byte[] decryptedBytes = cipher.doFinal(encryptedBytes);
            return new String(decryptedBytes, StandardCharsets.UTF_8);
        } catch (Exception e) {
            throw new RuntimeException("AES 解密失败", e);
        }
    }



    public static String encryptCCM(String plainText, String base64Key) {
        try {
            SecretKeySpec secretKeySpec = new SecretKeySpec(Base64.getDecoder().decode(base64Key), "AES");
            Cipher cipher = Cipher.getInstance("AES/CCM/NoPadding");

            byte[] iv = new byte[GCM_IV_LENGTH];
            SecureRandom random = new SecureRandom();
            random.nextBytes(iv); // 随机生成IV
            //for(int i=0; i<iv.length; i++) {
                //iv[i] = 'a';
            //}
            AEADParameterSpec ivSpec = new AEADParameterSpec(iv, 128);

            cipher.init(Cipher.ENCRYPT_MODE, secretKeySpec, ivSpec);

            byte[] encryptedBytes = cipher.doFinal(plainText.getBytes(StandardCharsets.UTF_8));


            byte[] result = new byte[iv.length + encryptedBytes.length];
            System.arraycopy(iv, 0, result, 0, iv.length);
            System.arraycopy(encryptedBytes, 0, result, iv.length, encryptedBytes.length);

            return Base64.getEncoder().encodeToString(result);
        } catch (Exception e) {
            throw new RuntimeException("AES 加密失败", e);
        }
    }

    /**
     * 解密方法
     *
     * @param encryptedText
     * @return
     */
    public static String decryptCCM(String encryptedText, String base64Key) {
        try {
            SecretKeySpec secretKeySpec = new SecretKeySpec(Base64.getDecoder().decode(base64Key), "AES");

            byte[] decoded = Base64.getDecoder().decode(encryptedText);

            // 3. 提取IV（前16字节）和密文（剩余部分）
            byte[] iv = new byte[GCM_IV_LENGTH];
            System.arraycopy(decoded, 0, iv, 0, iv.length);
            AEADParameterSpec ivSpec = new AEADParameterSpec(iv, 128);

            Cipher cipher = Cipher.getInstance("AES/CCM/NoPadding");
            cipher.init(Cipher.DECRYPT_MODE, secretKeySpec, ivSpec);

            byte[] encryptedBytes = new byte[decoded.length - GCM_IV_LENGTH];
            System.arraycopy(decoded, GCM_IV_LENGTH, encryptedBytes, 0, encryptedBytes.length);

            byte[] decryptedBytes = cipher.doFinal(encryptedBytes);
            return new String(decryptedBytes, StandardCharsets.UTF_8);
        } catch (Exception e) {
            throw new RuntimeException("AES 解密失败", e);
        }
    }


    public static String encryptSIV(String plainText, String base64Key) {
        try {

            final SivMode AES_SIV = new SivMode();
            byte[] key = Base64.getDecoder().decode(base64Key);
            byte[] ctrl = Arrays.copyOf(key, key.length/2);
            byte[] mac =  Arrays.copyOfRange(key, key.length/2, key.length);
            byte[] result = AES_SIV.encrypt(ctrl, mac, plainText.getBytes(StandardCharsets.UTF_8));
            return Base64.getEncoder().encodeToString(result);
        } catch (Exception e) {
            throw new RuntimeException("AES 加密失败", e);
        }
    }

    /**
     * 解密方法
     *
     * @param encryptedText
     * @return
     */
    public static String decryptSIV(String encryptedText, String base64Key) {
        try {
            final SivMode AES_SIV = new SivMode();
            byte[] key = Base64.getDecoder().decode(base64Key);
            byte[] ctrl = Arrays.copyOf(key, key.length/2);
            byte[] mac =  Arrays.copyOfRange(key, key.length/2, key.length);
            byte[] decryptedBytes = AES_SIV.decrypt(ctrl, mac,Base64.getDecoder().decode(encryptedText));
            return new String(decryptedBytes, StandardCharsets.UTF_8);
        } catch (Exception e) {
            throw new RuntimeException("AES 解密失败", e);
        }
    }



}
