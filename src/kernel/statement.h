// statement.h by scott.zgeng@gmail.com 2014.08.21


#ifndef  __STATEMENT_H__
#define  __STATEMENT_H__



#include "define.h"
#include "pod_vector.h"
#include "expression.h"

class database_t;
class statement_t
{
public:    
    virtual ~statement_t() {}    
    virtual db_int32 next() = 0;

    database_t* get_database() { return m_database; }
protected:
    database_t* m_database;
};


struct Parse;
struct SrcList;
struct Select;
struct IdList;


class project_node_t;
class select_stmt_t : public statement_t
{
public:
    select_stmt_t(database_t* db);
    virtual ~select_stmt_t();
public:
    db_int32 next();

public:
    result_t prepare(Parse *pParse, Select *pSelect);
    project_node_t* root() { return m_root; }
private:
    project_node_t* m_root;  

};


class column_table_t;
class expr_base_t;
class insert_stmt_t : public statement_t
{
public:
    insert_stmt_t(database_t* db);
    virtual ~insert_stmt_t();
public:    
    virtual db_int32 next();

public:
    result_t prepare(Parse *pParse, SrcList *pTabList, Select *pSelect, IdList *pColumn, int onError);

private:
    column_table_t* m_table;
    pod_vector<expr_base_t*, 64> m_insert_values;
    mem_stack_t m_mem;
};


#endif //__STATEMENT_H__


