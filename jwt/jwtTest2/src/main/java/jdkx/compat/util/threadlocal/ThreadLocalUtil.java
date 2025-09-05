package jdkx.compat.util.threadlocal;

import java.security.SecureRandom;
import java.util.Random;

public class ThreadLocalUtil {
    private static final ThreadLocal<LocalValueEntry> local = new ThreadLocal<>(){
        @Override
        protected LocalValueEntry initialValue() {
            return new LocalValueEntry();
        }
    };

    public static Random getRandom() {
        return getLocalValueEntry().getRandom();
    }

    public static SecureRandom getSecureRandom() {
        return getLocalValueEntry().getSecureRandom();
    }

    public static LocalValueEntry getLocalValueEntry() {
        LocalValueEntry valueEntry = local.get();
        if (valueEntry != null) {
            return valueEntry;
        }
        return new LocalValueEntry();
    }


}
