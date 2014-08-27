// join_node.h by scott.zgeng@gmail.com 2014.08.27


#ifndef  __JOIN_NODE_H__
#define  __JOIN_NODE_H__


#include "exec_node.h"


class join_node_t : public node_base_t
{
public:
    join_node_t(node_base_t* left, node_base_t* right);
    virtual ~join_node_t();

public:
    virtual result_t init(Parse* parse, Select* select);
    virtual void uninit();
    virtual result_t next(rowset_t* rows, mem_stack_t* mem);
    virtual db_int32 rowid_size();

private:
    node_base_t* m_left;
    node_base_t* m_right;
};


#endif //__JOIN_NODE_H__
