// exec_node.cpp by scott.zgeng@gmail.com 2014.08.08

#include "exec_node.h"

#include "expression.h"


#include "project_node.h"
#include "scan_node.h"
#include "join_node.h"
#include "hash_group_node.h"
#include "sort_node.h"


//-----------------------------------------------------------------------------
// node_generator_t
//-----------------------------------------------------------------------------
node_generator_t::node_generator_t(database_t* db, Parse* parse, Select* select)
{
    m_database = db;
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
        scan_nodes[i] = new scan_node_t(m_database, i);
        IF_RETURN_FAILED(scan_nodes[i] == NULL);

        ret = scan_nodes[i]->init(m_parse, m_select);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);
    }

    node_base_t* root = scan_nodes[0];
    ret = build_join(scan_nodes, tab_num, &root);
    IF_RETURN_FAILED(ret != RT_SUCCEEDED);

    if (m_select->pGroupBy != NULL && m_select->pGroupBy->nExpr > 0) {
        hash_group_node_t* group_node = new hash_group_node_t(m_database, root);
        IF_RETURN_FAILED(group_node == NULL);
        
        ret = group_node->init(m_parse, m_select);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);

        root = group_node;
    }

    if (m_select->pOrderBy != NULL && m_select->pOrderBy->nExpr > 0) {
        sort_node_t* sort_node = new sort_node_t(root);
        IF_RETURN_FAILED(sort_node == NULL);

        ret = sort_node->init(m_parse, m_select);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);

        root = sort_node;
    }

    node_base_t* project_node = new project_node_t(m_database, root);
    IF_RETURN_FAILED(project_node == NULL);

    ret = project_node->init(m_parse, m_select);
    IF_RETURN_FAILED(ret != RT_SUCCEEDED);


    *root_node = project_node;
    return RT_SUCCEEDED;
}



