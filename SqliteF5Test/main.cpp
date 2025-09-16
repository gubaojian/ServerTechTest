#include <iostream>

#include "sqlite3.h"

#include <iostream>
#include <string>
#include "sqlite3.h"

// 回调函数：处理查询结果
static int callback(void* data, int argc, char**argv, char** azColName) {
    std::string* action = static_cast<std::string*>(data);
    if (action && !action->empty()) {
        std::cout << "\n" << *action << " 结果：" << std::endl;
        *action = ""; // 仅打印一次提示
    }

    for (int i = 0; i < argc; i++) {
        std::cout << azColName[i] << " = " << (argv[i] ? argv[i] : "NULL") << " | ";
    }
    std::cout << std::endl;
    return 0;
}

int main() {
    sqlite3* db;
    char* errMsg = nullptr;
    int rc;
    std::string sql;
    std::string tableName = "docs_fts5"; // FTS5表名

    // 删除旧数据库文件（确保测试环境干净）
    std::remove("fts5_test.db");

    // 1. 打开数据库
    rc = sqlite3_open("fts5_test.db", &db);
    if (rc) {
        std::cerr << "无法打开数据库: " << sqlite3_errmsg(db) << std::endl;
        return rc;
    } else {
        std::cout << "成功打开数据库" << std::endl;
    }

    // 2. 创建FTS5全文索引表
    // FTS5表本质是虚拟表，用于全文检索，字段通常包含需要索引的文本内容
    sql = "CREATE VIRTUAL TABLE IF NOT EXISTS " + tableName +
        " USING fts5(id, title, content);"; // id为文档标识，title和content为索引字段

    rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "创建FTS5表失败: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    } else {
        std::cout << "成功创建FTS5全文索引表 " << tableName << std::endl;
    }

    // 3. 插入测试数据（适合全文检索的文本内容）
    sql = "INSERT INTO " + tableName + " (id, title, content) VALUES "
          "(1, 'SQLite简介', 'SQLite 是一款轻量级的嵌入式数据库com.taobao.efurture，支持多种操作系统Java-Websocket'),"
          "(2, 'FTS5功能', 'FTS5 是 SQLite 的 全文 检索模块，支持高效的文本搜索'),"
           "(3, 'FTS5功能', 'java 小写'),"
          "(4, '数据库对比', '与 MySQL 相比，SQLite 更适合嵌入式场景，无需独立服务');";

    rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "插入数据失败: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    } else {
        std::cout << "成功插入3条测试数据" << std::endl;
    }

    // 4. 执行全文检索（FTS5核心功能）
    // 搜索包含"SQLite"的记录
    std::string action = "搜索包含'taobao'的记录";
    sql = "SELECT id, title, content FROM " + tableName + " WHERE " + tableName + " MATCH 'taobao';";
    rc = sqlite3_exec(db, sql.c_str(), callback, &action, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "搜索失败: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }

    {
        // 搜索包含"嵌入式"或"全文"的记录（OR逻辑）
        action = "搜索包含'Java'或'Websocket'的记录";
        sql = "SELECT id, title, content FROM " + tableName + " WHERE " + tableName + " MATCH 'Java OR Websocket';";
        rc = sqlite3_exec(db, sql.c_str(), callback, &action, &errMsg);
        if (rc != SQLITE_OK) {
            std::cerr << "搜索失败: " << errMsg << std::endl;
            sqlite3_free(errMsg);
        }
    }

    {
        // 搜索包含"嵌入式"或"全文"的记录（OR逻辑）
        action = "搜索包含'java'的记录 找不到，大小写敏感";
        sql = "SELECT id, title, content FROM " + tableName + " WHERE " + tableName + " MATCH 'java';";
        rc = sqlite3_exec(db, sql.c_str(), callback, &action, &errMsg);
        if (rc != SQLITE_OK) {
            std::cerr << "搜索失败: " << errMsg << std::endl;
            sqlite3_free(errMsg);
        }
    }

    {
        // 搜索包含"嵌入式"或"全文"的记录（OR逻辑）
        action = "搜索包含'嵌入式'或'全文'的记录";
        sql = "SELECT id, title, content FROM " + tableName + " WHERE " + tableName + " MATCH '嵌入式 OR 全文';";
        rc = sqlite3_exec(db, sql.c_str(), callback, &action, &errMsg);
        if (rc != SQLITE_OK) {
            std::cerr << "搜索失败: " << errMsg << std::endl;
            sqlite3_free(errMsg);
        }
    }

    {
        // 搜索包含"嵌入式"或"全文"的记录（OR逻辑）
        action = "搜索所有的记录";
        sql = "SELECT id, title, content FROM " + tableName;
        rc = sqlite3_exec(db, sql.c_str(), callback, &action, &errMsg);
        if (rc != SQLITE_OK) {
            std::cerr << "搜索失败: " << errMsg << std::endl;
            sqlite3_free(errMsg);
        }
    }


    // 5. 关闭数据库
    sqlite3_close(db);
    std::cout << "\n数据库已关闭" << std::endl;

    return 0;
}
