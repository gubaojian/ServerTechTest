package org.efurture.test;
//TIP To <b>Run</b> code, press <shortcut actionId="Run"/> or
// click the <icon src="AllIcons.Actions.Execute"/> icon in the gutter.
public class Main {
    public static void main(String[] args) {
        String touch = "33-33*33";
        var result = touch.split("[-x*+]");
        for(int i=0; i<result.length; i++) {
            System.out.println(result[i]);
        }

    }

    public static String[] getTouchs(String touch) {
        String[] results = touch.split("[x*+]");
        for(int i=0; i<results.length; i++) {
            if (results[i] != null) {
                results[i] = results[i].trim();
            }
        }
        return results;
    }
}