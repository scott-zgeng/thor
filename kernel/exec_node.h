// exec_node.h by scott.zgeng@gmail.com 2014.08.08

 
#ifndef  __EXEC_NODE__
#define  __EXEC_NODE__

extern "C" {
#include "../sql/sqliteInt.h"
#include "../sql/vdbeInt.h"
}




#include "define.h"


#define MAX_JOIN_TABLE 8




#include "expression.h"



class cursor_t
{
public:
    result_t next_segment(row_set_t* rows);
private:
    db_int32 m_curr_segment_idx;
};

class table_t
{
public:
    static table_t* find_table(const char* tab_name);

    result_t init_cursor(cursor_t* cursor);
};




class node_base_t
{
public:
    virtual ~node_base_t() {}
public:
    virtual result_t init(Parse* parse, Select* select) = 0;
    virtual void uninit() = 0;
    virtual result_t next(row_set_t* rows, mem_stack_t* mem) = 0;
    virtual db_int32 size() = 0;
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


class scan_node_t : public node_base_t
{
public:
    scan_node_t(int index);
    virtual ~scan_node_t();

public:
    virtual result_t init(Parse* parse, Select* select);
    virtual void uninit();
    virtual result_t next(row_set_t* rows, mem_stack_t* mem);
    virtual db_int32 size();

private:
    db_int32 m_index;
    expr_base_t* m_where;  // filter of where 
    cursor_t m_cursor;

    row_set_t m_row_set;
    rowid_t m_row_buf[SEGMENT_SIZE];    
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
    virtual db_int32 size();

private:
    node_base_t* m_left;
    node_base_t* m_right;
};


class project_node_t : public node_base_t
{
public:
    project_node_t(node_base_t* children);
    virtual ~project_node_t();

public:
    virtual result_t init(Parse* parse, Select* select);
    virtual void uninit();
    virtual result_t next(row_set_t* rows, mem_stack_t* mem);
    virtual db_int32 size();

    result_t next();

public:
    node_base_t* m_children;
    db_int32 m_curr_idx;

    // TODO(scott.zgeng@gmail.com): 后续使用vector，现在先简单用
    static const db_int32 MAX_SELECT_RESULT_NUM = 16;
    expr_base_t* m_select_expr[MAX_SELECT_RESULT_NUM];
    db_int32 m_select_num;
    
    mem_handle_t m_expr_mem[MAX_SELECT_RESULT_NUM];
    row_set_t m_sub_rows;

    mem_stack_t m_mem;
};



//class group_node_t : public node_base_t
//{
//
//};
//
//
//class aggr_node_t : public node_base_t
//{
//
//};
//
//class sort_node_t : public node_base_t
//{
//
//};
//



#endif //__EXEC_NODE__

