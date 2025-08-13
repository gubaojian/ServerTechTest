package org.jwt;

import java.security.KeyPair;

public class CplusRsa {

    public static void main(String[] args) throws Exception {
        {
            String plainText = "hello world rsa";
            KeyPair keyPair = RsaUtil.generateKeyPair();
            String publicKey = RsaUtil.getPublicKeyStr(keyPair);
            System.out.println(RsaUtil.getPrivateKeyStr(keyPair));
            System.out.println(publicKey);
            System.out.println(RsaUtil.encrypt(plainText, publicKey));

        }
        {
            System.out.println("padding test");
            String plainText = "hello world rsa";
            String privateKey = "MIIEvQIBADANBgkqhkiG9w0BAQEFAASCBKcwggSjAgEAAoIBAQC3LsCB4ElKKMhbxlSO06ohn3wk7QblfdevC9Jlqw7qL5s4wk7s3VvNyyKLgtMqwchq4EtC9gL+vEzMivVyOgfS8IT5so3ZwJkjcMrvy6822+Qk44hBNKr6fYVugXhoExtjXKAN2xFWM4M7qMbGRvzDcAFi2eWde4pVY9KaPYi33KEkM41umLsWXv8zp60FKrleXTlhx6gV8wu4bLGbTCOjIjRPQM7Sel9zsW6tY/7tLrms0pRVG9esNp+/Hb77dcjI22PlaH3luNS5jiy5nGznmdo1+HseFVQ8RK9CnpsdTot5ajctE/ND52i/UpJ4MkpcYFd0B+9tsVLdcPE1cttHAgMBAAECggEABi7JyJhtYNdni9Fx4TA7uc7MPeLSLMTGlt3rSAsyAa2Bq7TDPTNredK1Y8gSoIrR3OmdILF1AGwSm4TFLAnRYclEc7kXGJZrlMqlztotG8joNmaHnsvMSZAbBNPR4JFkh0IFFPKj7yjl8QmNS/vxZBdPtKpuGZal4KPx48rX5ny5MBxjtaFW+eBK9Dla+1EqiuFHOU1uIibbNHDouS8g5wDFmRwFL+nrWgv9xSGj7BPfkFi0mpfGN6rTZeJQBlgzLCx4BRxcD8P62hOW0/sMiKORnSvDtWu9e6lA9qAidmzG8QyHkwz/u+BFFdbPB/PQUVw8uzVTJzvcwASumCRxYQKBgQDCrS1oPsazdixBedrADHbNP4njxrCCLZ9jJ5jazepgVCYSNGcM0YjHr6PfCAseEKT8BXpFDVVy5UR3jhZS+XasTVQl34cXUgZjIkFqSgLHPkeyo1p/QXi4HQNjBpQfpjrq5TpDyBLbfZ6Z8NLlXCtIcrVpdDrWP3jvj3MtFhEHjQKBgQDw4rQ6P3BIEE/PQd2u3HJvH8WqfHYdKQW6s4i1sSkLR2Je/my0H5m4o0LRtwG5xs3VPNcK2RdQAosIKtAcxVOm2u7qg0pZmbXYH5TfL/YsaPY72yM0qjda6xcZU4X1Yd6x9Xa08pgmBisRr/Uo2uNUpCn+RBOnPDjaSpXEfXffIwKBgHnXGxkoWQIOzun30uHpqx0QTEPDocsHtL7BFJi00aCSafVw2KIcLggUNHKtPRAHCMs2vmyjSLyNI0nUIsKxoQV7rFO7z3fX/WlkEh7szUpX/1WdiVEl7+EDP5BlmKUqS6uh5dJwUOUQfQgJwmSMSAaizEmA1iYrOYxtcn9gVS4tAoGBAMNXdMQPkeQ+phny1ezphDstTsR0bewzyhufX+vHoPsuhk12kXx4a9ZZPuSGPfYDjAOydMitR8Rwa4LSBTZvpuiWfkza7z498kMzSSy83ishax0bFi+tIXqvTmoRW36kQU2bOwp9+HhNZDvRr0PUTanj/tHdLvrdUVVkSpOvE7h1AoGAY6TqZi6YbNJ9sldm/VW+4icJ3J4i3SnWhTzX4f0wJ0yh7BrRNZ/00mYYG+VY1zfe/4p5kpbD5Qee7CrYsYu41/VX4z3qOlMXz8ht4E5zisqs+Z12LyCRe1te6hwiGGyx66Wg1ihM4Pntx3eGptcdWXVjde/dQNJCv4+F8TrCqLc=";

            String publicKey = "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAty7AgeBJSijIW8ZUjtOqIZ98JO0G5X3XrwvSZasO6i+bOMJO7N1bzcsii4LTKsHIauBLQvYC/rxMzIr1cjoH0vCE+bKN2cCZI3DK78uvNtvkJOOIQTSq+n2FboF4aBMbY1ygDdsRVjODO6jGxkb8w3ABYtnlnXuKVWPSmj2It9yhJDONbpi7Fl7/M6etBSq5Xl05YceoFfMLuGyxm0wjoyI0T0DO0npfc7FurWP+7S65rNKUVRvXrDafvx2++3XIyNtj5Wh95bjUuY4suZxs55naNfh7HhVUPESvQp6bHU6LeWo3LRPzQ+dov1KSeDJKXGBXdAfvbbFS3XDxNXLbRwIDAQAB";


            {
                System.out.println("PKCS1Padding");
                System.out.println(RsaUtil.encrypt(plainText, publicKey));
            }
            {
                String encryptOAEPPadding = RsaOAEPPaddingUtil.encrypt(plainText, publicKey);
                System.out.println(encryptOAEPPadding);
                System.out.println(RsaOAEPPaddingUtil.decrypt(encryptOAEPPadding, privateKey));
            }
            {
                String encryptOAEPPadding = RsaOAEPPaddingSha256Util.encrypt(plainText, publicKey);
                System.out.println(encryptOAEPPadding);
                System.out.println(RsaOAEPPaddingSha256Util.decrypt(encryptOAEPPadding, privateKey));
            }

            {
                String encryptOAEPPadding = RsaOAEPPaddingSha512Util.encrypt(plainText, publicKey);
                System.out.println(encryptOAEPPadding);
                System.out.println(RsaOAEPPaddingSha512Util.decrypt(encryptOAEPPadding, privateKey));
            }


            {
                String encryptOAEPPadding = RsaOAEPPaddingAllSha256Util.encrypt(plainText, publicKey);
                System.out.println(encryptOAEPPadding);
                System.out.println(RsaOAEPPaddingAllSha256Util.decrypt(encryptOAEPPadding, privateKey));
            }

        }
    }
}
