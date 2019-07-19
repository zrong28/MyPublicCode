#include "SQLiteWrapper.h"

SQLiteWrapper::SQLiteWrapper():db_(0)
{

}

SQLiteWrapper::~SQLiteWrapper(){

}

/**
 * @brief SQLiteWrapper::Open   打开一个sqlite数据库文件
 * @param db_file
 * @return
 */
bool SQLiteWrapper::Open(const std::string &db_file){

    if(sqlite3_open(db_file.c_str(),&db_)!=SQLITE_OK)
    {
        return false;
    }

    return true;
}

/**
 * @brief SQLiteWrapper::Close 关闭数据库文件
 * @return
 */
bool SQLiteWrapper::Close(){

    if(sqlite3_close(db_)!=SQLITE_OK){
#if DEBUG_MSG_PRINT
        perror("close sqlite error:");
#endif
        return false;
    }
    db_=NULL;

    return true;
}

/**
 * @brief SQLiteWrapper::SelectStmt 执行select语句，
 * @param stmt sql语句
 * @param res 使用sqlite3_exec()时传给回调函数的参数
 * @return
 */
bool SQLiteWrapper::SelectStmt(const std::string &stmt, ResultTable &res){

    char *errmsg;
    int ret;

    res.reset();

    ret=sqlite3_exec(db_,stmt.c_str(),SelectCallback,
                     static_cast<void*>(&res),&errmsg);

    if(ret!=SQLITE_OK)
        return false;
    return true;

}

/**
 * @brief SQLiteWrapper::SelectCallback select操作的回调函数
 * @param p_data
 * @param num_fields
 * @param p_fields
 * @param p_col_names
 * @return
 */
int SQLiteWrapper::SelectCallback(void *p_data, int num_fields, char **p_fields, char **p_col_names)
{
    ResultTable* res=reinterpret_cast<ResultTable*>(p_data);

    ResultRecord record;

#ifdef SQLITE_WRAPPER_REPORT_COLUMN_NAMES
    if(res->records_.size()=0)
    {
        ResultRecord col_names;
        for(int i=0;i<num_fields;i++)
        {
            if(p_fields[i])
                record.fields_.push_back("<null>");
        }
        res->records_.push_back(record);
    }
#endif

    for(int i=0;i<num_fields;i++)
    {
        if(p_fields[i])
            record.fields_.push_back(p_fields[i]);
        else
            record.fields_.push_back("<null>");
    }

    res->records_.push_back(record);

    return 0;
}

/**
 * @brief SQLiteWrapper::Statement
 * @param statement
 * @return /这个返回的SQLiteStatement指针记住要在用完之后释放。否则不仅内存泄漏，并且关闭数据库操作句柄的时候会出错。
 */
SQLiteStatement* SQLiteWrapper::Statement(const std::string &statement)
{
    SQLiteStatement* stmt;
    try{
        stmt=new SQLiteStatement(statement,db_);
        return stmt;
    }
    catch(const char* e){
        return 0;
    }
}

/**
 * @brief SQLiteStatement::SQLiteStatement 一个可存放查询结果表的类，可以通过相关方法提取查询执行的结果。
 * @param statement SQL语句
 * @param db 数据库套接字
 */
SQLiteStatement::SQLiteStatement(const std::string &statement, sqlite3 *db)
{
    if(sqlite3_prepare(db,statement.c_str(),-1,&stmt_,0)!=SQLITE_OK){
        throw sqlite3_errmsg(db);
    }

    if(!stmt_){
        throw "stmt_ is 0";
    }

}

SQLiteStatement::~SQLiteStatement(){
    if(stmt_){
        sqlite3_finalize(stmt_);
    }

//    std::cout<<"delete sqlite3 stmt"<<std::endl;
}

SQLiteStatement::SQLiteStatement():stmt_(0){}


bool SQLiteStatement::Bind(int pos_zero_indexed, const std::string &value){
    if(sqlite3_bind_text(
                stmt_,pos_zero_indexed+1,
                value.c_str(),
                value.length(),
                SQLITE_TRANSIENT
                )
            !=SQLITE_OK){
        return false;
    }
    return true;
}

bool SQLiteStatement::Bind(int pos_zero_indexed, double value){
    if(sqlite3_bind_double(stmt_,
                           pos_zero_indexed+1,
                           value
                           )
            !=SQLITE_OK){
        return false;
    }
    return true;
}

