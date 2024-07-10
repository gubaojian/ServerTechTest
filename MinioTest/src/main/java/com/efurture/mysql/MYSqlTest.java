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
/**
 * mysql 版本 8.0
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


    /**
     * INSERT INTO  user(name,password,nick,status,uuid,message,gmt_create,gmt_modified) VALUES ('kdnxppyahxiIIIozQUmHbpHrAgQCqLpI','lhcOaIoGdUrHspRRjPiaqlGx','IgIWhBixOAvTBVoY',0,'3491c769-6743-419c-a182-6a5b6af3dcd0','攩撎縭玬惁庱燑期棁覧脵逵匠邬垖僾闿笑烙觿儧崮忹婱瓴貦衦聿兾揇稻邷赜膒稺蟯札搗訖慃疺圦犠譡婅玵錷脏溌嶚铩橂駶撽賾籈钮徇砷豵邛熲芔嫑耀甧靣啕葀媢醠铹礋漥伣伇氌畠遶诊宦儣搇欘雧亽呷賧蒠訇灿斤錌輻偸馛郩滜丱乐桒鱸亁炵鸑句朆萘耹炻弙馾缩祆璭豰锺蒯蕶弗囂僆薝疡凬繤堪俔厳魘栾楉駉恮胍鬅鱈鲦图酢甖裡尖鑟趕謆烧若篸嗶圳阅硠拵初秒絧楬絒鍟擦趷獕墁狩簼逘觇鬞庠鬦璑柭仯斻甶廵訋悪嵛冾丱尾晓简','2021-10-23 22:14:16','2021-10-23 22:14:16');
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

    @Test
    public void testPrepareTable() throws ClassNotFoundException, SQLException {
        Class.forName("com.mysql.cj.jdbc.Driver");
        Connection connection = getCollection();
        Statement stmt = connection.createStatement();
        String dropSql = "DROP TABLE IF EXISTS user;";
        stmt.executeUpdate(dropSql);

        String createTableSql = "CREATE TABLE `user` (\n" +
                "  `id` bigint NOT NULL AUTO_INCREMENT,\n" +
                "  `name` varchar(64) DEFAULT NULL,\n" +
                "  `password` varchar(32) DEFAULT NULL,\n" +
                "  `status` tinyint DEFAULT '0',\n" +
                "  `nick` varchar(64) DEFAULT NULL,\n" +
                "  `uuid` varchar(64) DEFAULT NULL,\n" +
                "  `message` text,\n" +
                "  `gmt_create` datetime DEFAULT NULL,\n" +
                "  `gmt_modified` datetime DEFAULT NULL,\n" +
                "  PRIMARY KEY (`id`),\n" +
                "  UNIQUE KEY `id_UNIQUE` (`id`),\n" +
                "  KEY `name_password` (`name`,`password`)\n" +
                ") ENGINE=InnoDB AUTO_INCREMENT=9993939 DEFAULT CHARSET=utf8;\n";
        stmt.executeUpdate(createTableSql);
        stmt.close();
        connection.close();
    }




    /**
     * mysql 多线程 9秒-10秒 select 每秒6万的QPS
     * postgresql 多线程 6秒-7秒 总共64万，select 每秒9-10万的QPS
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
     * mysql 单线程整体时间6-7秒， 10万次查询，相当于一次数据 1.5-2万
     * postgresql 单线程 4 秒左右，10万次查询 相当于2.5万次 1秒
     * */
    @Test
    public void testTableBenchPrepareSelect() throws ClassNotFoundException, SQLException {

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

    /**
     * 插入6-7秒左右
     * */
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
     * mysql  每秒1的1万的QPS 6万条6秒左右， 数据6万左右
     * postgresql 数据总量6万左右 总共4秒左右，相当于1秒钟1-1.5万
     * */
    @Test
    public void testInsertTableBenchInsertPools() throws ClassNotFoundException, SQLException, InterruptedException {
        Class.forName("com.mysql.cj.jdbc.Driver");


        ThreadPoolExecutor executor = new ThreadPoolExecutor(16, 16, 8, TimeUnit.MINUTES,
                new LinkedBlockingDeque<Runnable>());


        long start = System.currentTimeMillis();
        for(int i=0; i<320; i++){
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

    /**
     * mysql : 单线程插入 1万条 插入性能 5-6秒左右，
     * postgresql. 单线程 1 万 插入 2.8-3.520 秒，相当于每秒 2000-3000条
     * */
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
