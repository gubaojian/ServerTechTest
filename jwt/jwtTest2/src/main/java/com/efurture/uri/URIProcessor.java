package com.efurture.uri;


import jdkx.compat.util.Compat;

import java.net.MalformedURLException;
import java.net.URI;
import java.net.URISyntaxException;
import java.net.URL;
import java.nio.ByteBuffer;
import java.nio.CharBuffer;
import java.nio.charset.CharacterCodingException;
import java.nio.charset.CharsetEncoder;
import java.util.*;

/**
 * Helper class for building or manipulating URI references. Not safe for concurrent use.
 * 支持常用： http https
 * 暂时不支持：mailto:test@example.com
 * */
public class URIProcessor {
    private final URI sourceUri;
    private final Map<String, List<String>> queryMap;
    private String overridePath;
    private String overrideFragment;

    public URIProcessor(URI sourceUri) {
        this.sourceUri = sourceUri;
        this.queryMap = new LinkedHashMap<>(12);
        this.parseQueryParametersToMap();
    }

    private URIProcessor(URI sourceUri, Map<String, List<String>> queryMap) {
        this.sourceUri = sourceUri;
        this.queryMap = new LinkedHashMap<>(12);
        putAllParameters(queryMap);
    }

    public static URIProcessor fromURI(URI uri) {
        return new URIProcessor(uri);
    }

    public static URIProcessor fromURI(String uriStr) {
        URI uri = URI.create(uriStr);
        return new URIProcessor(uri);
    }

    public static URIProcessor fromURL(URL url) {
        try {
            return new URIProcessor(url.toURI());
        } catch (URISyntaxException e) {
            throw new IllegalArgumentException(e);
        }
    }

    public static URIProcessor fromURL(String url) {
        return fromURI(url);
    }

    public static URIProcessor fromProcessor(URIProcessor processor) {
        return new URIProcessor(processor.getSourceUri(), processor.getQueryMap());
    }

    public URI getSourceUri() {
        return sourceUri;
    }

    public Map<String, List<String>> getQueryMap() {
        return queryMap;
    }

    /**
     * Searches the query string for the first value with the given key.
     */
    public String getQueryParameter(String key) {
        return getQueryParameter(key, null);
    }
    public String getQueryParameter(String key, String defaultValue) {
        List<String> values = queryMap.get(key);
        if (values == null || values.isEmpty()) {
            return defaultValue;
        }
        return values.get(0);
    }

    public List<String> getQueryParameters(String key) {
        List<String> values = queryMap.get(key);
        if (values == null || values.isEmpty()) {
            return Collections.<String>emptyList();
        }
        return new ArrayList<String>(values);
    }

    /**
     * Searches the query string for the first value with the given key and interprets it
     * as a boolean value. "false" and "0" are interpreted as <code>false</code>, everything
     * else is interpreted as <code>true</code>.
     *
     * @param key which will be decoded
     * @param defaultValue the default value to return if there is no query parameter for key
     * @return the boolean interpretation of the query parameter key
     */
    public boolean getBooleanQueryParameter(String key, boolean defaultValue) {
        String flag = getQueryParameter(key);
        if (flag == null) {
            return defaultValue;
        }
        flag = flag.toLowerCase(Locale.ROOT);
        return (!"false".equals(flag) && !"0".equals(flag));
    }


    /**
     * Encodes the key and value and then appends the parameter to the
     * query string.
     *
     * @param key which will be encoded
     * @param value which will be encoded
     */
    public URIProcessor appendQueryParameter(String key, String value) {
        if (key == null || value == null) {
            throw new IllegalArgumentException("key or value should not be null");
        }
        List<String> values = queryMap.get(key);
        if (values == null) {
            values = new ArrayList<>(2);
            queryMap.put(key, values);
        }
        values.add(value);
        return this;
    }

    public URIProcessor setQueryParameter(String key, String value) {
        if (key == null || value == null) {
            throw new IllegalArgumentException("key or value should not be null");
        }
        List<String> values = queryMap.get(key);
        if (values == null) {
            values = new ArrayList<>(2);
            queryMap.put(key, values);
        }
        values.clear();
        values.add(value);
        return this;
    }

