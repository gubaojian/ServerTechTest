package com.efurture.mysql;

import com.efurture.util.CHStringUtil;
import junit.framework.TestCase;
import org.apache.commons.lang3.RandomStringUtils;
import org.apache.commons.lang3.RandomUtils;
import org.apache.commons.lang3.time.DateFormatUtils;
import org.junit.Test;

import java.sql.*;
import java.util.Date;
import java.util.UUID;
import java.util.concurrent.LinkedBlockingDeque;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;


/**
 *
 * mysql 单个表1000万数据影响不大，查询速度依旧很快。
 *
 * count时候注意，限制最大页数。设计时候通过隐藏总页数或者缓存总页数来处理。
 * 通过上一页，下一页，来处理。也可以通过id来限制。
 *
 * 页数超过50-10万就比较难以处理啦。
 *
 * 10000万条数据 20G 左右
 *
 * 尽量把文件单独独立存储起来。
 *
 *
 * */
public class MYSqlTest  extends TestCase {


    /**
     * CREATE TABLE `user` (
     *   `id` bigint NOT NULL AUTO_INCREMENT,
     *   `name` varchar(64) DEFAULT NULL,
     *   `passwrod` varchar(32) DEFAULT NULL,
     *   `nick` varchar(64) DEFAULT NULL,
     *   `uuid` varchar(64) DEFAULT NULL,
     *   `message` varchar(45) DEFAULT NULL,
     *   `gmt_create` datetime DEFAULT NULL,
     *   `gmt_modified` datetime DEFAULT NULL,
     *   PRIMARY KEY (`id`)
     * ) ENGINE=InnoDB DEFAULT CHARSET=utf8;
     * */


    @Test
    public void testShowSql(){
        String sql;
        sql = "INSERT INTO  user" +
                "(id," +
                "name," +
                "password," +
                "nick," +
                "uuid," +
                "message," +
                "gmt_create," +
                "gmt_modified)" +
                " VALUES " +
                "(null,'"
                + RandomStringUtils.randomAlphabetic(32) +
                "','" + RandomStringUtils.randomAlphabetic(24) +
                "','" + RandomStringUtils.randomAlphabetic(16) +
                "','" + UUID.randomUUID().toString() +
                "','" + CHStringUtil.randomString(128, 1024) +
                "','" + DateFormatUtils.format(new Date(), "yyyy-MM-dd HH:mm:ss") +
                "','" + DateFormatUtils.format(new Date(), "yyyy-MM-dd HH:mm:ss") +
                "');";
        System.out.println(sql);
    }



    /**
     * select 每秒6-10万的QPS
     * */
    @Test
    public void testInsertTableBenchSelectPools() throws ClassNotFoundException, SQLException, InterruptedException {
        Class.forName("com.mysql.cj.jdbc.Driver");


        ThreadPoolExecutor executor = new ThreadPoolExecutor(16, 16, 8, TimeUnit.MINUTES,
                new LinkedBlockingDeque<Runnable>());


        long start = System.currentTimeMillis();
        for(int i=0; i<32; i++){
            Runnable runnable = new Runnable() {
                @Override
                public void run() {
                    try{
                        testInsertTableBenchPrepareSelect(20000);
                    }catch (Exception e){
                        e.printStackTrace();
                    }
                }
            };
            executor.execute(runnable);
        }
        executor.shutdown();
        executor.awaitTermination(10, TimeUnit.DAYS);
        System.out.println("pool bench insert used " + (System.currentTimeMillis() - start));

    }

    @Test
    public void testInsertTableBenchPrepareSelect(int count) throws ClassNotFoundException, SQLException {

        Class.forName("com.mysql.cj.jdbc.Driver");
        Connection conn = null;
        PreparedStatement stmt = null;
        try{
            conn = getCollection();
            String sql = "select * from user where id = ?";
            stmt = conn.prepareStatement(sql);


            long start = System.currentTimeMillis();

            for(int i=0; i<count; i++) {
                stmt.setLong(1, RandomUtils.nextLong(2l, 300*10000l));
                ResultSet rs =  stmt.executeQuery();
                while (rs.next()){
                    long id  = rs.getLong(1);
                }
                rs.close();

            }

            System.out.println("parpare bench select used " + (System.currentTimeMillis() - start));
            // 完成后关闭
        }catch (Exception e){
            e.printStackTrace();
        }finally {
            if(stmt != null){
                stmt.close();
            }
            if(conn != null){
                conn.close();
            }
        }
    }

