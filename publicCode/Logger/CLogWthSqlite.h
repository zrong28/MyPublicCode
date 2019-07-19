/*
 *使用流程：
 * 1、初始化数据库名称；
 * 2、初始化在sqlite数据库中存出日志的表名、字段名；
 * 3、调用相关函数进行日志写入/删除。
 * 4、如果设置好了数据库相关参数，当进行日志生成函数调用的时候，会把生成的日志写入到相关数据库表格中
 */

#ifndef CLOGWTHSQLITE_H
#define CLOGWTHSQLITE_H

#include "clogger.h"
#include "SQLiteWrapper.h"


#define DATABASE "logs_test.db"
#define LOGS_TABLE "logs"
#define DATE_TYPE "%Y-%m-%d %H:%M:%S"

/**
 * @brief day_diff  日期间隔计算
 */
int day_diff(int year_start, int month_start, int day_start
             , int year_end, int month_end, int day_end);
std::string getCurrentTime();

class CLogWthSqlite:public CLogger
{

public:

    static CLogWthSqlite* getInstance();
//    bool init(const char *file_name, int log_buf_size, int split_lines);    //初始化

    //初始化
    void init(SQLiteWrapper *db_,const char *table_name,const char *field_time,
              const char *field_text,const char *field_level,size_t log_max_len);  //设置日志存储的数据库名、表名字和字段名字。

    //日志写
    void write_log(LOGS_LEVEL level, const char *format,...);

    //日志删除
    bool deleteLogs(int save_days);//删除数据库中的日志条目，同时可选择保留固定天数的日志

private:

    //日志写入数据库
    bool writeLogIntoSqlite(const char* log_time,const char *log_level,const char *log_text);  //日志写入数据库

    CLogWthSqlite();

    ~CLogWthSqlite();

    string m_log_db;

    string m_log_table;

    string m_log_field_time;

    string m_log_field_level;

    string m_log_field_text;

    SQLiteWrapper* m_sqlwp;

};

#endif // CLOGWTHSQLITE_H
