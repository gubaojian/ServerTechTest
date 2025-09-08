package jdkx.compat.util.threadlocal;

import javax.crypto.Mac;
import java.security.InvalidKeyException;
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


    public Mac getHmacSHA256() throws NoSuchAlgorithmException, InvalidKeyException {
        if (hmacSHA256 == null) {
            hmacSHA256 =  Mac.getInstance("HmacSHA256");
        }
        return hmacSHA256;
    }

    public String genTraceId() {
        StringBuilder sb = new StringBuilder(96);
        sb.append(currentTimeNano());
        sb.append('-');
        sb.append(Math.abs(getRandom().nextLong()));
        sb.append('-');
        sb.append(Math.abs(getSecureRandom().nextLong()));
        return sb.toString();
    }

    public long currentTimeNano() {
        long currentTime = System.currentTimeMillis()*1000000L;
        if (currentTime == preTime) {
            currentTime += Math.max((System.nanoTime() - preNanoTime), 1L);
        } else {
            if (currentTime < preTime) {
                currentTime = preTime + 1;
            }
            preNanoTime = System.nanoTime();
            preTime = currentTime;
        }
        return currentTime;
    }

    private long preTime = 0;
    private long preNanoTime = 0;
}
