package com.efurture.postgresql;

import com.efurture.util.CHStringUtil;
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
 * postgresql 版本14.0
 *
 * */
public class PostgresqlTest {

    @Test
    public void testPrepareTable() throws ClassNotFoundException, SQLException {
        Class.forName("org.postgresql.Driver");
        Connection connection = DriverManager.getConnection("jdbc:postgresql://127.0.0.1:5432/test", "postgres", "lovejava");
        Statement stmt = connection.createStatement();
        String sql = "";

        sql = "DROP TABLE IF EXISTS user_test;";
        sql += "-- Table: public.user_test\n" +
                "\n" +
                "DROP TABLE public.user_test;\n" +
                "\n" +
                "CREATE TABLE IF NOT EXISTS public.user_test\n" +
                "(\n" +
                "    id SERIAL  PRIMARY KEY,\n" +
                "    name character varying(64) COLLATE pg_catalog.\"default\",\n" +
                "    password character varying(63) COLLATE pg_catalog.\"default\",\n" +
                "    status integer,\n" +
                "    nick character varying(64) COLLATE pg_catalog.\"default\",\n" +
                "    uuid character varying(64) COLLATE pg_catalog.\"default\",\n" +
                "    message text COLLATE pg_catalog.\"default\",\n" +
                "    gmt_create timestamp without time zone,\n" +
                "    gmt_modified timestamp without time zone\n" +
                ")\n" +
                "\n" +
                "TABLESPACE pg_default;\n" +
                "\n" +
                "ALTER TABLE public.user_test\n" +
                "    OWNER to postgres;\n" +
                "-- Index: name_password_index\n" +
                "\n" +
                "-- DROP INDEX public.name_password_index;\n" +
                "\n" +
                "CREATE INDEX name_password_index\n" +
                "    ON public.user_test USING btree\n" +
                "    (name COLLATE pg_catalog.\"default\" ASC NULLS LAST, password COLLATE pg_catalog.\"default\" ASC NULLS LAST)\n" +
                "    TABLESPACE pg_default;";
        stmt.executeUpdate(sql);
        stmt.close();
        connection.close();
    }


    /**
     * 单线程 1 万 插入 2.8-3.520 秒，相当于每秒 2000-3000条
     * */
    @Test
    public void testInsertTableBenchInsert() throws ClassNotFoundException, SQLException {
        Class.forName("org.postgresql.Driver");
        Connection conn = DriverManager.getConnection("jdbc:postgresql://127.0.0.1:5432/test", "postgres", "lovejava");
        Statement stmt = null;
        try{
            stmt = conn.createStatement();

            int count = 10000;
            long start = System.currentTimeMillis();

            for(int i=0; i<count; i++) {
                String sql;
                sql = "INSERT INTO  user_test" +
                        "(" +
                        "name," +
                        "password," +
                        "status," +
                        "nick," +
                        "uuid," +
                        "message," +
                        "gmt_create," +
                        "gmt_modified)" +
                        " VALUES " +
                        "('"
                        + RandomStringUtils.randomAlphabetic(32) +
                        "','" + RandomStringUtils.randomAlphabetic(24) +
                        "','" + "0" +
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


    /**
     * 数据总量6万左右 总共4秒左右，相当于1秒钟1-1.5万
     * */
    @Test
    public void testInsertTableBenchInsertPools() throws ClassNotFoundException, SQLException, InterruptedException {
        Class.forName("org.postgresql.Driver");


        ThreadPoolExecutor executor = new ThreadPoolExecutor(16, 16, 8, TimeUnit.MINUTES,
                new LinkedBlockingDeque<Runnable>());


        long start = System.currentTimeMillis();
        for(int i=0; i<32; i++){
            Runnable runnable = new Runnable() {
                @Override
                public void run() {
                    try{
                        testInsertTableBenchInsert(2000);
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


    private void testInsertTableBenchInsert(int count) throws ClassNotFoundException, SQLException {
        Connection conn = DriverManager.getConnection("jdbc:postgresql://127.0.0.1:5432/test", "postgres", "lovejava");
        Statement stmt = null;
        try{
            stmt = conn.createStatement();

            long start = System.currentTimeMillis();

            for(int i=0; i<count; i++) {
                String sql;
                sql = "INSERT INTO  user_test" +
                        "(" +
                        "name," +
                        "password," +
                        "status," +
                        "nick," +
                        "uuid," +
                        "message," +
                        "gmt_create," +
                        "gmt_modified)" +
                        " VALUES " +
                        "('"
                        + RandomStringUtils.randomAlphabetic(32) +
                        "','" + RandomStringUtils.randomAlphabetic(24) +
                        "','" + "0" +
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


    /**
     * 单线程 4 秒左右，10万次查询 相当于2.5万次 1秒
     * */
    @Test
    public void testTableBenchPrepareSelect() throws ClassNotFoundException, SQLException {
        Class.forName("org.postgresql.Driver");
        Connection conn = null;
        PreparedStatement stmt = null;
        try{
            conn = DriverManager.getConnection("jdbc:postgresql://127.0.0.1:5432/test", "postgres", "lovejava");
            String sql = "select * from user_test where id = ?";
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

    /**
     * 多线程 6秒-7秒 总共64万，select 每秒9-10万的QPS
     * */
    @Test
    public void testTableBenchSelectPools() throws ClassNotFoundException, SQLException, InterruptedException {
        Class.forName("com.mysql.cj.jdbc.Driver");
        ThreadPoolExecutor executor = new ThreadPoolExecutor(16, 16, 8, TimeUnit.MINUTES,
                new LinkedBlockingDeque<Runnable>());


        long start = System.currentTimeMillis();
        for(int i=0; i<32; i++){
            Runnable runnable = new Runnable() {
                @Override
                public void run() {
                    try{
                        testTableBenchPrepareSelect(20000);
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

    private void testTableBenchPrepareSelect(int count) throws ClassNotFoundException, SQLException {
        Connection conn = null;
        PreparedStatement stmt = null;
        try{
            conn =  DriverManager.getConnection("jdbc:postgresql://127.0.0.1:5432/test", "postgres", "lovejava");
            String sql = "select * from user_test where id = ?";
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
}
