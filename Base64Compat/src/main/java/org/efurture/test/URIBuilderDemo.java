package org.efurture.test;

import jdkx.compat.uri.URIProcessor;

public class URIBuilderDemo {

    public static void main(String[] args) {
        {
            URIProcessor processor = URIProcessor.fromURI("https://a.com:8080/api?name=test&id=1");
            System.out.println(processor.getQueryParameter("name"));
        }
        {
            URIProcessor processor = URIProcessor.fromURI("https://a.com:8080/api?name=test=&id=1=");
            System.out.println(processor.getQueryParameter("name"));
            System.out.println(processor.getQueryParameter("id"));
        }
        {
            URIProcessor processor = URIProcessor.fromURI("https://a.com:8080/api?name=test&id=1");
            processor.setQueryParameter("name&", "update&name");
            System.out.println(processor.getQueryParameter("name"));
        }

        {
            URIProcessor processor = URIProcessor.fromURI("https://a.com:8080/api?name=test&id=1&name%2526=update%2526name");
            System.out.println(processor.getQueryParameter("name&"));
        }

        {
            URIProcessor processor = URIProcessor.fromURI("https://test.com/shop?goods=手机&brand=华为");
            System.out.println(processor.getQueryParameter("goods"));
            System.out.println(processor.getSourceUri().getRawQuery());
            System.out.println(processor.getSourceUri().getQuery());
            System.out.println(processor.toURI());
        }

        {
            URIProcessor processor = URIProcessor.fromURL("https://test.com/shop?goods=手机&brand=华为");
            System.out.println(processor.getQueryParameter("goods"));
            System.out.println(processor.toURL());
        }

        {
            URIProcessor processor = URIProcessor.fromURL("https://test.com/shop?goods=%25E6%2589%258B%25E6%259C%25BA&brand=%25E5%258D%258E%25E4%25B8%25BA");
            System.out.println(processor.getQueryParameter("goods"));
            System.out.println(processor.toURL());
        }

        {
            URIProcessor processor = URIProcessor.fromURI("https://a.com:8080/api?name=test&name=test&id=1");
            processor.setQueryParameter("name&", "update&name");
            System.out.println(processor.getQueryParameters("name"));
        }

    }
}
