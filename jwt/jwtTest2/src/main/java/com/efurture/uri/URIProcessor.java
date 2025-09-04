package com.efurture.uri;


import java.net.MalformedURLException;
import java.net.URI;
import java.net.URISyntaxException;
import java.net.URL;
import java.util.*;

/**
 * Helper class for building or manipulating URI references. Not safe for concurrent use.
 * */
public class URIProcessor {
    private final URI sourceUri;
    private final Map<String, List<String>> queryMap;
    private String overridePath;
    private String overrideFragment;

    public URIProcessor(URI sourceUri) {
        this.sourceUri = sourceUri;
        this.queryMap = new LinkedHashMap<>();
        this.parseQueryParametersToMap();
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
        } catch (URISyntaxException e) {
            throw new IllegalArgumentException(e);
        }
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
            int nextAmpersand = query.indexOf('&', start);
            int end = nextAmpersand != -1 ? nextAmpersand : query.length();

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
            if (nextAmpersand != -1) {
                start = nextAmpersand + 1;
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
        StringBuilder sb = new StringBuilder();
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

}
