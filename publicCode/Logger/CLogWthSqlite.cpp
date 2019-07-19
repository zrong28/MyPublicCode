#include "CLogWthSqlite.h"


/**
 * @brief getCurrentTime 获取当前系统时间
 * @return 返回日期字符串，格式：Thu May 30 14:11:31 2019

 */
std::string get_CurrentTime(){
    time_t timep;
    time(&timep);
    std::string time=ctime(&timep);
    return time;
}

/**
 * @brief getDateOfType 自定义当前时间输出，格式定义参照strftime()的时间格式定义方法
 * @param datetype
 * @return
 */
std::string getDateOfType(const char *datetype){

    struct tm tm;

    char buf[255];

    strptime(get_CurrentTime().c_str(), "%a %b %d %H:%M:%S %Y", &tm);

//    printf("asctime:%s\n",asctime(&tm));

    memset(buf,0,sizeof(buf));

    strftime(buf, sizeof(buf), datetype, &tm);

    return buf;

}

/**
 * @brief day_diff      日期间隔计算
 * @param year_start    开始年份
 * @param month_start   开始月份
 * @param day_start     开始日
 * @param year_end      结束年份
 * @param month_end     结束月份
 * @param day_end       结束日
 * @return              两个日期的间隔
 */
int day_diff(int year_start, int month_start, int day_start
             , int year_end, int month_end, int day_end)
{
    int y2, m2, d2;
    int y1, m1, d1;

    m1 = (month_start + 9) % 12;
    y1 = year_start - m1/10;
    d1 = 365*y1 + y1/4 - y1/100 + y1/400 + (m1*306 + 5)/10 + (day_start - 1);

    m2 = (month_end + 9) % 12;
    y2 = year_end - m2/10;
    d2 = 365*y2 + y2/4 - y2/100 + y2/400 + (m2*306 + 5)/10 + (day_end - 1);

    return (d2 - d1);
}

CLogWthSqlite::CLogWthSqlite(){
    this->m_sqlwp=NULL;
}

CLogWthSqlite::~CLogWthSqlite(){

}

CLogWthSqlite* CLogWthSqlite::getInstance(){
    static CLogWthSqlite cws;
    return &cws;
}

/**
 * @brief CLogWthSqlite::set_DB_Param   设置数据库的相关参数
 * @param db_   数据库操作对象的指针
 * @param table_name    数据库中的日志表名称
 * @param field_time    数据库中的日志表中的日志时间的字段名
 * @param field_text    数据库中的日志表中的日志文本的字段名
 * @param log_max_len   单条日志字符传最大长度
 */
void CLogWthSqlite::init(SQLiteWrapper* db_,
                         const char *table_name,
                         const char *field_time,
                         const char *field_text,
                         const char *field_level,
                         size_t log_max_len){

    this->m_log_buf_size=log_max_len;
    m_buf=new char[m_log_buf_size];
    memset(m_buf,0,this->m_log_buf_size);

    if(db_!=NULL){
        this->m_sqlwp=db_;
    }

    if(table_name!=NULL){
        this->m_log_table=table_name;
    }

    if(field_time!=NULL){
        this->m_log_field_time=field_time;
    }

    if(field_text!=NULL){
        this->m_log_field_text=field_text;
    }

    this->m_log_field_level=field_level;
}

/**
 * @brief CLogWthSqlite::writeLogIntoSqlite
 * @param log_time
 * @param log_text
 * @param log_level
 * @return
 */
bool CLogWthSqlite::writeLogIntoSqlite(const char* log_time,const char *log_level,const char *log_text){

    if(this->m_sqlwp==NULL){
        return false;
    }

    string sql="insert into "+m_log_table+" values ('"+
            log_time+"','"+log_level+"','"+log_text+"')";

    return this->m_sqlwp->DirectStatement(sql);
}

/**
 * @brief CLogWthSqlite::deleteLogs 日志删除
 * @param save_days 定义保留多少天的日志，-1为全删
 * @return 删除成功返回true，失败返回false
 */
bool CLogWthSqlite::deleteLogs(int save_days){

    if(this->m_sqlwp==NULL){
        return false;
    }

    string current_date=getDateOfType(DATE_TYPE);
    int c_day,c_month,c_year;
    sscanf(current_date.c_str(),"%d-%d-%d %*s",&c_year,&c_month,&c_day);

    string select="select "+this->m_log_field_time+" from "+this->m_log_table;
    SQLiteStatement* stmt=this->m_sqlwp->Statement(select);

    if(stmt!=NULL){
        while(stmt->NextRow()){

            string logs_date=stmt->ValueString(0);

            int l_day,l_month,l_year;
            sscanf(logs_date.c_str(),"%d-%d-%d %*s",&l_year,&l_month,&l_day);
            int d_diff=day_diff(l_year,l_month,l_day,c_year,c_month,c_day);

            if(d_diff>save_days){

                string delete_sql="delete from "+m_log_table+" where "+m_log_field_time+" like '"+logs_date+"'";
                if(m_sqlwp->DirectStatement(delete_sql))
                {
                    std::cout<<"Deleted a log succesed"<<std::endl;
                }else{
                    std::cout<<"Deleted a log failed"<<std::endl;
                }
            }
        }
        delete stmt;
    }

    return true;
}

/**
 * @brief CLogWthSqlite::write_log 写日志
 * @param level 日志等级
 * @param format 日志内容
 */
void CLogWthSqlite::write_log(LOGS_LEVEL level, const char *format,...){

    struct timeval now={0,0};

    gettimeofday(&now,NULL);

    time_t t = now.tv_sec;

    struct tm* sys_tm=localtime(&t);

    struct tm my_tm=*sys_tm;

    char s[16]={0};

    //确认日志等级
    switch(level){
    case LOGS_LEVEL::DEBUG:
        strcpy(s,"DEBUG");
        break;

    case LOGS_LEVEL::INFO:
        strcpy(s,"INFO");
        break;

    case LOGS_LEVEL::WARNNING:
        strcpy(s,"WARNNING");
        break;

    case LOGS_LEVEL::ERROR:
        strcpy(s,"ERROR");
        break;

    default:
        strcpy(s,"INFO");
        break;
    }

    char l_date[255]={0};

    //获取日志时间
    int n=snprintf(l_date,255,"%d-%02d-%02d %02d:%02d:%02d.%06d",
                   my_tm.tm_year+1900,my_tm.tm_mon+1,my_tm.tm_mday,
                   my_tm.tm_hour,my_tm.tm_min,my_tm.tm_sec,now.tv_usec);

    pthread_mutex_lock(m_mutex);
    m_count++;

    pthread_mutex_unlock(this->m_mutex);

    va_list valst;

    va_start(valst,format);

    pthread_mutex_lock(m_mutex);

    //整合日志具体内容
    int m=vsnprintf(m_buf,m_log_buf_size-1,format,valst);

//    m_buf[m]='\n';

    pthread_mutex_unlock(this->m_mutex);

    va_end(valst);

    //写入数据库
    writeLogIntoSqlite(l_date,s,m_buf);
}
