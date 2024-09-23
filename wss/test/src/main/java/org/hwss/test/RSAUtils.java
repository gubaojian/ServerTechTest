package org.hwss.test;

import java.io.ByteArrayOutputStream;
import java.math.BigInteger;
import java.net.InetAddress;
import java.net.URL;
import java.net.UnknownHostException;
import java.nio.charset.StandardCharsets;
import java.security.*;
import java.security.spec.InvalidKeySpecException;
import java.security.spec.PKCS8EncodedKeySpec;
import java.security.spec.RSAPublicKeySpec;
import java.util.UUID;
import javax.crypto.BadPaddingException;
import javax.crypto.Cipher;
import javax.crypto.IllegalBlockSizeException;
import javax.crypto.NoSuchPaddingException;

/**
 * https://www.hutool.cn/docs/#/core/%E5%B7%A5%E5%85%B7%E7%B1%BB/16%E8%BF%9B%E5%88%B6%E5%B7%A5%E5%85%B7-HexUtil
 * */
public class RSAUtils {
    public static void main(String[] args) throws NoSuchAlgorithmException, UnknownHostException {
        // 生成 RSA 密钥对
        KeyPairGenerator keyPairGenerator = KeyPairGenerator.getInstance("RSA");
        keyPairGenerator.initialize(2048);
        KeyPair keyPair = keyPairGenerator.generateKeyPair();
        PublicKey publicKey = keyPair.getPublic();
        PrivateKey privateKey = keyPair.getPrivate();

        // 要加密的文本
        String originalText = "wss?appId=111&role=router&routerId=wss?appId=111&role=router&routerId=xx5555555555552333&authToken=xx23523532532535353wss?appId=111&role=router&routerId=xx5555555555552333&authToken=xx23523532532535353&authId=xx777235235325325&timestamp=55555555555xx23t325233535&fmt=json/other这是要加密的文本这是要加密的文本这是要加密的文本这是要加密的文本这是要加密的文本这是要加密的文本这是要加密的文本";

        try {
            // 使用公钥加密
            String encryptString = encrypt(originalText, publicKey);
            String decryptStr = decrypt(encryptString, privateKey);
            long start = System.currentTimeMillis();
            for (int i=0; i<1000; i++) {
                //decryptStr = decrypt(encryptString, privateKey);
            }
            long end = System.currentTimeMillis();
            System.out.println("used " + (end - start));
            System.out.println("公钥：" + byteArrayToHex(publicKey.getEncoded()));
            System.out.println("私钥：" + byteArrayToHex(privateKey.getEncoded()));
            System.out.println("加密后的文本：" + encryptString);
            System.out.println("解密成功：" +  decryptStr.equals(originalText));
            System.out.println("解密文本：" +  decryptStr);
            System.out.println("原始文本：" +  originalText);
            System.out.println("getSHA256Hash " + getSHA256Hash(originalText));

            StringBuilder sb = new StringBuilder();
            sb.append("time=");
            sb.append(System.currentTimeMillis());
            sb.append("&hostId=");
            sb.append("4c1d-bbdc-3c3d70584c32");
            sb.append("&authId=");
            sb.append(UUID.randomUUID().toString());
            sb.append("&authToken=");
            sb.append(UUID.randomUUID().toString() + UUID.randomUUID().toString());


            String demoText =  sb.toString();
            String demo = encrypt(demoText, publicKey);
            System.out.println(demoText);
            System.out.println(demoText.length() + " hex encrypt length " + demo.length());

            StringBuilder urlSb = new StringBuilder("wss://localhost:9001/wss?");
            urlSb.append("serverAppId=");
            urlSb.append(System.currentTimeMillis());
            urlSb.append("&serverAppToken=");
            urlSb.append(UUID.randomUUID().toString());
            urlSb.append("&role=");
            urlSb.append("server");
            urlSb.append("&sv=");
            urlSb.append(demo);


            String url = urlSb.toString();

            System.out.println(url);
            System.out.println(url.length());


        } catch (Exception e) {
            e.printStackTrace();
        }

        String testPublicKey = "30820122300d06092a864886f70d01010105000382010f003082010a0282010100c69cd369c74a1d99bc1c6bdaa5b4d0e9d4ca7b467fdfb6becacc7c050f84c04384d46b55f21d8e2a4738d4ae060c3fbaa3fee1b23b0d140486b5234d86c6a7f979b4c9a479a28010689026fd24968d6d4cc796fe7b06eb9ddbc6cb4d62b60dcff45fcc75a957e654b55e095b2365a0962df58af86418c0b37d4cf3cdedcf45637d0cf6a3bb265a6659c62815c8906504dbf3d0b5c522963567f2e7824f83e9895f2ce698d00917b4ef9b5eb6c3832dfa178193d10cf2b5d54f472c762ff6d340711b1b3b7b75954dfc05adf84910bd561dfae1f469a6f48c0838336736eb1bb2f1fff4f8fb8799b932b096c4978ffb1839958f8817b82ca2627e3d3399edf4eb0203010001";
        String testPrivateKey = "308204bc020100300d06092a864886f70d0101010500048204a6308204a20201000282010100c69cd369c74a1d99bc1c6bdaa5b4d0e9d4ca7b467fdfb6becacc7c050f84c04384d46b55f21d8e2a4738d4ae060c3fbaa3fee1b23b0d140486b5234d86c6a7f979b4c9a479a28010689026fd24968d6d4cc796fe7b06eb9ddbc6cb4d62b60dcff45fcc75a957e654b55e095b2365a0962df58af86418c0b37d4cf3cdedcf45637d0cf6a3bb265a6659c62815c8906504dbf3d0b5c522963567f2e7824f83e9895f2ce698d00917b4ef9b5eb6c3832dfa178193d10cf2b5d54f472c762ff6d340711b1b3b7b75954dfc05adf84910bd561dfae1f469a6f48c0838336736eb1bb2f1fff4f8fb8799b932b096c4978ffb1839958f8817b82ca2627e3d3399edf4eb0203010001028201001bb04456f456144f58e6bb70857672438f0b55694c2d738c20b8a6378c64cceafccb22559e85d94a72986d29ffa4d8839c164dffab74d4cc16bd8cf8e367b186399de7189b6e789d10febbb8c1cf8d52e607d338920d326ed4f35f675ff21275e1d3332d5a18eb7ef6b3efe8f943d341ec915538f0b774b1f12f77b3c74a0499adb3efe45902b073ab4e923bb596c4b57dcde2f541ab20aa81fe3492e1fd6b556d406b73978cb7e9d6db0638514299bd3e665ec9d831bfef9ba397e7d9f5a5c06932af92a303de3340f53cee4f88d840ef49ba87701335b50355cf2e7ecd57967a147efc91bc68678ad0391ab9e66761ee2f5e5eb5662d2fd380cb558821d30902818100e66f08d54f2dba91a8c05b317586252af5d2720de5f180a259f67865777f81421601348251815acf08ded771cd8a728d23ae2a5b8132350aee46df9e4cd611feed4d05537500d384d16eda8bdc71b5257a59067ba5cc71c9407df505ff1f3bd40edb151e356a2b76b17f8bcadbdd7fa5a6df5bc72e6254c79818c595783fadc902818100dca5fb5c1ae1bbc52ceb06d076aadcdd75014ee07421ee2cdcf5c3767d88aa0b644c3fa457c5450b089a0ff3edd59154a7d909e0f1efb656fa83a77ce5d15eaaf0882b5c38345cb753d91bec2e1afb6b56be020cc669c7cf026eed44d217188b8d79a8d624f9be73f3f3b24f2db0d96333d7e8768a4be4d3dd4e5ea83f5f1713028180082e931ab7d800d2e1320b4ae3486313766d057db6d76f60d45eeb01774fc0b02f05d5ee76a4682c07bd99dac19893dead10a7125eb100af25acc08de75e867f0dbff95b70411a41920a249a68b664df32cbec6e018213dc387d00713910cbd808bb00b1f4e5932b80bbeda3c350c71283ad549deae8b4e7ed5f9bcc65dba4f90281801a9584eb5bf1c685c31badfe7fcda63ea8523806a2f75392e42c5818769778a556b83912bbe070a91ba83433d77bdbbeaf5a5eee5337e6682456a85f4bf125cec29c255d3f085003667ec9f672b79d8319385b56deb0eff9e8f9246d1e42da1a36396a3622ce543bf430705349d5fcdb0eebfc7e0bbb891e5d938ba409be9331028180302877b363ba57f62eaeb284bb4a4d4c2ed217826866ed14d2c45febdbd6677e9ea09315604a4a6e1d2a78cd1f7aa53486df46f0ef1bfb117f60f546976ce75ef45f197afd39201823c8a4d6ef250213108340cd49afa9b44b439065065b7ce3abab7acd537e840553f79347be9d81aa9bde45b4d30870498b0aa7149b454915";
        String testEncryptText = "b332c3d96beab24a8ebef99859738c0c55228143280ca5cfe108cd1a12556e9c744f995f0757084e7f44edaa9d1f9bcdf0fe366d477c2000adfaa200971975fc222b7d3883d9dbfc57f165bf2a64b5915fe6b3b15703a3bfd70985e6cbc2830a54c045607c65efe6a6357a2d2f1edab6573abf8344c0b09c4f43d69d2e4ff1ff672c44ae660ec3c3ab4ae09ae57c2961f157dfcb51e2dabb3430ffed446390533251f83f495e8a7dbd694c0f45b01b40ffec916d46ca5722d87e7ec9e1833316a44ac1c47442748d9485605462758de683d5f3eba90597a76e4286e7d2884876587a6f4ca83c45b7e4513fc3aa392f6f7cd7968f5113cdb0d5904d693ec4a0d0_416242de7a5c681a93bf37f661c06955aa335f4fe406848d8b53c8491968d9f4e64341d59bea89ab6ddb96350acd92ef8aed8a69d15c7177cc79caf18915b85b7bfa868908a8dd4df00356874acc7563643078f2ef545ab5d6e1849870b98d4adb7794a2f6474b0ccfd9e34bcbf757d936f209556c6ae5fbe8deb0dec4da29b2de87e591de93f6fca9750c0589b6c363be56128ee2d03907d878470b61fd265347cde29d8acd7b8e9e7a94c8394f32337cd9aed291beea6fb8dd99449a01ce354f1b8cc18aa85c0816427549fcdcfaadcc6534f0929f5be52889b9b8cf68cec24778fd5473d542131f6ebe400d953ec15192bc43624e1893a2858b5c1854ac4f";

        System.out.println("测试解密：" + testDecrypt(testEncryptText, testPrivateKey));

        System.out.println("InetAddress.getLocalHost().getHostName()" + InetAddress.getLocalHost().getHostName());
    }

