// join_node.cpp by scott.zgeng@gmail.com 2014.08.27




#include "join_node.h"




//-----------------------------------------------------------------------------
// join_node_t
//-----------------------------------------------------------------------------
join_node_t::join_node_t(node_base_t* left, node_base_t* right)
{
    m_left = left;
    m_right = right;
}

join_node_t::~join_node_t()
{

}


result_t join_node_t::init(Parse* parse, Select* select)
{
    return RT_FAILED;
}

void join_node_t::uninit()
{

}

db_int32 join_node_t::rowid_size()
{
    return m_left->rowid_size() + m_right->rowid_size();
}


result_t join_node_t::next(rowset_t* rows, mem_stack_t* mem)
{
    return RT_FAILED;
}


