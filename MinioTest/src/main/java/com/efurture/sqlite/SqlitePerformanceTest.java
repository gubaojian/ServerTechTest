package com.efurture.sqlite;

import com.efurture.util.CHStringUtil;
import org.apache.commons.lang3.RandomStringUtils;
import org.apache.commons.lang3.RandomUtils;
import org.apache.commons.lang3.time.DateFormatUtils;
import org.junit.After;
import org.junit.Before;
import org.junit.Test;

import java.sql.*;
import java.util.UUID;
import java.util.concurrent.LinkedBlockingDeque;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;

public class SqlitePerformanceTest {

    @Before
    public void setUP() {

    }

    @Test
    public void testInsert(){

    }

    /**
     * 10万3秒， 服务端机器好些性能更高
     * */
    @Test
    public void testMemoryTableBenchPrepareInsert() throws SQLException, ClassNotFoundException {
        Class.forName("org.sqlite.JDBC");
        Connection connection = DriverManager.getConnection("jdbc:sqlite::memory:");
        Statement stmt = connection.createStatement();
        String createSql = "";

        createSql = "DROP TABLE IF EXISTS user;"
                + "CREATE TABLE IF NOT EXISTS user("
                + "id INTEGER PRIMARY KEY NOT NULL,"
                + "name TEXT NOT NULL,"
                + "password TEXT NOT NULL,"
                + "status INTEGER NOT NULL,"
                + "nick TEXT NOT NULL,"
                + "uuid TEXT NOT NULL,"
                + "message TEXT NOT NULL,"
                + "gmt_create INTEGER NOT NULL,"
                + "gmt_modified INTEGER NOT NULL"
                + ");"
                + "CREATE INDEX  user_name_password_index ON user (name, password);";
        stmt.executeUpdate(createSql);
        stmt.close();
        PreparedStatement insertStmt = null;
        String insertSql = "INSERT INTO  user(name,password,status,nick,uuid,message,gmt_create,gmt_modified) VALUES (?, ?, 0, ?, ?, ?,?,?)";
        insertStmt = connection.prepareStatement(insertSql);
        int count = 100000;
        long start = System.currentTimeMillis();

        for(int i=0; i<count; i++) {
            insertStmt.setString(1, RandomStringUtils.randomAlphabetic(32));
            insertStmt.setString(2, RandomStringUtils.randomAlphabetic(24));
            insertStmt.setString(3, RandomStringUtils.randomAlphabetic(16));
            insertStmt.setString(4, UUID.randomUUID().toString());
            insertStmt.setString(5, CHStringUtil.randomString(128, 1024));
            insertStmt.setDate(6, new java.sql.Date(new java.util.Date().getTime()));
            insertStmt.setDate(7, new java.sql.Date(new java.util.Date().getTime()));
            boolean rs =  insertStmt.execute();

        }

        System.out.println("parpare bench inser used " + (System.currentTimeMillis() - start) + "ms");

        insertStmt.close();
        connection.close();
    }


    @Test
    public void testPrepareTable() throws ClassNotFoundException, SQLException {
        Class.forName("org.sqlite.JDBC");
        Connection connection = DriverManager.getConnection("jdbc:sqlite:test.db");
        Statement stmt = connection.createStatement();
        String sql = "";

        sql = "DROP TABLE IF EXISTS user;"
            + "CREATE TABLE IF NOT EXISTS user("
            + "id INTEGER PRIMARY KEY NOT NULL,"
            + "name TEXT NOT NULL,"
            + "password TEXT NOT NULL,"
            + "status INTEGER NOT NULL,"
            + "nick TEXT NOT NULL,"
            + "uuid TEXT NOT NULL,"
            + "message TEXT NOT NULL,"
            + "gmt_create INTEGER NOT NULL,"
            + "gmt_modified INTEGER NOT NULL"
            + ");"
            + "CREATE INDEX  user_name_password_index ON user (name, password);";
        stmt.executeUpdate(sql);
        stmt.close();
        connection.close();
    }


