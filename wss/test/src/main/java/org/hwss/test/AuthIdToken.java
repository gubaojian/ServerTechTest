package org.hwss.test;

import org.apache.commons.lang3.RandomStringUtils;

import java.util.UUID;

public class AuthIdToken {

    public static void main(String[] args) {
        System.out.println(RandomStringUtils.randomNumeric(12));
        System.out.println(RandomStringUtils.randomAlphanumeric(32));


        System.out.println(RandomStringUtils.randomNumeric(12));
        System.out.println(RandomStringUtils.randomAlphanumeric(32));



        System.out.println(UUID.randomUUID().toString());
        System.out.println(RandomStringUtils.randomAlphanumeric(32));




    }
}
