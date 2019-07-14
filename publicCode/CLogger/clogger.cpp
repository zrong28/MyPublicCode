#include "clogger.h"

using namespace std;


CLogger::CLogger(){
    m_count=0;
    m_mutex=new pthread_mutex_t;
    pthread_mutex_init(m_mutex,NULL);
}

CLogger::~CLogger(){

    if(m_fp!=NULL){
        fclose(m_fp);
        m_fp==NULL;
    }

    pthread_mutex_destroy(this->m_mutex);

    if(this->m_mutex!=NULL){
        delete this->m_mutex;
    }

    if(this->m_buf!=NULL){
        delete[] this->m_buf;
    }
}

/**
 * @brief CLogger::init 初始化
 * @param file_name 文件名
 * @param log_buf_size 一条日志的长度限制
 * @param split_lines  一个日志文本的行数限制
 * @return
 */
bool CLogger::init(const char *file_name, int log_buf_size, int split_lines){

    m_log_buf_size=log_buf_size;
    m_buf=new char[m_log_buf_size];

    memset(m_buf,'\0',m_log_buf_size);

    m_split_lines=split_lines;

    time_t t=time(NULL);
    struct tm* sys_tm=localtime(&t);

    struct tm my_tmm=*sys_tm;
    const char *p =strrchr(file_name,'/');
    char log_full_name[256]={0};

    if(p==NULL){    //日志名字追加日期

        snprintf(log_full_name,255,"%d_%02d_%02d_%s",
                 my_tmm.tm_year+1900,my_tmm.tm_mon+1,my_tmm.tm_mday,
                 file_name);

    }else{

        strcpy(log_name,p+1);

        strncpy(dir_name,file_name,p-file_name+1);

        snprintf(log_full_name,255,"%s%d_%02d_%02d_%s",
                 dir_name,
                 my_tmm.tm_year+1900,my_tmm.tm_mon+1,my_tmm.tm_mday,
                 log_name);
    }

    m_today=my_tmm.tm_mday;

    printf("%s\n",log_full_name);

    m_fp=fopen(log_full_name,"a");

    if(m_fp==NULL){
//        perror("fopen error:");
        return false;
    }

    return true;
}

/**
 * @brief CLogger::write_log 日志写入
 * @param level 日志等级
 * @param format 日志格式 后面...表示附带具体参数（类似于printf的定义方式）
 *
 */
void CLogger::write_log(LOGS_LEVEL level, const char *format,...){

    struct timeval now={0,0};

    gettimeofday(&now,NULL);

    time_t t = now.tv_sec;

    struct tm* sys_tm=localtime(&t);

    struct tm my_tm=*sys_tm;

    char s[16]={0};

    switch(level){
    case LOGS_LEVEL::DEBUG:
        strcpy(s,"[debug]:");
        break;

    case LOGS_LEVEL::INFO:
        strcpy(s,"[info]:");
        break;

    case LOGS_LEVEL::WARNNING:
        strcpy(s,"[warnning]:");
        break;

    case LOGS_LEVEL::ERROR:
        strcpy(s,"[error]:");
        break;

    default:
        strcpy(s,"[info]:");
        break;
    }

    int n=snprintf(m_buf,255,"%d-%02d-%02d %02d:%02d:%02d.%06d %s",
                   my_tm.tm_year+1900,my_tm.tm_mon+1,my_tm.tm_mday,
                   my_tm.tm_hour,my_tm.tm_min,my_tm.tm_sec,now.tv_usec,s);

    pthread_mutex_lock(m_mutex);
    m_count++;

    if(this->m_today!=my_tm.tm_mday||m_count%m_split_lines==0){ //过去一天或者日志行数达到所设限制，就创建新的日志文件

        char new_log[255]={0};
        fflush(m_fp);
        fclose(m_fp);
        char tail[16]={0};

        snprintf(tail,16,"%d_%02d_%02d",my_tm.tm_year+1900,my_tm.tm_mon+1,my_tm.tm_mday);

        if(m_today!=my_tm.tm_mday){

            snprintf(new_log,255,"%s%s%s",dir_name,tail,log_name);
            m_today=my_tm.tm_mday;
            m_count=0;

        }else{
            snprintf(new_log,255,"%s%s%s.%d",dir_name,tail,log_name,m_count/m_split_lines);
        }

        m_fp=fopen(new_log,"a");
    }

    pthread_mutex_unlock(this->m_mutex);

    va_list valst;

    va_start(valst,format);

    pthread_mutex_lock(m_mutex);

    int m=vsnprintf(m_buf+n,m_log_buf_size-1,format,valst);

    m_buf[n+m-1]='\n';
    fputs(m_buf,m_fp);

    pthread_mutex_unlock(this->m_mutex);

    va_end(valst);
}

void CLogger::flush(void){
    pthread_mutex_lock(this->m_mutex);
    fflush(m_fp);
    pthread_mutex_unlock(this->m_mutex);
}
