// exec_node.h by scott.zgeng@gmail.com 2014.08.08

 
#ifndef  __EXEC_NODE_H__
#define  __EXEC_NODE_H__

extern "C" {
#include "../sql/sqliteInt.h"
#include "../sql/vdbeInt.h"
}


#include "define.h"
#include "expression.h"
#include "pod_vector.h"
#include "column_table.h"


class node_base_t
{
public:
    virtual ~node_base_t() {}
public:
    virtual result_t init(Parse* parse, Select* select) = 0;
    virtual void uninit() = 0;
    virtual result_t next(row_set_t* rows, mem_stack_t* mem) = 0;
    virtual db_int32 rowid_size() = 0;
};


class expr_base_t;
class node_generator_t
{
public:
    node_generator_t(Parse* parse, Select* select);
    ~node_generator_t();

public:
    result_t build(node_base_t** root);

private:
    result_t build_join(node_base_t** scan_nodes, db_int32 tab_num, node_base_t** root);

private:
    Parse* m_parse;
    Select* m_select;
};




class project_node_t : public node_base_t
{
public:
    static const db_int32 DEFAULT_EXPR_NUM = 16;
    project_node_t(node_base_t* children);
    virtual ~project_node_t();

public:
    virtual result_t init(Parse* parse, Select* select);
    virtual void uninit();
    virtual result_t next(row_set_t* rows, mem_stack_t* mem);
    virtual db_int32 rowid_size();

    result_t next();
    db_int32 count() const { 
        return m_sub_rows.count();
    }

    void* column_data(db_int32 idx) {
        return m_expr_values[idx];
    }

    data_type_t column_type(db_int32 idx) {
        return m_expr_columns[idx]->type();
    }

private:
    node_base_t* m_children;
    pod_vector<expr_base_t*, DEFAULT_EXPR_NUM> m_expr_columns;
    mem_stack_t m_mem;
    row_set_t m_sub_rows;
    pod_vector<void*, DEFAULT_EXPR_NUM> m_expr_values;

public:
    // 下面的变量需要提出去
    db_int32 m_curr_idx;
};


class scan_node_t : public node_base_t
{
public:
    scan_node_t(int index);
    virtual ~scan_node_t();

public:
    virtual result_t init(Parse* parse, Select* select);
    virtual void uninit();
    virtual result_t next(row_set_t* rows, mem_stack_t* mem);
    virtual db_int32 rowid_size();

private:
    db_int32 m_index;
    expr_base_t* m_where;  // filter of where 
    cursor_t m_cursor;
};





class join_node_t : public node_base_t
{
public:
    join_node_t(node_base_t* left, node_base_t* right);
    virtual ~join_node_t();

public:
    virtual result_t init(Parse* parse, Select* select);
    virtual void uninit();
    virtual result_t next(row_set_t* rows, mem_stack_t* mem);
    virtual db_int32 rowid_size();

private:
    node_base_t* m_left;
    node_base_t* m_right;
};




#endif //__EXEC_NODE_H__

