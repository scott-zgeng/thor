// exec_node.cpp by scott.zgeng@gmail.com 2014.08.08

#include "exec_node.h"
#include "expression.h"


node_generator_t::node_generator_t(Parse* parse, Select* select)
{
    m_parse = parse;
    m_select = select;
}

node_generator_t::~node_generator_t()
{

}


result_t node_generator_t::build_join(node_base_t** scan_nodes, db_int32 tab_num, node_base_t** root)
{
    if (tab_num == 1) return RT_SUCCEEDED;

    result_t ret;
    node_base_t* node = scan_nodes[0];
    for (db_int32 i = 1; i < tab_num; i++) {
        node_base_t* new_node = new join_node_t(node, scan_nodes[i]);
        IF_RETURN_FAILED(new_node == NULL);

        ret = new_node->init(m_parse, m_select);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);

        node = new_node;
    }

    return RT_SUCCEEDED;
}

result_t node_generator_t::build(node_base_t** root_node)
{
    IF_RETURN_FAILED(m_select->pSrc->nSrc != m_select->pSrc->nAlloc);
    IF_RETURN_FAILED(m_select->pSrc->nSrc > MAX_JOIN_TABLE);

    result_t ret;
    db_int32 tab_num = m_select->pSrc->nSrc;

    node_base_t* scan_nodes[MAX_JOIN_TABLE];
    for (db_int32 i = 0; i < tab_num; i++) {

        // TODO(scott.zgeng@gmail.com): 需要增加异常情况退出的内存泄露
        scan_nodes[i] = new scan_node_t(i);
        IF_RETURN_FAILED(scan_nodes[i] == NULL);

        ret = scan_nodes[i]->init(m_parse, m_select);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);
    }

    node_base_t* root = scan_nodes[0];
    ret = build_join(scan_nodes, tab_num, &root);
    IF_RETURN_FAILED(ret != RT_SUCCEEDED);

    *root_node = root;

    return RT_SUCCEEDED;
}



scan_node_t::scan_node_t(int index)
{
    m_index = index;
    m_where = NULL;
}

scan_node_t::~scan_node_t()
{

}

result_t scan_node_t::init(Parse* parse, Select* select)
{
    result_t ret;

    ret = expr_base_t::build(select->pWhere, &m_where);
    IF_RETURN_FAILED(ret != RT_SUCCEEDED);

    return RT_SUCCEEDED;
}

void scan_node_t::uninit()
{

}

// 在调用next之前，应用需要先调用该节点size()，计算用来缓存行号的空间，并且双方已经协商好数据格式
result_t scan_node_t::next(context_t* ctx)
{
    assert(!m_where->has_null());
    result_t ret;
    mem_handle_t result;

    expr_context_t* ectx = new expr_context_t();

    ret = m_where->calc(ectx, result);
    IF_RETURN_FAILED(ret != RT_SUCCEEDED);

    db_int8* ptr = (db_int8*)result.ptr();

    // 获取所有有效的结果集合，去掉不符合的行
    for (int i = 0; i < ectx->row_count(); i++) {
        if (ptr[i] != 0) {
            printf("found %d\n", i);
        }
    }




    return RT_SUCCEEDED;
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

result_t join_node_t::next(context_t* pack)
{
    return RT_FAILED;
}