    /**
     * select 每秒6-10万的QPS
     * */
    @Test
    public void testInsertTableBenchPrepareSelect() throws ClassNotFoundException, SQLException {

        Class.forName("com.mysql.cj.jdbc.Driver");
        Connection conn = null;
        PreparedStatement stmt = null;
        try{
            conn = getCollection();
            String sql = "select * from user where id = ?";
            stmt = conn.prepareStatement(sql);


            int count = 100000;
            long start = System.currentTimeMillis();

            for(int i=0; i<count; i++) {
                stmt.setLong(1, RandomUtils.nextLong(2l, 300*10000l));
                ResultSet rs =  stmt.executeQuery();
                while (rs.next()){
                    long id  = rs.getLong(1);
                }
                rs.close();

            }

            System.out.println("parpare bench select used " + (System.currentTimeMillis() - start));
            // 完成后关闭
        }catch (Exception e){
            e.printStackTrace();
        }finally {
            if(stmt != null){
                stmt.close();
            }
            if(conn != null){
                conn.close();
            }
        }
    }

    @Test
    public void testInsertTableBenchPrepareInsert() throws ClassNotFoundException, SQLException {

        Class.forName("com.mysql.cj.jdbc.Driver");
        Connection conn = null;
        PreparedStatement stmt = null;
        try{
            conn = getCollection();
            String sql = "INSERT INTO  user(id,name,password,nick,uuid,message,gmt_create,gmt_modified) VALUES (null,?, ?, ?, ?, ?,?,?)";
            stmt = conn.prepareStatement(sql);


            int count = 10000;
            long start = System.currentTimeMillis();

            for(int i=0; i<count; i++) {
                stmt.setString(1, RandomStringUtils.randomAlphabetic(32));
                stmt.setString(2, RandomStringUtils.randomAlphabetic(24));
                stmt.setString(3, RandomStringUtils.randomAlphabetic(16));
                stmt.setString(4, UUID.randomUUID().toString());
                stmt.setString(5, CHStringUtil.randomString(128, 1024));
                stmt.setDate(6, new java.sql.Date(new Date().getTime()));
                stmt.setDate(7, new java.sql.Date(new Date().getTime()));
                boolean rs =  stmt.execute();

            }

            System.out.println("parpare bench inser used " + (System.currentTimeMillis() - start));
            // 完成后关闭
        }catch (Exception e){
            e.printStackTrace();
        }finally {
            if(stmt != null){
                stmt.close();
            }
            if(conn != null){
                conn.close();
            }
        }
    }


    /**
     * 每秒1-2万的QPS
     * */
    @Test
    public void testInsertTableBenchInsertPools() throws ClassNotFoundException, SQLException, InterruptedException {
        Class.forName("com.mysql.cj.jdbc.Driver");


        ThreadPoolExecutor executor = new ThreadPoolExecutor(16, 16, 8, TimeUnit.MINUTES,
                new LinkedBlockingDeque<Runnable>());


        long start = System.currentTimeMillis();
        for(int i=0; i<32; i++){
            Runnable runnable = new Runnable() {
                @Override
                public void run() {
                    try{
                        testInsertTableBenchInsert(20000);
                    }catch (Exception e){
                        e.printStackTrace();
                    }
                }
            };
            executor.execute(runnable);
        }
        executor.shutdown();
        executor.awaitTermination(10, TimeUnit.DAYS);
        System.out.println("pool bench insert used " + (System.currentTimeMillis() - start));

    }

    @Test
    public void testInsertTableBenchInsert(int count) throws ClassNotFoundException, SQLException {
        Connection conn = null;
        Statement stmt = null;
        try{
            conn = getCollection();
            stmt = conn.createStatement();

            long start = System.currentTimeMillis();

            for(int i=0; i<count; i++) {
                String sql;
                sql = "INSERT INTO  user" +
                        "(id," +
                        "name," +
                        "password," +
                        "nick," +
                        "uuid," +
                        "message," +
                        "gmt_create," +
                        "`gmt_modified`)" +
                        " VALUES " +
                        "(null,'"
                        + RandomStringUtils.randomAlphabetic(32) +
                        "','" + RandomStringUtils.randomAlphabetic(24) +
                        "','" + RandomStringUtils.randomAlphabetic(16) +
                        "','" + UUID.randomUUID().toString() +
                        "','" + CHStringUtil.randomString(128, 1024) +
                        "','" + DateFormatUtils.format(new Date(), "yyyy-MM-dd HH:mm:ss") +
                        "','" + DateFormatUtils.format(new Date(), "yyyy-MM-dd HH:mm:ss") +
                        "');";

                boolean rs = stmt.execute(sql);
            }

            System.out.println("bench insert used " + (System.currentTimeMillis() - start));
            // 完成后关闭
        }catch (Exception e){
            e.printStackTrace();
        }finally {
            if(stmt != null){
                stmt.close();
            }
            if(conn != null){
                conn.close();
            }
        }
    }