    public  static String testDecrypt(String input, String privateKey) {
        try {
            return  decrypt(input, privateKey);
        } catch (NoSuchAlgorithmException e) {
            throw new RuntimeException(e);
        } catch (InvalidKeySpecException e) {
            throw new RuntimeException(e);
        } catch (NoSuchPaddingException e) {
            throw new RuntimeException(e);
        } catch (IllegalBlockSizeException e) {
            throw new RuntimeException(e);
        } catch (BadPaddingException e) {
            throw new RuntimeException(e);
        } catch (InvalidKeyException e) {
            throw new RuntimeException(e);
        }
    }

    public static String decrypt(String input, String keyHex) throws NoSuchAlgorithmException, InvalidKeySpecException, NoSuchPaddingException, IllegalBlockSizeException, BadPaddingException, InvalidKeyException {
        PKCS8EncodedKeySpec keySpec = new PKCS8EncodedKeySpec(hexToByteArray(keyHex));
        KeyFactory keyFactory = KeyFactory.getInstance("RSA");
        PrivateKey privateKey = keyFactory.generatePrivate(keySpec);
        return decrypt(input, privateKey);
    }

    public static String decrypt(String input, PrivateKey key) throws NoSuchPaddingException, NoSuchAlgorithmException, InvalidKeyException, IllegalBlockSizeException, BadPaddingException {
        Cipher cipher = Cipher.getInstance("RSA");
        cipher.init(Cipher.DECRYPT_MODE, key);
        String[] pieces = input.split("_");
        ByteArrayOutputStream output = new ByteArrayOutputStream();
        for(String item : pieces) {
            byte[] decryptedBytes = cipher.doFinal(hexToByteArray(item));
            output.writeBytes(decryptedBytes);
        }
        return output.toString(StandardCharsets.UTF_8);
    }


