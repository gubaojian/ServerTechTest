package org.efurture.test;

import jdkx.util.compat.Base64;
import jdkx.util.compat.Compat;

import java.nio.charset.StandardCharsets;

//TIP To <b>Run</b> code, press <shortcut actionId="Run"/> or
// click the <icon src="AllIcons.Actions.Execute"/> icon in the gutter.
public class Main {
    public static void main(String[] args) {
        Compat.Math.addExact(0, 0);
        System.out.println(Base64.getEncoder().encodeToString("hello 中国啊师傅3".getBytes(StandardCharsets.UTF_8)));
        System.out.println(Base64.getUrlEncoder().encodeToString("hello 中国88".getBytes(StandardCharsets.UTF_8)));
    }
}