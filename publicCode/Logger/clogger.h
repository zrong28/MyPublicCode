#ifndef CLOGGER_H
#define CLOGGER_H

#include <stdio.h>
#include <iostream>
#include <string>
#include <cstring>
#include <pthread.h>
#include <sys/time.h>
#include <time.h>
#include <stdarg.h>

using std::cout;
using std::endl;
using std::string;

//日志操作类
class CLogger{
public:
    typedef struct LogsNode{
        string s_time;
        string s_level;
        string s_log_text;
    }LogsNode;


    enum LOGS_LEVEL{
        DEBUG=0,
        INFO,
        WARNNING,
        ERROR
    };

//    enum LOGS_SAVE{
//        TEXT_FILE_SAVE=0,
//        DB_FILE_SAVE,
//        BOTH_SAVE
//    };

public:

    static CLogger* getInstance(){
        static CLogger instance;
        return &instance;
    }

    virtual bool init(const char* file_name,int log_buf_size=8192,int split_lines=5000000); //初始化

    bool setTextFile(const char * _file_name);  //设置存储日志的文本文件
//    bool setSqliteDB(const char * _db_route);   //设置存储日志的sqlite数据库

//    bool setLogsSave(LOGS_SAVE _SAVE_TYPE,bool isSave);    //日志存储位置

    virtual void write_log(LOGS_LEVEL level,const char * format,...);   //日志写入

    void flush(void);

protected:
    CLogger();
    virtual ~CLogger();

//private:

    pthread_mutex_t *m_mutex;

    char dir_name[1024];
    char log_name[1024];
    int m_split_lines;
    int m_log_buf_size;
    long long m_count;
    int m_today;
    FILE *m_fp;
    char *m_buf;

}; // CLogger

#endif // CLOGGER_H