    public static String encrypt(String input, String hexPublicKey) throws NoSuchPaddingException, NoSuchAlgorithmException, InvalidKeyException, InvalidKeySpecException, IllegalBlockSizeException, BadPaddingException {
        BigInteger modulus = new BigInteger(hexPublicKey.substring(0, 256), 16);
        BigInteger exponent = new BigInteger(hexPublicKey.substring(256), 16);
        RSAPublicKeySpec keySpec = new RSAPublicKeySpec(modulus, exponent);
        KeyFactory keyFactory = KeyFactory.getInstance("RSA");
        PublicKey publicKey = keyFactory.generatePublic(keySpec);
        return  encrypt(input, publicKey);
    }

    public static String encrypt(String input, PublicKey publicKey) throws NoSuchPaddingException, NoSuchAlgorithmException, InvalidKeyException, IllegalBlockSizeException, BadPaddingException {
        Cipher cipher = Cipher.getInstance("RSA");
        cipher.init(Cipher.ENCRYPT_MODE, publicKey);
        StringBuilder sb = new StringBuilder();
        //如果用byte[]分割后单次解密无法复原完整的字符串的一部分
        byte[]  bts = input.getBytes(StandardCharsets.UTF_8);
        int length = 244; //2048 最大245字节,单次240个，尽量减少次数，减少数据长度 1024 公钥
        for(int i=0; i<bts.length; i +=length) {
            int len = Math.min(length, bts.length-i);
            byte[] encryptedBytes = cipher.doFinal(bts, i, len);
            sb.append(byteArrayToHex(encryptedBytes));
            sb.append("_");
        }
        sb.deleteCharAt(sb.length()-1);
        return  sb.toString();

    }

