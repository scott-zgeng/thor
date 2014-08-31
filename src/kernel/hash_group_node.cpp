// hash_group_node.cpp by scott.zgeng@gmail.com 2014.08.29


#include "hash_group_node.h"



db_uint32 calc_data_len(data_type_t type)
{
    switch (type)
    {

    case DB_INT8:
        return sizeof(db_int8);
    case DB_INT16:
        return sizeof(db_int16);
    case DB_INT32:
        return sizeof(db_int32);
    case DB_INT64:
        return sizeof(db_int64);
    case DB_FLOAT:
        return sizeof(db_float);
    case DB_DOUBLE:
        return sizeof(db_double);
    case DB_STRING:
        return sizeof(db_string);    
    default:
        return (-1); 
    }
}



row_table_t::row_table_t()
{
    m_row_len = 0;
}

row_table_t::~row_table_t()
{
    m_mem_region.release();
}


result_t row_table_t::init(database_t* db)
{
    m_mem_region.init(db->get_mem_pool());
    return RT_SUCCEEDED;
}


result_t row_table_t::add_column(data_type_t type, db_uint32 len)
{
    m_field_pos.push_back(m_row_len);    
    m_row_len += len;

    return RT_SUCCEEDED;
}














hash_group_node_t::hash_group_node_t(database_t* db, node_base_t* children) 
{
    m_children = children;
    m_database = db;
    m_first = true;
}

hash_group_node_t::~hash_group_node_t()
{

}


result_t hash_group_node_t::init(Parse* parse, Select* select)
{
    // NOTE(scott.zgeng): 目前计划有最大支持列数限制，如果有必要可以去掉
    IF_RETURN_FAILED(select->pEList->nExpr > MAX_AGGR_COLUMNS);

    result_t ret;
    expr_factory_t factory(m_database);    
    for (db_int32 i = 0; i < select->pEList->nExpr; i++) {        
        ret = add_aggr_sub_expr(factory, select->pEList->a[i].pExpr);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);
    }

    for (db_int32 i = 0; i < select->pGroupBy->nExpr; i++) {
        group_item_t item;
        item.values = NULL;
        ret = factory.build(select->pGroupBy->a[i].pExpr, &item.expr);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);

        bool is_succ = m_group_columns.push_back(item);
        IF_RETURN_FAILED(!is_succ);
    }

    ret = m_rowset.init(m_children->rowid_size());
    IF_RETURN_FAILED(ret != RT_SUCCEEDED);

    m_first = true;

    return RT_SUCCEEDED;
}


result_t hash_group_node_t::add_aggr_sub_expr(expr_factory_t& factory, Expr* pExpr)
{
    result_t ret;
    
    if (pExpr->op == TK_AGG_FUNCTION) {
        assert(pExpr->x.pList == NULL || pExpr->x.pList->nExpr == 1);

        aggr_item_t item = {NULL, AGGR_UNKNOWN, NULL};        
        
        if (pExpr->x.pList != NULL) {
            ret = factory.build(pExpr->x.pList->a->pExpr, &item.expr);
            IF_RETURN_FAILED(ret != RT_SUCCEEDED);
        }

        item.type = get_aggr_type(pExpr->u.zToken);
        IF_RETURN_FAILED(item.type == AGGR_UNKNOWN);

        bool is_succ = m_aggr_columns.push_back(item);
        IF_RETURN_FAILED(!is_succ);

        return RT_SUCCEEDED;

    } else if (pExpr->op == TK_AGG_COLUMN) {        
        Expr clone_expr;
        memcpy(&clone_expr, pExpr, sizeof(clone_expr));
        clone_expr.op = TK_COLUMN;        
        aggr_item_t item = { NULL, AGGR_COLUMN, NULL };

        ret = factory.build(&clone_expr, &item.expr);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);

        bool is_succ = m_aggr_columns.push_back(item);
        IF_RETURN_FAILED(!is_succ);

        return RT_SUCCEEDED;

    } else {}


    if (pExpr->pLeft != NULL) {
        ret = add_aggr_sub_expr(factory, pExpr->pLeft);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);
    }


    if (pExpr->pRight != NULL) {
        ret = add_aggr_sub_expr(factory, pExpr->pRight);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);
    }

    return RT_SUCCEEDED;
}



hash_group_node_t::aggr_type_t hash_group_node_t::get_aggr_type(const char* token)
{
    assert(token != NULL);

    if (strcmp(token, "count") == 0)
        return AGGR_FUNC_COUNT;
    else if (strcmp(token, "min") == 0)
        return AGGR_FUNC_MIN;
    else if (strcmp(token, "max") == 0)
        return AGGR_FUNC_MAX;
    else if (strcmp(token, "sum") == 0)
        return AGGR_FUNC_SUM;
    else if (strcmp(token, "avg") == 0)
        return AGGR_FUNC_AVG;
    else
        return AGGR_UNKNOWN;
}


void hash_group_node_t::uninit()
{

}


result_t hash_group_node_t::build(mem_stack_t* mem)
{
    result_t ret;

    while (true) {
        ret = m_children->next(&m_rowset, mem);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);

        if (m_rowset.count() == 0) break;

        mem_handle_t spin_handle;
        mem->spin_memory(spin_handle);

        for (db_uint32 i = 0; i < m_group_columns.size(); i++) {
            mem_handle_t handle;
            group_item_t& item = m_group_columns[i];
            ret = item.expr->calc(&m_rowset, mem, handle);
            IF_RETURN_FAILED(ret != RT_SUCCEEDED); 

            item.values = handle.transfer();            
        }

        for (db_uint32 i = 0; i < m_aggr_columns.size(); i++) {
            mem_handle_t handle;
            aggr_item_t& item = m_aggr_columns[i];
            ret = item.expr->calc(&m_rowset, mem, handle);
            IF_RETURN_FAILED(ret != RT_SUCCEEDED);

            item.values = handle.transfer();
        }


        if (m_rowset.count() < SEGMENT_SIZE) break;
    }

    return RT_SUCCEEDED;
}

result_t hash_group_node_t::next(rowset_t* rows, mem_stack_t* mem)
{
    result_t ret;
    if UNLIKELY(m_first) {
        ret = build(mem);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);
        m_first = false;
    }
        


    //m_aggr_table.
    return RT_FAILED;
}


db_int32 hash_group_node_t::rowid_size()
{
    return 0;
}


