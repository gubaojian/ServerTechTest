package jdkx.compat.util.threadlocal;

import javax.crypto.Mac;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.text.SimpleDateFormat;
import java.util.Random;

/**
 * only can used in one thread local, all value use lazy init
 * */
public class LocalValueEntry {
    private Random random;
    private SecureRandom secureRandom;
    private SimpleDateFormat dateFormatOne;
    private SimpleDateFormat dateFormatTwo;
    private Mac hmacSHA256;

    public Random getRandom() {
        if (random == null) {
            random = new Random();
        }
        return random;
    }

    public SecureRandom getSecureRandom() {
        if (secureRandom == null) {
            secureRandom = new SecureRandom();
        }
        return secureRandom;
    }

    public SimpleDateFormat getDateFormatOne() {
        if (dateFormatOne  == null) {
            dateFormatOne = new SimpleDateFormat("yyyy-MM-dd");
        }
        return dateFormatOne;
    }

    public SimpleDateFormat getDateFormatTwo() {
        if (dateFormatTwo == null) {
            dateFormatTwo = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
        }
        return dateFormatTwo;
    }


    public Mac getHmacSHA256() throws NoSuchAlgorithmException {
        if (hmacSHA256 == null) {
            hmacSHA256 =  Mac.getInstance("HmacSHA256");
        }
        return hmacSHA256;
    }
}