    /**
     * 1 - 1.5s 总共 10万次查询
     * 二次测试2.3秒，相当于每秒2万次查询
     * */
    @Test
    public void testTableBenchPrepareSelect() throws ClassNotFoundException, SQLException {

        Class.forName("org.sqlite.JDBC");
        Connection conn = DriverManager.getConnection("jdbc:sqlite:test.db");
        conn.createStatement().execute("PRAGMA synchronous = 0;PRAGMA cache_size=-2000000");

        PreparedStatement stmt = null;
        try{
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

    /**
     * 同步 6-7秒 每秒2000左右
     * 异步 3秒，相当于每秒3000
     * 关键还是异步
     * */
    @Test
    public void testInsertTableBenchPrepareInsert() throws SQLException, ClassNotFoundException {
        Class.forName("org.sqlite.JDBC");
        Connection connection = DriverManager.getConnection("jdbc:sqlite:test.db");

        connection.createStatement().execute("PRAGMA synchronous = OFF;");
        PreparedStatement stmt = null;
        String sql = "INSERT INTO  user(name,password,status,nick,uuid,message,gmt_create,gmt_modified) VALUES (?, ?, 0, ?, ?, ?,?,?)";
        stmt = connection.prepareStatement(sql);
        int count = 10000;
        long start = System.currentTimeMillis();

        for(int i=0; i<count; i++) {
            stmt.setString(1, RandomStringUtils.randomAlphabetic(32));
            stmt.setString(2, RandomStringUtils.randomAlphabetic(24));
            stmt.setString(3, RandomStringUtils.randomAlphabetic(16));
            stmt.setString(4, UUID.randomUUID().toString());
            stmt.setString(5, CHStringUtil.randomString(128, 1024));
            stmt.setDate(6, new java.sql.Date(new java.util.Date().getTime()));
            stmt.setDate(7, new java.sql.Date(new java.util.Date().getTime()));
            boolean rs =  stmt.execute();

        }

        System.out.println("parpare bench inser used " + (System.currentTimeMillis() - start));

        stmt.close();
        connection.close();
    }

    /**
     * 同步 6-7秒 每秒2000左右
     * 异步 3秒，相当于每秒3000
     *
     * */
    @Test
    public void testWALTableBenchPrepareInsert() throws SQLException, ClassNotFoundException {
        Class.forName("org.sqlite.JDBC");
        Connection connection = DriverManager.getConnection("jdbc:sqlite:test.db");

        connection.createStatement().execute("PRAGMA synchronous = OFF;PRAGMA cache_size=-200000");
        PreparedStatement stmt = null;
        String sql = "INSERT INTO  user(name,password,status,nick,uuid,message,gmt_create,gmt_modified) VALUES (?, ?, 0, ?, ?, ?,?,?)";
        stmt = connection.prepareStatement(sql);
        int count = 10000;
        long start = System.currentTimeMillis();

        for(int i=0; i<count; i++) {
            stmt.setString(1, RandomStringUtils.randomAlphabetic(32));
            stmt.setString(2, RandomStringUtils.randomAlphabetic(24));
            stmt.setString(3, RandomStringUtils.randomAlphabetic(16));
            stmt.setString(4, UUID.randomUUID().toString());
            stmt.setString(5, CHStringUtil.randomString(128, 1024));
            stmt.setDate(6, new java.sql.Date(new java.util.Date().getTime()));
            stmt.setDate(7, new java.sql.Date(new java.util.Date().getTime()));
            boolean rs =  stmt.execute();

        }

        System.out.println("parpare bench inser used " + (System.currentTimeMillis() - start));

        stmt.close();
        connection.close();
    }



    /**
     * select 每秒8-10万的QPS,
     * */
    @Test
    public void testTableBenchSelectPools() throws ClassNotFoundException, SQLException, InterruptedException {
        Class.forName("org.sqlite.JDBC");
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
        Connection conn = DriverManager.getConnection("jdbc:sqlite:test.db");
        PreparedStatement stmt = null;
        try{
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
     * 多线程不推荐写，
     * */
    @Test
    public void testInsertTableBenchInsertPools() throws ClassNotFoundException, SQLException, InterruptedException {
        Class.forName("org.sqlite.JDBC");
        final Connection conn = DriverManager.getConnection("jdbc:sqlite:test.db");

        ThreadPoolExecutor executor = new ThreadPoolExecutor(16, 16, 8, TimeUnit.MINUTES,
                new LinkedBlockingDeque<Runnable>());


        long start = System.currentTimeMillis();
        for(int i=0; i<32; i++){
            Runnable runnable = new Runnable() {
                @Override
                public void run() {
                    try{
                        testInsertTableBenchInsert(2000, conn);
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
        conn.close();
    }



    private void testInsertTableBenchInsert(int count, Connection conn) throws ClassNotFoundException, SQLException {
        Statement stmt = null;
        try{
            stmt = conn.createStatement();

            long start = System.currentTimeMillis();

            for(int i=0; i<count; i++) {
                String sql;
                sql = "INSERT INTO  user" +
                        "(" +
                        "name," +
                        "password," +
                        "status," +
                        "nick," +
                        "uuid," +
                        "message," +
                        "gmt_create," +
                        "`gmt_modified`)" +
                        " VALUES " +
                        "('"
                        + RandomStringUtils.randomAlphabetic(32) +
                        "','" + RandomStringUtils.randomAlphabetic(24) +
                        "','" + "0" +
                        "','" + RandomStringUtils.randomAlphabetic(16) +
                        "','" + UUID.randomUUID().toString() +
                        "','" + CHStringUtil.randomString(128, 1024) +
                        "','" + DateFormatUtils.format(new java.util.Date(), "yyyy-MM-dd HH:mm:ss") +
                        "','" + DateFormatUtils.format(new java.util.Date(), "yyyy-MM-dd HH:mm:ss") +
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
        }
    }

    @After
    public void tearDown() {

    }
}