    public URIProcessor setPath(String path) {
        if (path == null) {
            path = "/";
        }
        overridePath = path;
        return this;
    }

    public URIProcessor setFragment(String fragment) {
        if (fragment == null) {
            fragment = "/";
        }
        overrideFragment = fragment;
        return this;
    }

    public URIProcessor clearQuery() {
        queryMap.clear();
        return this;
    }

    public URIProcessor clearAllQueryParameters() {
        return clearQuery();
    }

    public URIProcessor clearAllParameters() {
        return clearQuery();
    }

    public URIProcessor putAllParameter(Map<String, String> parameters) {
        Set<Map.Entry<String, String>> entries = parameters.entrySet();
        for(Map.Entry<String, String> entry : entries) {
            appendQueryParameter(entry.getKey(), entry.getValue());
        }
        return this;
    }

    public URIProcessor putAllParameters(Map<String, List<String>> parameters) {
        Set<Map.Entry<String, List<String>>> entries = parameters.entrySet();
        for(Map.Entry<String, List<String>> entry : entries) {
            for(String value : entry.getValue()) {
                appendQueryParameter(entry.getKey(), value);
            }
        }
        return this;
    }

    public URIProcessor removeQueryParameter(String key) {
        queryMap.remove(key);
        return this;
    }

    public URI toURI() {
        if (queryMap.isEmpty()) {
            return sourceUri;
        }
        try {
            String query = mapToQuery();
            String path = overridePath == null ? sourceUri.getPath() : overridePath;
            String fragment  = overrideFragment == null ? sourceUri.getFragment() : overrideFragment;
            return new URI(
                    sourceUri.getScheme(),
                    sourceUri.getUserInfo(),
                    sourceUri.getHost(),
                    sourceUri.getPort(),
                    path,
                    query,
                    fragment
            );
        } catch (Exception e) {
            throw new IllegalArgumentException(e);
        }
    }

    /**
     * 比toURI性能更好，速度快10-20倍左右。
     * */
    public String fastToUri() {
        StringBuilder sb = new StringBuilder(1024);
        if (sourceUri.getScheme() != null) {
            sb.append(sourceUri.getScheme());
            sb.append(":");
        }
        if (sourceUri.getHost() != null) {
            sb.append("//");
            if (sourceUri.getUserInfo() != null) {
                sb.append(sourceUri.getRawUserInfo());
                sb.append('@');
            }
            String host = sourceUri.getHost();
            boolean needBrackets = ((host.indexOf(':') >= 0)
                    && !host.startsWith("[")
                    && !host.endsWith("]"));
            if (needBrackets) sb.append('[');
            sb.append(host);
            if (needBrackets) sb.append(']');
            if (sourceUri.getPort() != -1) {
                sb.append(':');
                sb.append(sourceUri.getPort());
            }
        }
        if (sourceUri.getRawPath() != null) {
            sb.append(sourceUri.getRawPath());
        }
        if (!queryMap.isEmpty()) {
            sb.append('?');
            String query = mapToQuery();
            sb.append(quote(query, L_URIC, H_URIC));
        }
        if (sourceUri.getRawFragment() != null) {
            sb.append('#');
            sb.append(sourceUri.getRawFragment());
        }
        return sb.toString();
    }

    public URL toURL() {
        try {
            return toURI().toURL();
        } catch (MalformedURLException e) {
            throw new IllegalArgumentException(e);
        }
    }

    @Override
    public String toString() {
        return "URIProcessor{" +
                "sourceUri=" + sourceUri +
                ", queryMap=" + queryMap +
                '}';
    }

