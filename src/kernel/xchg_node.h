// xchg_node.h by scott.zgeng@gmail.com 2014.10.05

#ifndef  __XCHG_NODE_H__
#define  __XCHG_NODE_H__

#include "exec_node.h"







class xchg_node_t : public node_base_t
{
public:
    virtual result_t init(Parse* parse, Select* select) = 0;
    virtual void uninit() = 0;
    virtual result_t next(rowset_t* rs, mem_stack_t* mem) = 0;

    virtual rowset_mode_t rowset_mode() const = 0;
    virtual db_uint32 table_count() const = 0;

};

#endif //__XCHG_NODE_H__