bool SQLiteStatement::BindNull(int pos_zero_indexed){
    if(sqlite3_bind_null(
                stmt_,
                pos_zero_indexed+1
                )
            !=SQLITE_OK){
        return false;
    }
    return true;
}

/**
 * @brief SQLiteStatement::Execute
 * @return
 */
bool SQLiteStatement::Execute(){
    int rc=sqlite3_step(stmt_);
    if(rc==SQLITE_BUSY){
#if DEBUG_MSG_PRINT
        std::cout<<"SQLITE BUSY"<<std::endl;
#endif
        return false;
    }

    if(rc==SQLITE_ERROR){
#if DEBUG_MSG_PRINT
        std::cout<<"SQLITE ERROR"<<std::endl;
#endif
        return false;
    }
    if(rc==SQLITE_MISUSE){
#if DEBUG_MSG_PRINT
        std::cout<<"SQLITE ERROR"<<std::endl;
#endif
        return false;
    }

    if(rc!=SQLITE_DONE){
        return false;
    }

    sqlite3_reset(stmt_);
    return true;
}

SQLiteStatement::dataType SQLiteStatement::DataType(int pos_zero_indexed){
    return dataType(sqlite3_column_type(stmt_,pos_zero_indexed));
}

/**
 * @brief SQLiteStatement::ValueInt 返回某一个字段的int型的值
 * @param pos_zero_indexed 目标字段在数据库表中的顺序号，从左到右，从0开始。
 * @return
 */
int SQLiteStatement::ValueInt(int pos_zero_indexed){
    return sqlite3_column_int(stmt_,pos_zero_indexed);
}

/**
 * @brief SQLiteStatement::ValueString 返回某一个字段的string型的值
 * @param pos_zero_indexed 目标字段在数据库表中的顺序号，从左到右，从0开始。
 * @return
 */
std::string SQLiteStatement::ValueString(int pos_zero_indexed){
    return std::string(reinterpret_cast<const char *>(sqlite3_column_text(stmt_,pos_zero_indexed)));
}

/**
 * @brief SQLiteStatement::RestartSelect
 * @return
 */
bool SQLiteStatement::RestartSelect(){
    sqlite3_reset(stmt_);
    return true;
}

bool SQLiteStatement::Reset(){
    int rc=sqlite3_step(stmt_);

    sqlite3_reset(stmt_);

    if(rc==SQLITE_ROW)
        return true;
    return false;
}

/**
 * @brief SQLiteStatement::NextRow
 * @return
 */
bool SQLiteStatement::NextRow(){

    int rc=sqlite3_step(stmt_);

    if(rc==SQLITE_ROW){
        return true;
    }

    if(rc==SQLITE_DONE){
        sqlite3_reset(this->stmt_);
        return false;
    }
    else if(rc==SQLITE_MISUSE){
#if DEBUG_MSG_PRINT
        std::cout<<"SQLiteStatement::NextRow SQLITE_MISUSE"<<std::endl;
#endif
    }
    else if(rc==SQLITE_BUSY){
#if DEBUG_MSG_PRINT
        std::cout<<"SQLiteStatement::NextRow SQLITE_BUSY"<<std::endl;
#endif
    }
    else if(rc==SQLITE_ERROR){
#if DEBUG_MSG_PRINT
        std::cout<<"SQLiteStatement::NextRow SQLITE_ERROR"<<std::endl;
#endif
    }
    return false;
}

/**
 * @brief SQLiteWrapper::DirectStatement    sql语句执行
 * @param stmt sql语句
 * @return 返回执行结果
 */
bool SQLiteWrapper::DirectStatement(const std::string &stmt){
    char * errmsg;

    int ret;

    ret=sqlite3_exec(this->db_,stmt.c_str(),0,0,&errmsg);

    if(ret!=SQLITE_OK){

#if DEBUG_MSG_PRINT
        printf("%s \n",errmsg);
#endif

        sqlite3_free(errmsg);

        return false;
    }

    return true;
}

std::string SQLiteWrapper::LastError(){
    return sqlite3_errmsg(this->db_);
}

bool SQLiteWrapper::Begin(){
    return this->DirectStatement("begin");
}

bool SQLiteWrapper::Commit(){
    return this->DirectStatement("commit");
}

bool SQLiteWrapper::Rollback(){
    return this->DirectStatement("rollback");
}