    /**
     * parse query to map.
     */
    private void parseQueryParametersToMap() {
        String query = sourceUri.getQuery();
        if (query == null  || query.isEmpty()) {
            return;
        }
        int start = 0;
        do {
            int nextAndPos = query.indexOf('&', start);
            int end = nextAndPos != -1 ? nextAndPos : query.length();

            int separator = query.indexOf('=', start);
            if (separator > end || separator == -1) {
                separator = end;
            }

            String key = URICoders.decode(query.substring(start, separator));
            List<String> values = queryMap.get(key);
            if (values == null) {
                values = new ArrayList<>(2);
                queryMap.put(key, values);
            }
            if (separator == end) {
                values.add("");
            } else {
                String value = query.substring(separator + 1, end);
                values.add(URICoders.decode(value));
            }

            // Move start to end of name.
            if (nextAndPos != -1) {
                start = nextAndPos + 1;
            } else {
                break;
            }

        } while (start < query.length());
    }

    /**
     * parse query to map.
     */
    private String mapToQuery() {
        if (queryMap.isEmpty()) {
            return null;
        }
        StringBuilder sb = new StringBuilder(512);
        for(Map.Entry<String, List<String>> entry : queryMap.entrySet()) {
            List<String> values = entry.getValue();
            for(String value : values) {
                sb.append(URICoders.encode(entry.getKey()));
                sb.append("=");
                sb.append(URICoders.encode(value));
                sb.append("&");
            }
        }
        if (sb.length() > 0) {
            sb.deleteCharAt(sb.length()-1);
        }
        return sb.toString();
    }

