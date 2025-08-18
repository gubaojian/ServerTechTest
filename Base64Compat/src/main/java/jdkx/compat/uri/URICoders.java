package jdkx.compat.uri;

import java.io.UnsupportedEncodingException;
import java.net.URLDecoder;
import java.net.URLEncoder;

/**
 * Decodes '%'-escaped octets in the given string using the UTF-8 scheme.
 * Replaces invalid octets with the unicode replacement character ("\uFFFD").
 */
public class URICoders {

    private URICoders() {}

    public static String decode(String s) {
        if (s == null) {
            return null;
        }
        try {
            return  URLDecoder.decode(s, "UTF-8");
        } catch (UnsupportedEncodingException e) {
            throw new IllegalArgumentException(e);
        }
    }

    public static String encode(String s) {
        if (s == null) {
            return null;
        }
        try {
            return  URLEncoder.encode(s, "UTF-8");
        } catch (UnsupportedEncodingException e) {
            throw new IllegalArgumentException(e);
        }
    }
}
