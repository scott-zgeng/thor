// scan_node.h by scott.zgeng@gmail.com 2014.08.27


#ifndef  __SCAN_NODE_H__
#define  __SCAN_NODE_H__



#include "exec_node.h"


class scan_node_t : public node_base_t
{
public:
    scan_node_t(int index);
    virtual ~scan_node_t();

public:
    virtual result_t init(Parse* parse, Select* select);
    virtual void uninit();
    virtual result_t next(rowset_t* rowset, mem_stack_t* mem);
    virtual db_int32 rowid_size();

private:
    db_int32 m_index;
    expr_base_t* m_where;  // where condition
    cursor_t m_cursor;
    rowid_t m_rows[SEGMENT_SIZE];
    rowset_t m_rowset;
    db_bool m_eof;
};



#endif //__SCAN_NODE_H__