    // Quote any characters in s that are not permitted
    // by the given mask pair
    //
    private static String quote(String s, long lowMask, long highMask) {
        StringBuilder sb = null;
        CharsetEncoder encoder = null;
        boolean allowNonASCII = ((lowMask & L_ESCAPED) != 0);
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            if (c < '\u0080') {
                if (!match(c, lowMask, highMask)) {
                    if (sb == null) {
                        sb = new StringBuilder();
                        sb.append(s, 0, i);
                    }
                    appendEscape(sb, (byte)c);
                } else {
                    if (sb != null)
                        sb.append(c);
                }
            } else if (allowNonASCII
                    && (Character.isSpaceChar(c)
                    || Character.isISOControl(c))) {
                if (encoder == null)
                    encoder = Compat.StandardCharsets.UTF_8.newEncoder();
                if (sb == null) {
                    sb = new StringBuilder();
                    sb.append(s, 0, i);
                }
                appendEncoded(encoder, sb, c);
            } else {
                if (sb != null)
                    sb.append(c);
            }
        }
        return (sb == null) ? s : sb.toString();
    }

    // Character-class masks, in reverse order from RFC2396 because
    // initializers for static fields cannot make forward references.

    // digit    = "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" |
    //            "8" | "9"
    private static final long L_DIGIT = 0x3FF000000000000L; // lowMask('0', '9');
    private static final long H_DIGIT = 0L;

    // upalpha  = "A" | "B" | "C" | "D" | "E" | "F" | "G" | "H" | "I" |
    //            "J" | "K" | "L" | "M" | "N" | "O" | "P" | "Q" | "R" |
    //            "S" | "T" | "U" | "V" | "W" | "X" | "Y" | "Z"
    private static final long L_UPALPHA = 0L;
    private static final long H_UPALPHA = 0x7FFFFFEL; // highMask('A', 'Z');

    // lowalpha = "a" | "b" | "c" | "d" | "e" | "f" | "g" | "h" | "i" |
    //            "j" | "k" | "l" | "m" | "n" | "o" | "p" | "q" | "r" |
    //            "s" | "t" | "u" | "v" | "w" | "x" | "y" | "z"
    private static final long L_LOWALPHA = 0L;
    private static final long H_LOWALPHA = 0x7FFFFFE00000000L; // highMask('a', 'z');

    // alpha         = lowalpha | upalpha
    private static final long L_ALPHA = L_LOWALPHA | L_UPALPHA;
    private static final long H_ALPHA = H_LOWALPHA | H_UPALPHA;

    // alphanum      = alpha | digit
    private static final long L_ALPHANUM = L_DIGIT | L_ALPHA;
    private static final long H_ALPHANUM = H_DIGIT | H_ALPHA;

    // hex           = digit | "A" | "B" | "C" | "D" | "E" | "F" |
    //                         "a" | "b" | "c" | "d" | "e" | "f"
    private static final long L_HEX = L_DIGIT;
    private static final long H_HEX = 0x7E0000007EL; // highMask('A', 'F') | highMask('a', 'f');

    // mark          = "-" | "_" | "." | "!" | "~" | "*" | "'" |
    //                 "(" | ")"
    private static final long L_MARK = 0x678200000000L; // lowMask("-_.!~*'()");
    private static final long H_MARK = 0x4000000080000000L; // highMask("-_.!~*'()");

    // unreserved    = alphanum | mark
    private static final long L_UNRESERVED = L_ALPHANUM | L_MARK;
    private static final long H_UNRESERVED = H_ALPHANUM | H_MARK;

    // reserved      = ";" | "/" | "?" | ":" | "@" | "&" | "=" | "+" |
    //                 "$" | "," | "[" | "]"
    // Added per RFC2732: "[", "]"
    private static final long L_RESERVED = 0xAC00985000000000L; // lowMask(";/?:@&=+$,[]");
    private static final long H_RESERVED = 0x28000001L; // highMask(";/?:@&=+$,[]");

    // The zero'th bit is used to indicate that escape pairs and non-US-ASCII
    // characters are allowed; this is handled by the scanEscape method below.
    private static final long L_ESCAPED = 1L;
    private static final long H_ESCAPED = 0L;

    // uric          = reserved | unreserved | escaped
    private static final long L_URIC = L_RESERVED | L_UNRESERVED | L_ESCAPED;
    private static final long H_URIC = H_RESERVED | H_UNRESERVED | H_ESCAPED;

    // pchar         = unreserved | escaped |
    //                 ":" | "@" | "&" | "=" | "+" | "$" | ","
    private static final long L_PCHAR
            = L_UNRESERVED | L_ESCAPED | 0x2400185000000000L; // lowMask(":@&=+$,");
    private static final long H_PCHAR
            = H_UNRESERVED | H_ESCAPED | 0x1L; // highMask(":@&=+$,");

    // All valid path characters
    private static final long L_PATH = L_PCHAR | 0x800800000000000L; // lowMask(";/");
    private static final long H_PATH = H_PCHAR; // highMask(";/") == 0x0L;

    // Dash, for use in domainlabel and toplabel
    private static final long L_DASH = 0x200000000000L; // lowMask("-");
    private static final long H_DASH = 0x0L; // highMask("-");

    // Dot, for use in hostnames
    private static final long L_DOT = 0x400000000000L; // lowMask(".");
    private static final long H_DOT = 0x0L; // highMask(".");

    // userinfo      = *( unreserved | escaped |
    //                    ";" | ":" | "&" | "=" | "+" | "$" | "," )
    private static final long L_USERINFO
            = L_UNRESERVED | L_ESCAPED | 0x2C00185000000000L; // lowMask(";:&=+$,");
    private static final long H_USERINFO
            = H_UNRESERVED | H_ESCAPED; // | highMask(";:&=+$,") == 0L;

    // reg_name      = 1*( unreserved | escaped | "$" | "," |
    //                     ";" | ":" | "@" | "&" | "=" | "+" )
    private static final long L_REG_NAME
            = L_UNRESERVED | L_ESCAPED | 0x2C00185000000000L; // lowMask("$,;:@&=+");
    private static final long H_REG_NAME
            = H_UNRESERVED | H_ESCAPED | 0x1L; // highMask("$,;:@&=+");

    // All valid characters for server-based authorities
    private static final long L_SERVER
            = L_USERINFO | L_ALPHANUM | L_DASH | 0x400400000000000L; // lowMask(".:@[]");
    private static final long H_SERVER
            = H_USERINFO | H_ALPHANUM | H_DASH | 0x28000001L; // highMask(".:@[]");

    // Special case of server authority that represents an IPv6 address
    // In this case, a % does not signify an escape sequence
    private static final long L_SERVER_PERCENT
            = L_SERVER | 0x2000000000L; // lowMask("%");
    private static final long H_SERVER_PERCENT
            = H_SERVER; // | highMask("%") == 0L;

    // scheme        = alpha *( alpha | digit | "+" | "-" | "." )
    private static final long L_SCHEME = L_ALPHA | L_DIGIT | 0x680000000000L; // lowMask("+-.");
    private static final long H_SCHEME = H_ALPHA | H_DIGIT; // | highMask("+-.") == 0L

    // scope_id = alpha | digit | "_" | "."
    private static final long L_SCOPE_ID
            = L_ALPHANUM | 0x400000000000L; // lowMask("_.");
    private static final long H_SCOPE_ID
            = H_ALPHANUM | 0x80000000L; // highMask("_.");

    // -- Escaping and encoding --

    private static final char[] hexDigits = {
            '0', '1', '2', '3', '4', '5', '6', '7',
            '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
    };

    private static void appendEscape(StringBuilder sb, byte b) {
        sb.append('%');
        sb.append(hexDigits[(b >> 4) & 0x0f]);
        sb.append(hexDigits[(b >> 0) & 0x0f]);
    }

    // -- Character classes for parsing --

    // RFC2396 precisely specifies which characters in the US-ASCII charset are
    // permissible in the various components of a URI reference.  We here
    // define a set of mask pairs to aid in enforcing these restrictions.  Each
    // mask pair consists of two longs, a low mask and a high mask.  Taken
    // together they represent a 128-bit mask, where bit i is set iff the
    // character with value i is permitted.
    //
    // This approach is more efficient than sequentially searching arrays of
    // permitted characters.  It could be made still more efficient by
    // precompiling the mask information so that a character's presence in a
    // given mask could be determined by a single table lookup.

    // To save startup time, we manually calculate the low-/highMask constants.
    // For reference, the following methods were used to calculate the values:

    // Compute the low-order mask for the characters in the given string
    //     private static long lowMask(String chars) {
    //        int n = chars.length();
    //        long m = 0;
    //        for (int i = 0; i < n; i++) {
    //            char c = chars.charAt(i);
    //            if (c < 64)
    //                m |= (1L << c);
    //        }
    //        return m;
    //    }

    // Compute the high-order mask for the characters in the given string
    //    private static long highMask(String chars) {
    //        int n = chars.length();
    //        long m = 0;
    //        for (int i = 0; i < n; i++) {
    //            char c = chars.charAt(i);
    //            if ((c >= 64) && (c < 128))
    //                m |= (1L << (c - 64));
    //        }
    //        return m;
    //    }

    // Compute a low-order mask for the characters
    // between first and last, inclusive
    //    private static long lowMask(char first, char last) {
    //        long m = 0;
    //        int f = Math.max(Math.min(first, 63), 0);
    //        int l = Math.max(Math.min(last, 63), 0);
    //        for (int i = f; i <= l; i++)
    //            m |= 1L << i;
    //        return m;
    //    }

    // Compute a high-order mask for the characters
    // between first and last, inclusive
    //    private static long highMask(char first, char last) {
    //        long m = 0;
    //        int f = Math.max(Math.min(first, 127), 64) - 64;
    //        int l = Math.max(Math.min(last, 127), 64) - 64;
    //        for (int i = f; i <= l; i++)
    //            m |= 1L << i;
    //        return m;
    //    }

    // Tell whether the given character is permitted by the given mask pair
    private static boolean match(char c, long lowMask, long highMask) {
        if (c == 0) // 0 doesn't have a slot in the mask. So, it never matches.
            return false;
        if (c < 64)
            return ((1L << c) & lowMask) != 0;
        if (c < 128)
            return ((1L << (c - 64)) & highMask) != 0;
        return false;
    }

    private static void appendEncoded(CharsetEncoder encoder, StringBuilder sb, char c) {
        ByteBuffer bb = null;
        try {
            bb = encoder.encode(CharBuffer.wrap("" + c));
        } catch (CharacterCodingException x) {
            assert false;
        }
        while (bb.hasRemaining()) {
            int b = bb.get() & 0xff;
            if (b >= 0x80)
                appendEscape(sb, (byte)b);
            else
                sb.append((char)b);
        }
    }

}
