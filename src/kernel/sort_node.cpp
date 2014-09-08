// sort_node.cpp by scott.zgeng@gmail.com 2014.08.29


#include "sort_node.h"


sort_node_t::sort_node_t(node_base_t* children)
{
    m_children = children;
}

sort_node_t::~sort_node_t()
{

}


result_t sort_node_t::init(Parse* parse, Select* select)
{
    return RT_FAILED;
}

void sort_node_t::uninit()
{

}

result_t sort_node_t::next(rowset_t* rows, mem_stack_t* mem)
{
    return RT_FAILED;
}


