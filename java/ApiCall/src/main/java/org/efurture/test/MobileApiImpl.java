package org.efurture.test;

import java.util.Map;

public class MobileApiImpl implements MobileApi {


    @Override
    public String hello() {
        System.out.println("hello method in");
        return "hello world";
    }

    @Override
    public String helloV2(String love) {
        System.out.println("hello method in22");
        return "hello worlddd ";
    }

    @Override
    public String helloV3(Map<String,Object> request) {
        return "hello worlddd ";
    }

    @Override
    public void hello4() {

    }
}
