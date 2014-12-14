// join_node.cpp by scott.zgeng@gmail.com 2014.08.27




#include "join_node.h"




//-----------------------------------------------------------------------------
// join_node_t
//-----------------------------------------------------------------------------
hash_join_node_t::hash_join_node_t(statement_t* stmt, node_base_t* left, node_base_t* right, 
    const db_char* left_table, const db_char* right_table)
{

    m_stmt = stmt;
    m_left = left;
    m_right = right;

    strcpy(m_left_name, left_table);
    strcpy(m_right_name, right_table);

    m_left_expr = NULL;
    m_right_expr = NULL;
}

hash_join_node_t::~hash_join_node_t()
{

}


result_t hash_join_node_t::init(Parse* parse, Select* select)
{
    result_t ret;
    Expr* join_cond = expr_factory_t::find_join_condition(select->pWhere, m_left_name, m_right_name);
    IF_RETURN_FAILED(join_cond == NULL);

    expr_factory_t left_factory(m_stmt, m_left);    
    ret = left_factory.build(join_cond->pLeft, &m_left_expr);
    IF_RETURN_FAILED(ret != RT_SUCCEEDED);

    expr_factory_t right_factory(m_stmt, m_right);
    ret = right_factory.build(join_cond->pRight, &m_right_expr);
    IF_RETURN_FAILED(ret != RT_SUCCEEDED);

    return RT_SUCCEEDED;
}

void hash_join_node_t::uninit()
{

}



result_t hash_join_node_t::next(rowset_t* rs, mem_stack_t* mem)
{



    return RT_FAILED;
}


result_t hash_join_node_t::build_hash_table(mem_stack_t* mem)
{   
    result_t ret;

    while (true) {
        ret = m_right->next(&m_rowset, mem);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);

        mem_handle_t mem_handle;
        ret = m_right_expr->calc(&m_rowset, mem, mem_handle);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);

        if (m_rowset.count <= SEGMENT_SIZE) break;
    }

    return RT_SUCCEEDED;
}

