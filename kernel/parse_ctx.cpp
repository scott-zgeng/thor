// parse_ctx.c by scott.zgeng@gmail.com 2014.07.11

#include <assert.h>



#include "parse_ctx.h"


int sqlite3SelectCreatePlan(Parse* pParse, Select* pSelete)
{
    if (!pParse->columnStorage) return SQLITE_OK;

    node_generator_t generator(pParse, pSelete);

    node_base_t* root = NULL;
    if (generator.build(&root) != RT_SUCCEEDED)
        return SQLITE_ERROR;

    pParse->pVdbe->pRootNode = root;

    return SQLITE_OK;
}



node_generator_t::node_generator_t(Parse* parse, Select* select)
{
    m_parse = parse;
    m_select = select;
}

node_generator_t::~node_generator_t()
{

}



result_t node_generator_t::build_join(node_base_t** scan_nodes, int32 tab_num, node_base_t** root)
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

result_t node_generator_t::build(node_base_t** root_node)
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

result_t node_generator_t::build_expression(Expr* expr, expr_base_t** root)
{
    assert(expr);
    
    expr_base_t* expr_node = create_expression(expr);
    IF_RETURN_FAILED(expr_node == NULL);
    
    result_t ret;
    if (expr->pLeft != NULL) {
        ret = build_expression(expr->pLeft, &expr_node->m_left);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);
    }

    if (expr->pRight != NULL) {
        ret = build_expression(expr->pRight, &expr_node->m_right);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);
    }


    return RT_SUCCEEDED;
}

expr_base_t* node_generator_t::create_expression(Expr* expr)
{
    switch (expr->op)
    {
    case TK_COLUMN:
        return new expr_column_t();
    default:
        return NULL;
    }
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


expr_column_t::expr_column_t()
{

}


expr_column_t::~expr_column_t()
{

}

result_t expr_column_t::init(Expr* expr)
{
    return RT_FAILED;
}

