#ifndef SQLITEWRAPPER_H
#define SQLITEWRAPPER_H

#include <string>
#include <vector>
#include <iostream>

#include <sqlite3.h>

/**
 * @brief The SQLiteStatement class
 */
class SQLiteStatement{
private:
    friend class SQLiteWrapper;
    SQLiteStatement(std::string const& statement,sqlite3* db);

public:
    SQLiteStatement();

    enum dataType{
        INT=SQLITE_INTEGER,
        FLT=SQLITE_FLOAT,
        TXT=SQLITE_TEXT,
        BLB=SQLITE_BLOB,
        NUL=SQLITE_NULL
    };

    dataType DataType(int pos_zero_indexed);

    int ValueInt(int pos_zero_indexed);
    std::string ValueString(int pos_zero_indexed);

    ~SQLiteStatement();

    bool Bind(int pos_zero_indexed,std::string const& value);
    bool Bind(int pos_zero_indexed,double value);
    bool Bind(int pos_zero_indexed,int value);
    bool BindNull(int pos_zero_indexed);

    bool Execute();

    bool NextRow();

    bool Reset();

    bool RestartSelect();

private:
    sqlite3_stmt* stmt_;

};

/**
 * @brief The SQLiteWrapper class   可以用于创建一个sqlite3的连接句柄实例，
 * 该类封装了一些sql语句执行、查询结果返回等等方法
 */
class SQLiteWrapper
{
public:
    SQLiteWrapper();
    ~SQLiteWrapper();

    bool Open(const std::string& db_file);
    bool Close();


    class ResultRecord{
    public:
        std::vector<std::string> fields_;

    };

    class ResultTable{
        friend class SQLiteWrapper;
    public:
        ResultTable():ptr_cur_record_(0){}
        std::vector<ResultRecord> records_;

        ResultRecord* next(){
            if(ptr_cur_record_<records_.size())
                return &(records_[ptr_cur_record_++]);

            return 0;
        }
    private:
        void reset(){
            records_.clear();
            ptr_cur_record_=0;
        }

    private:
        unsigned int ptr_cur_record_;
    };

    bool SelectStmt(std::string const& stmt,ResultTable& res);
    bool DirectStatement(const std::string& statement);
    SQLiteStatement* Statement(const std::string& stmt);
    std::string LastError();

    bool Begin();
    bool Commit();
    bool Rollback();

private:
    static int SelectCallback(void *p_data,int num_fields,char **p_fields,char **p_col_names);
    sqlite3* db_;

};

/**
 * 1、该类是一个可以存放SQLiteStatement指针的类,。
 *
 * 2、他的主要作用是管理当使用SQLiteWrapper::Statement()时返回的SQLiteStatement指针，
 * 并在调用了SQLiteWrapper::Statement()的函数结束时自动释放SQLiteStatement指针指向的
 * 内存，防止忘记手动delete后导致的内存泄露、以及接下来数据库套接字无法使用等等问题。
 *
 * For example:
 *
   void func(SQLiteWrapper& sql_wp)
   {
    SQLiteStatementManager ssm(sql_wp.Statement("select * from a_table"));

//    SQLiteStatementManager ssm=sql_wp.Statement("select * from a_table");
//    ssm->NextRow();

    if(ssm.GetStmt()==NULL){
        return;//函数结束，无需调用delete释放SQLiteWrapper::Statement()返回值指向的内存
    }
    else
    {
        while(ssm.GetStmt().NextRow())
        {

        }
    }

    //函数结束，无需调用delete释放SQLiteWrapper::Statement()返回值指向的内存
   }

 *
 *
*/
class SQLiteStatementManager{

public:
    SQLiteStatementManager(SQLiteStatement* stmt=NULL):m_stmt(stmt){}

    ~SQLiteStatementManager(){delete m_stmt;}

    //返回成员 SQLiteStatment* SQLiteStatementManager::m_stmt
    SQLiteStatement* GetStmt() const {return m_stmt;}

    SQLiteStatementManager& operator=(SQLiteStatement* stmt){m_stmt=stmt;return *this;}

    //重载->,好像没有很方便。。。
    SQLiteStatement* operator->() const {return m_stmt;}

    bool operator==(SQLiteStatement* pStmt){ return pStmt==m_stmt;}

    bool operator!=(SQLiteStatement* pStmt){ return pStmt!=m_stmt;}

private:
    SQLiteStatement* m_stmt;

};

#endif // SQLITEWRAPPER_H