    public static String encrypt2(String input, PublicKey publicKey) throws NoSuchPaddingException, NoSuchAlgorithmException, InvalidKeyException, IllegalBlockSizeException, BadPaddingException {
        Cipher cipher = Cipher.getInstance("RSA");
        cipher.init(Cipher.ENCRYPT_MODE, publicKey);
        StringBuilder sb = new StringBuilder();
        int maxLength = 60; //最大245字节，UTF-8最长4个字节。60个字符. 保证分割的完整性. 但加密次数多，加密结果大
        //如果用byte[]分割后单次解密无法复原完整的字符串的一部分
        for(int i=0; i<input.length(); i +=maxLength) {
            int len = Math.min(maxLength, input.length()-i);
            String item = input.substring(i, i + len);
            System.out.println("item: " + item + " " + len + " " + i + " " + input.length());
            byte[] encryptedBytes = cipher.doFinal(item.getBytes(StandardCharsets.UTF_8));
            sb.append(byteArrayToHex(encryptedBytes));
            sb.append("_");
        }
        sb.deleteCharAt(sb.length()-1);
        return  sb.toString();
    }




    public static String byteArrayToHex(byte[] bytes) {
        StringBuilder sb = new StringBuilder();
        for (byte b : bytes) {
            sb.append(String.format("%02x", b));
        }
        return sb.toString();
    }

    public static byte[] hexToByteArray(String s) {
        int len = s.length();
        byte[] data = new byte[len / 2];
        for (int i = 0; i < len; i += 2) {
            data[i / 2] = (byte) ((Character.digit(s.charAt(i), 16) << 4)
                    + Character.digit(s.charAt(i + 1), 16));
        }
        return data;
    }


    public static String getSHA256Hash(String input) throws NoSuchAlgorithmException {
        MessageDigest digest = MessageDigest.getInstance("SHA-256");
        byte[] hash = digest.digest(input.getBytes(StandardCharsets.UTF_8));
        return byteArrayToHex(hash);
    }
}