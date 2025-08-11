package org.jwt;

import io.jsonwebtoken.Claims;
import io.jsonwebtoken.Jwts;
import io.jsonwebtoken.SignatureAlgorithm;

import javax.crypto.spec.SecretKeySpec;
import java.security.Key;
import java.util.Date;
import java.util.HashMap;
import java.util.Map;
import java.util.UUID;

public class RsaMain {
    public static void main(String[] args) throws Exception {

        String secretKey = "i8zIHoTLy2t4uMIztIUi3vA129xYVKAE"; //RandomStringUtils.randomAlphanumeric(32); //32最小
        String connId = UUID.randomUUID().toString();
        String token = generateToken(
                connId, 0, secretKey);

        System.out.println(secretKey);
        System.out.println(connId);
        System.out.println(token);
        System.out.println(parseToken(token, secretKey));
    }

    /**
     * 生成JWT
     */
    public static String generateToken(String connId, long expireTime, String secretKey) {
        Key key = new SecretKeySpec(secretKey.getBytes(), SignatureAlgorithm.HS256.getJcaName());
        Map<String,String> claims = new HashMap<>();
        claims.put("connId", connId);
        if (expireTime <= 0) {
            return Jwts.builder()
                    .setClaims(claims)
                    .setIssuedAt(new Date(System.currentTimeMillis()))
                    .signWith(key, SignatureAlgorithm.RS256)
                    .compact();
        }
        return Jwts.builder()
                .setClaims(claims)
                .setIssuedAt(new Date(System.currentTimeMillis()))
                .setExpiration(new Date(System.currentTimeMillis() + expireTime))
                .signWith(key, SignatureAlgorithm.RS256)
                .compact();
    }

    /**
     * 解析JWT
     * @param token JWT字符串
     * @return 解析出的Claims
     * @throws Exception 解析失败时抛出异常
     */
    public static Claims parseToken(String token, String secretKey) throws Exception {
        Key key = new SecretKeySpec(secretKey.getBytes(), SignatureAlgorithm.HS256.getJcaName());
        System.out.println(""  + Jwts.parserBuilder()
                .setSigningKey(key)
                .build()
                .parseClaimsJws(token).getHeader());
        return Jwts.parserBuilder()
                .setSigningKey(key)
                .build()
                .parseClaimsJws(token)
                .getBody();
    }
}