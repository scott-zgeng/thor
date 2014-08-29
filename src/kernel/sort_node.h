// sort_node.h by scott.zgeng@gmail.com 2014.08.29


#ifndef  __SORT_NODE_H__
#define  __SORT_NODE_H__



#include "exec_node.h"



class sort_node_t : public node_base_t
{
public:
    sort_node_t(node_base_t* children);
    virtual ~sort_node_t();

public:
    virtual result_t init(Parse* parse, Select* select);
    virtual void uninit();
    virtual result_t next(rowset_t* rows, mem_stack_t* mem);
    virtual db_int32 rowid_size();

private:   
    node_base_t* m_children;    
};



#endif //__SORT_NODE_H__


