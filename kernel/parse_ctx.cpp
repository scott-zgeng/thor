// parse_ctx.c by scott.zgeng@gmail.com 2014.07.11

#include <assert.h>

#include "parse_ctx.h"


node_generator::node_generator(Parse* parse, Select* select)
{
    m_parse = parse;
    m_select = select;
}

node_generator::~node_generator()
{

}



result_t node_generator::build_join(node_base_t** scan_nodes, int32 tab_num, node_base_t** root)
{
    if (tab_num == 1) return RT_SUCCEEDED;

    result_t ret;
    node_base_t* node = scan_nodes[0];
    for (int32 i = 1; i < tab_num; i++) {
        node_base_t* new_node = new join_node_t(node, scan_nodes[i]);
        IF_RETURN_FAILED(new_node == NULL);

        ret = new_node->init(m_parse, m_select);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);

        node = new_node;
    }

    return RT_SUCCEEDED;
}

result_t node_generator::build(node_base_t** root_node)
{
    IF_RETURN_FAILED(m_select->pSrc->nSrc != m_select->pSrc->nAlloc);
    IF_RETURN_FAILED(m_select->pSrc->nSrc > MAX_JOIN_TABLE);

    result_t ret;
    int32 tab_num = m_select->pSrc->nSrc;

    node_base_t* scan_nodes[MAX_JOIN_TABLE];    
    for (int32 i = 0; i < tab_num; i++) {
        
        // TODO(scott.zgeng@gmail.com): 需要增加异常情况退出的内存泄露
        scan_nodes[i] = new scan_node_t(i);
        IF_RETURN_FAILED(scan_nodes[i] == NULL);
        
        ret = scan_nodes[i]->init(m_parse, m_select);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);
    }

    node_base_t* root = scan_nodes[0];    
    ret = build_join(scan_nodes, tab_num, &root);
    IF_RETURN_FAILED(ret != RT_SUCCEEDED);


    



    return RT_SUCCEEDED;
}


scan_node_t::scan_node_t(int index)
{
    m_index = index;
}

scan_node_t::~scan_node_t()
{

}

result_t scan_node_t::init(Parse* parse, Select* select)
{
    return RT_FAILED;
}

void scan_node_t::uninit()
{

}

const char* scan_node_t::name()
{
    return "SCAN_NODE";
}

result_t scan_node_t::next(query_pack_t* pack)
{
    return RT_FAILED;
}



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

result_t join_node_t::next(query_pack_t* pack)
{
    return RT_FAILED;
}

const char* join_node_t::name()
{
    return "JOIN_NODE";
}

