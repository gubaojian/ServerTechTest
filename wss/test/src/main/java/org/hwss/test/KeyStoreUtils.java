package org.hwss.test;

import java.io.FileInputStream;
import java.security.Key;
import java.security.KeyStore;
import java.security.PrivateKey;
import java.security.PublicKey;
import java.security.cert.Certificate;

public class KeyStoreUtils {

    public static void main(String[] argv) throws Exception {
        FileInputStream is = new FileInputStream("/Users/efurture/holo-literacy-app.flutter2/android/story.keystore");

        KeyStore keystore = KeyStore.getInstance(KeyStore.getDefaultType());
        keystore.load(is, "hi123456".toCharArray());

        String alias = "hi-story";

        Key key = keystore.getKey(alias, "hi123456".toCharArray());
        if (key instanceof PrivateKey) {
            // Get certificate of public key
            Certificate cert = keystore.getCertificate(alias);

            // Get public key
            PublicKey publicKey = cert.getPublicKey();

            // Return a key pair
            System.out.println("publicKey " + RSAUtils.byteArrayToHex(publicKey.getEncoded()));
        }
    }
}
