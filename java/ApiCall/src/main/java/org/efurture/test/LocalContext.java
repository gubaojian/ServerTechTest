package org.efurture.test;

import java.util.Map;
import java.util.function.Supplier;

public class LocalContext {

    public static Map<String,Object> getSource() {
        return localSource.get();
    }

    public static void setSource(Map<String,Object> source) {
        localSource.set(source);
    }

    private static final ThreadLocal<Map<String,Object>> localSource = new ThreadLocal<>();
}
