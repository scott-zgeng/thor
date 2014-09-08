// exec_node.h by scott.zgeng@gmail.com 2014.08.08

 
#ifndef  __EXEC_NODE_H__
#define  __EXEC_NODE_H__

extern "C" {
#include "../sql/sqliteInt.h"
#include "../sql/vdbeInt.h"
}


#include "define.h"
#include "expression.h"
#include "column_table.h"


class node_base_t
{
public:
    friend class node_generator_t;
    virtual ~node_base_t() {}
public:
    virtual result_t init(Parse* parse, Select* select) = 0;
    virtual void uninit() = 0;
    virtual result_t next(rowset_t* rs, mem_stack_t* mem) = 0;
  
    virtual rowset_mode_t rowset_mode() const = 0;
    virtual db_uint32 table_count() const = 0;
};


class expr_base_t;
class node_generator_t
{
public:
    node_generator_t(database_t* db, Parse* parse, Select* select);
    ~node_generator_t();

public:
    result_t build(node_base_t** root);

private:
    result_t build_join(node_base_t** scan_nodes, db_int32 tab_num, node_base_t** root);

private:
    Parse* m_parse;
    Select* m_select;
    database_t* m_database;
};






#endif //__EXEC_NODE_H__

