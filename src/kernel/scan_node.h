// scan_node.h by scott.zgeng@gmail.com 2014.08.27


#ifndef  __SCAN_NODE_H__
#define  __SCAN_NODE_H__



#include "exec_node.h"


class scan_node_t : public node_base_t
{
public:
    scan_node_t(statement_t* stmt, int index);
    virtual ~scan_node_t();

public:
    virtual result_t init(Parse* parse, Select* select);
    virtual void uninit();
    virtual result_t next(rowset_t* rs, mem_stack_t* mem);

    virtual rowset_mode_t rowset_mode() const {
        return SINGLE_TABLE_MODE;
    }

    virtual db_uint32 table_count() const {
        return 1;
    }

private:
    db_int32 m_index;
    expr_base_t* m_where;  // where condition
    cursor_t m_cursor;    
    single_rowset_t m_rowset;
    db_bool m_eof;

    rowid_t* m_odd_rows;
    db_uint32 m_odd_count;

    statement_t* m_stmt;
};



#endif //__SCAN_NODE_H__

