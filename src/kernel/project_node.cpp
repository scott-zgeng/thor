// project_node.h by scott.zgeng@gmail.com 2014.08.29


#include "project_node.h"


//-----------------------------------------------------------------------------
// project_node_t
//-----------------------------------------------------------------------------
project_node_t::project_node_t(database_t* db, node_base_t* children)
{
    m_database = db;         
    m_children = children;
    m_sub_rowset = NULL;
}

project_node_t::~project_node_t()
{

}

result_t project_node_t::init(Parse* parse, Select* select)
{
    ExprList* expr_list = select->pEList;
    expr_factory_t factory(m_database, m_children);
    result_t ret;
    for (db_int32 i = 0; i < expr_list->nExpr; i++) {
        expr_base_t* expr = NULL;
        ret = factory.build(expr_list->a[i].pExpr, &expr);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);

        m_expr_columns.push_back(expr);
    }

    m_sub_rowset = create_rowset(m_children->rowset_mode(), m_children->table_count());    
    IF_RETURN_FAILED(m_sub_rowset == NULL);

    m_expr_values.resize(expr_list->nExpr);

    return RT_SUCCEEDED;
}

void project_node_t::uninit()
{
}



result_t project_node_t::next(rowset_t* rs, mem_stack_t* mem)
{
    return RT_FAILED;
}


result_t project_node_t::next()
{
    result_t ret;

    m_mem.reset();

    ret = m_children->next(m_sub_rowset, &m_mem);
    IF_RETURN_FAILED(ret != RT_SUCCEEDED);

    if (m_sub_rowset->count == 0)
        return RT_SUCCEEDED;

    for (db_uint32 i = 0; i < m_expr_columns.size(); i++) {

        mem_handle_t mem_handle;
        ret = m_expr_columns[i]->calc(m_sub_rowset, &m_mem, mem_handle);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);

        m_expr_values[i] = mem_handle.transfer();
    }

    return RT_SUCCEEDED;
}


