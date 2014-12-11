// join_node.h by scott.zgeng@gmail.com 2014.08.27


#ifndef  __JOIN_NODE_H__
#define  __JOIN_NODE_H__


#include "exec_node.h"


class hash_join_node_t : public node_base_t
{
public:
    hash_join_node_t(statement_t* stmt, node_base_t* left, node_base_t* right, const db_char* left_table, const db_char* right_table);
    virtual ~hash_join_node_t();

public:
    virtual result_t init(Parse* parse, Select* select);
    virtual void uninit();
    virtual result_t next(rowset_t* rs, mem_stack_t* mem);

    virtual rowset_mode_t rowset_mode() const {
        return MULTI_TABLE_MODE;
    }

    virtual db_uint32 table_count() const {
        return m_left->table_count() + m_right->table_count();
    }

protected:
    result_t build_hash_table(mem_stack_t* mem);

private:
    statement_t* m_stmt;
    node_base_t* m_left;
    node_base_t* m_right;

    db_char m_left_name[MAX_TAB_NAME_LEN + 1];
    db_char m_right_name[MAX_TAB_NAME_LEN + 1];

    expr_base_t* m_left_expr;
    expr_base_t* m_right_expr;

    single_rowset_t m_rowset;
};


#endif //__JOIN_NODE_H__