    @Test
    public void testInsertTableBenchInsert() throws ClassNotFoundException, SQLException {
        Class.forName("com.mysql.cj.jdbc.Driver");
        Connection conn = null;
        Statement stmt = null;
        try{
            conn = getCollection();
            stmt = conn.createStatement();

            int count = 10000;
            long start = System.currentTimeMillis();

            for(int i=0; i<count; i++) {
                String sql;
                sql = "INSERT INTO  user" +
                        "(id," +
                        "name," +
                        "password," +
                        "nick," +
                        "uuid," +
                        "message," +
                        "gmt_create," +
                        "`gmt_modified`)" +
                        " VALUES " +
                        "(null,'"
                        + RandomStringUtils.randomAlphabetic(32) +
                        "','" + RandomStringUtils.randomAlphabetic(24) +
                        "','" + RandomStringUtils.randomAlphabetic(16) +
                        "','" + UUID.randomUUID().toString() +
                        "','" + CHStringUtil.randomString(128, 1024) +
                        "','" + DateFormatUtils.format(new Date(), "yyyy-MM-dd HH:mm:ss") +
                        "','" + DateFormatUtils.format(new Date(), "yyyy-MM-dd HH:mm:ss") +
                        "');";

                boolean rs = stmt.execute(sql);
            }

            System.out.println("bench inser used " + (System.currentTimeMillis() - start));
            // 完成后关闭
        }catch (Exception e){
            e.printStackTrace();
        }finally {
            if(stmt != null){
                stmt.close();
            }
            if(conn != null){
                conn.close();
            }
        }
    }

    @Test
    public void testInsertTable() throws ClassNotFoundException, SQLException {
        Class.forName("com.mysql.cj.jdbc.Driver");
        Connection conn = null;
        Statement stmt = null;
        try{
            conn = getCollection();
            stmt = conn.createStatement();
            String sql;
            sql = "INSERT INTO  user" +
                    "(id," +
                    "name," +
                    "password," +
                    "nick," +
                    "uuid," +
                    "message," +
                    "gmt_create," +
                    "`gmt_modified`)" +
                    " VALUES " +
                    "(null,'"
                    + RandomStringUtils.randomAlphabetic(32) +
                    "','" + RandomStringUtils.randomAlphabetic(24) +
                    "','" + RandomStringUtils.randomAlphabetic(16) +
                    "','" + UUID.randomUUID().toString() +
                    "','" + CHStringUtil.randomString(128, 1024) +
                    "','" + DateFormatUtils.format(new Date(), "yyyy-MM-dd HH:mm:ss") +
                    "','" + DateFormatUtils.format(new Date(), "yyyy-MM-dd HH:mm:ss") +
                    "');";
            System.out.println(sql);
            boolean rs = stmt.execute(sql);


            // 完成后关闭
        }catch (Exception e){
            e.printStackTrace();
        }finally {
            if(stmt != null){
                stmt.close();
            }
            if(conn != null){
                conn.close();
            }
        }
     }



    @Test
    public void testExampleSelect() throws ClassNotFoundException, SQLException {
        Class.forName("com.mysql.cj.jdbc.Driver");
        Connection conn = null;
        Statement stmt = null;
        try{
            conn = getCollection();
            stmt = conn.createStatement();
            String sql;
            sql = "SELECT id, name, url FROM websites";
            ResultSet rs = stmt.executeQuery(sql);

            // 展开结果集数据库
            while(rs.next()){
                // 通过字段检索
                int id  = rs.getInt("id");
                String name = rs.getString("name");
                String url = rs.getString("url");

                // 输出数据
                System.out.print("ID: " + id);
                System.out.print(", 站点名称: " + name);
                System.out.print(", 站点 URL: " + url);
                System.out.print("\n");
            }
            // 完成后关闭
            rs.close();
        }catch (Exception e){
            e.printStackTrace();
        }finally {
            if(stmt != null){
                stmt.close();
            }
            if(conn != null){
                conn.close();
            }
        }
    }


    private Connection getCollection() throws SQLException {
        String user = "root";
        String password = "Lovejava";
        Connection conn = DriverManager.getConnection("jdbc:mysql://localhost:3306/test_demo?useSSL=false&allowPublicKeyRetrieval=true&serverTimezone=UTC",
                user,password);
        return  conn;
    }
}
