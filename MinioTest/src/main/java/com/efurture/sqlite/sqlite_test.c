#include <stdio.h>
#include <sqlite3.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdint.h>

static int callback(void *NotUsed, int argc, char **argv, char **azColName){
    int i;
    for(i=0; i<argc; i++){
      printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("hello world\n");
    return 0;
}

static int64_t time_now_ms() {
   struct timeval start;
   gettimeofday(&start, NULL);
   return (start.tv_sec*1000 + start.tv_usec/1000);
}
  
int main(int argc, char **argv){
    sqlite3 *db;
    char *zErrMsg = 0;
    int rc;
    rc = sqlite3_open("test.db", &db);
    if( rc ){
      fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
      sqlite3_close(db);
      return(1);
    }
    
    char* dropTableSql = "DROP TABLE IF EXISTS user;"; 
    rc = sqlite3_exec(db, dropTableSql, callback, 0, &zErrMsg);
    if( rc!=SQLITE_OK ){
      fprintf(stderr, "SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
    }
    
    char createTableSql[512];
    
    snprintf(createTableSql, sizeof(createTableSql), "%s%s%s%s%s%s%s%s%s%s%s%s", "DROP TABLE IF EXISTS user;",
                 "CREATE TABLE IF NOT EXISTS user(",
                 "id INTEGER PRIMARY KEY NOT NULL,",
                 "name TEXT NOT NULL,",
                 "password TEXT NOT NULL,",
                 "status INTEGER NOT NULL,",
                 "nick TEXT NOT NULL,",
                 "uuid TEXT NOT NULL,",
                 "message TEXT NOT NULL,",
                 "gmt_create INTEGER NOT NULL,",
                 "gmt_modified INTEGER NOT NULL",
                 ");");
    
    rc = sqlite3_exec(db, createTableSql, callback, 0, &zErrMsg);
    
    char* insertSql = "INSERT INTO  user(name,password,nick,status,uuid,message,gmt_create,gmt_modified) VALUES ('kdnxppyahxiIIIozQUmHbpHrAgQCqLpI','lhcOaIoGdUrHspRRjPiaqlGx','IgIWhBixOAvTBVoY',0,'3491c769-6743-419c-a182-6a5b6af3dcd0','攩撎縭玬惁庱燑期棁覧脵逵匠邬垖僾闿笑烙觿儧崮忹婱瓴貦衦聿兾揇稻邷赜膒稺蟯札搗訖慃疺圦犠譡婅玵錷脏溌嶚铩橂駶撽賾籈钮徇砷豵邛熲芔嫑耀甧靣啕葀媢醠铹礋漥伣伇氌畠遶诊宦儣搇欘雧亽呷賧蒠訇灿斤錌輻偸馛郩滜丱乐桒鱸亁炵鸑句朆萘耹炻弙馾缩祆璭豰锺蒯蕶弗囂僆薝疡凬繤堪俔厳魘栾楉駉恮胍鬅鱈鲦图酢甖裡尖鑟趕謆烧若篸嗶圳阅硠拵初秒絧楬絒鍟擦趷獕墁狩簼逘觇鬞庠鬦璑柭仯斻甶廵訋悪嵛冾丱尾晓简','2021-10-23 22:14:16','2021-10-23 22:14:16');";

    char* selectSql = "select * from user limit 1;";
     rc = sqlite3_exec(db, "PRAGMA journal_mode=WAL;PRAGMA synchronous = 0; ", callback, 0, &zErrMsg);
     if( rc!=SQLITE_OK ){
      fprintf(stderr, "SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
    }
     
     struct timeval start;
     
     gettimeofday(&start, NULL);
     
     printf("s: %ld ms: %ld ", start.tv_sec, (start.tv_sec*1000 + start.tv_usec/1000) );

     int64_t start_time = time_now_ms();
     
     for(int i=0; i < 100000; i++) {
         rc = sqlite3_exec(db, insertSql, callback, 0, &zErrMsg);
         if( rc!=SQLITE_OK ){
	      fprintf(stderr, "SQL error: %s\n", zErrMsg);
	      sqlite3_free(zErrMsg);
	    }
     }
     
     int64_t end_time = time_now_ms();
     
     printf("used %ld ms ", (end_time - start_time));


    sqlite3_close(db);
    return 0;
}
