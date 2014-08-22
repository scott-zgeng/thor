// statement.cpp by scott.zgeng@gmail.com 2014.08.21


#include "statement.h"
#include "exec_node.h"



//-----------------------------------------------------------------------------
// select_stmt_t
//-----------------------------------------------------------------------------
select_stmt_t::select_stmt_t(database_t* db)
{
    m_root = NULL;
    m_database = db;
}


select_stmt_t::~select_stmt_t()
{
}


result_t select_stmt_t::prepare(Parse *pParse, Select *pSelect)
{
    node_generator_t generator(pParse, pSelect);
        
    node_base_t* root = NULL;
    result_t ret = generator.build(&root);
    IF_RETURN_FAILED(ret != RT_SUCCEEDED);        

    // NOTE(scott.zgeng): 查询的根节点一定是project节点
    m_root = (project_node_t*)root;
    return RT_SUCCEEDED;
}


db_int32 select_stmt_t::next()
{        
    // TODO(scott.zgeng): project节点的数据结构需要提到STMT里面来
    if (m_root->next() != RT_SUCCEEDED)
        return SQLITE_ERROR;
    
    if (m_root->count() < SEGMENT_SIZE)
        return SQLITE_DONE;
    
    return SQLITE_ROW;    
}



//-----------------------------------------------------------------------------
// insert_stmt_t
//-----------------------------------------------------------------------------
insert_stmt_t::insert_stmt_t(database_t* db)
{
    m_database = db;
    m_table = NULL;
}


insert_stmt_t::~insert_stmt_t()
{
}



result_t insert_stmt_t::prepare(Parse *pParse, SrcList *pTabList, Select *pSelect, IdList *pColumn, int onError)
{
    assert(pTabList->nSrc == 1);
    char* name = pTabList->a[0].zName;
    assert(name != NULL);

    m_table = m_database->find_table(name);
    IF_RETURN_FAILED(m_table == NULL);

    // if (pColumn == NULL) 表示 没有输入列名称，则直接按照顺序插入
    assert(pColumn == NULL);

    ExprList* insert_values = pSelect->pEList;

    expr_base_t* expr;
    result_t ret;
    for (db_int32 i = 0; i < insert_values->nExpr; i++) {
        ret = expr_base_t::build(insert_values->a[i].pExpr, &expr);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);

        bool is_succ = m_insert_values.push_back(expr);
        IF_RETURN_FAILED(!is_succ);
    }

    return RT_SUCCEEDED;
}

db_int32 insert_stmt_t::next()
{
    m_mem.reset();
    mem_handle_t handle;
    result_t ret;
    for (db_uint32 i = 0; i < m_table->get_column_count(); i++) {
        expr_base_t* expr = m_insert_values[i];

        expr->calc(NULL, &m_mem, handle);
        column_base_t* column = m_table->get_column(i);
        ret = column->insert(handle.ptr(), 1);
        if (ret != RT_SUCCEEDED)
            return SQLITE_ERROR;
    }
    

    return SQLITE_DONE;
}


