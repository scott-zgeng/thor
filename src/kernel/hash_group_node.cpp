// hash_group_node.cpp by scott.zgeng@gmail.com 2014.08.29


#include "hash_group_node.h"






row_segement_t::row_segement_t()
{
    m_row_len = 0;
}


row_segement_t::~row_segement_t()
{
}


result_t row_segement_t::add_column(expr_base_t* expr)
{
    assert(expr != NULL);

    expr_item_t item;
    item.expr = expr;
    item.offset = m_row_len;
    item.size = calc_data_len(expr->data_type());
    
    m_row_len += item.size;

    bool is_succ = m_columns.push_back(item);
    IF_RETURN_FAILED(!is_succ);

    return RT_SUCCEEDED;
}


result_t row_segement_t::next(rowset_t* rs, mem_stack_t* mem, mem_handle_t& result)
{
    result_t ret;
    
    mem->alloc_memory(m_row_len * SEGMENT_SIZE, result);    
    db_byte* row_segment = (db_byte*)result.ptr();

    for (db_uint32 i = 0; i < m_columns.size(); i++) {        
        mem_handle_t handle;
        expr_item_t& item = m_columns[i];
        ret = item.expr->calc(rs, mem, handle);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);
        db_byte* src = (db_byte*)handle.ptr();
        db_byte* dst = row_segment + item.offset;
        db_uint32 size = item.size;            

        for (db_uint32 n = 0; n < rs->count; n++) {
            memcpy(dst, src, size);            
            src += size;
            dst += m_row_len;
        }
    }

    return RT_SUCCEEDED;
}



template<typename T>
struct group_op_t : public group_op_base_t
{
    virtual db_uint32 calc_hash(db_uint32 hash, void* row) {
        unsigned int seed = 131;        
        char* input = (char*)row;

        for (size_t i = 0; i < sizeof(T); i++) {
            hash = hash * seed + input[i];
        }
        return hash;
    }

    virtual db_bool is_equal(void* left_row, void* right_row) {
        return (*(T*)left_row == *(T*)right_row); 
    }
};


template<>
struct group_op_t<db_string>: public group_op_base_t
{
    virtual db_uint32 calc_hash(db_uint32 hash, void* row) {
        unsigned int seed = 131;
        char* input = (char*)row;

        while (*input != 0) {
            hash = hash * seed + *input;
            input++;
        }
        return hash;
    }

    virtual db_bool is_equal(void* left_row, void* right_row) {
        return (strcmp((db_char*)left_row, (db_char*)right_row) == 0);        
    }
};



group_op_base_t* aggr_table_t::create_group_op(data_type_t type)
{
    switch (type)
    {    
    case DB_INT8:
        return new group_op_t<db_int8>();
    case DB_INT16:
        return new group_op_t<db_int16>();
    case DB_INT32:
        return new group_op_t<db_int32>();
    case DB_INT64:
        return new group_op_t<db_int64>();
    case DB_FLOAT:
        return new group_op_t<db_float>();
    case DB_DOUBLE:
        return new group_op_t<db_double>();
    case DB_STRING:
        return new group_op_t<db_string>();
    default:
        return NULL;
    }
}


template<typename T, aggr_type_t AT>
struct aggr_op_t : public aggr_op_base_t 
{
    virtual void update(void* dst_row, void* src_row) {
        assert(false);
    }
};

template<typename T>
class aggr_op_t<T, AGGR_COLUMN> : public aggr_op_base_t
{
    virtual void update(void* dst_row, void* src_row) {}
};


template<>
class aggr_op_t<db_int64, AGGR_FUNC_COUNT> : public aggr_op_base_t
{
    virtual void update(void* dst_row, void* src_row) {
        *(db_int64*)dst_row += *(db_int64*)src_row;
    }
};



template<aggr_type_t AT>
aggr_op_base_t* aggr_table_t::create_min_max_aggr_op(data_type_t type)
{
    switch (type)
    {
    case DB_INT8:
        return new aggr_op_t<db_int8, AT>();
    case DB_INT16:
        return new aggr_op_t<db_int16, AT>();
    case DB_INT32:
        return new aggr_op_t<db_int32, AT>();
    case DB_INT64:
        return new aggr_op_t<db_int64, AT>();
    case DB_FLOAT:
        return new aggr_op_t<db_float, AT>();
    case DB_DOUBLE:
        return new aggr_op_t<db_double, AT>();
    case DB_STRING:
        return new aggr_op_t<db_string, AT>();
    default:
        return NULL;        
    }
}


template<aggr_type_t AT>
aggr_op_base_t* aggr_table_t::create_sum_avg_aggr_op(data_type_t type)
{
    switch (type)
    {
    case DB_INT64:
        return new aggr_op_t<db_int64, AT>();
    case DB_DOUBLE:
        return new aggr_op_t<db_double, AT>();
    default:
        return NULL;
    }
}

aggr_op_base_t* aggr_table_t::create_aggr_op(data_type_t type, aggr_type_t aggr_type)
{
    switch (aggr_type)
    {    
    case AGGR_COLUMN:
        return new aggr_op_t<void, AGGR_COLUMN>();
    case AGGR_FUNC_COUNT:
        return new aggr_op_t<db_int64, AGGR_FUNC_COUNT>();
    case AGGR_FUNC_SUM:
        return create_sum_avg_aggr_op<AGGR_FUNC_SUM>(type);
    case AGGR_FUNC_AVG:
        return create_sum_avg_aggr_op<AGGR_FUNC_AVG>(type);
    case AGGR_FUNC_MIN:
        return create_min_max_aggr_op<AGGR_FUNC_MIN>(type);        
    case AGGR_FUNC_MAX:
        return create_min_max_aggr_op<AGGR_FUNC_MAX>(type);
    default:
        return NULL;
    }    
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
    ret = m_aggr_table.init(m_database, m_children->rowset_mode(), m_children->table_count(), 0);
    IF_RETURN_FAILED(ret != RT_SUCCEEDED);
    
    expr_factory_t factory(m_database, m_children);
    for (db_int32 i = 0; i < select->pEList->nExpr; i++) {        
        ret = add_aggr_sub_expr(factory, select->pEList->a[i].pExpr);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);
    }

    for (db_int32 i = 0; i < select->pGroupBy->nExpr; i++) {
        expr_base_t* expr = NULL;
        ret = factory.build(select->pGroupBy->a[i].pExpr, &expr);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);

        ret = m_aggr_table.add_group_column(expr); 
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);
    }

    m_sub_rowset = create_rowset(m_children->rowset_mode(), m_children->table_count());    
    IF_RETURN_FAILED(m_sub_rowset == NULL);

    m_aggr_table.init_complete();
    
    m_first = true;
    return RT_SUCCEEDED;
}


result_t hash_group_node_t::add_aggr_sub_expr(expr_factory_t& factory, Expr* pExpr)
{
    result_t ret;
    
    if (pExpr->op == TK_AGG_FUNCTION) {
        assert(pExpr->x.pList == NULL || pExpr->x.pList->nExpr == 1);

        expr_base_t* expr = NULL;
        
        if (pExpr->x.pList != NULL) {
            ret = factory.build(pExpr->x.pList->a->pExpr, &expr);
            IF_RETURN_FAILED(ret != RT_SUCCEEDED);
        }

        aggr_type_t aggr_type = get_aggr_type(pExpr->u.zToken);
        IF_RETURN_FAILED(aggr_type == AGGR_UNKNOWN);

        ret = m_aggr_table.add_aggr_column(expr, aggr_type);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);

        return RT_SUCCEEDED;

    } else if (pExpr->op == TK_AGG_COLUMN) {        
        Expr clone_expr;
        memcpy(&clone_expr, pExpr, sizeof(clone_expr));
        clone_expr.op = TK_COLUMN;                

        expr_base_t* expr = NULL;
        ret = factory.build(&clone_expr, &expr);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);

        ret = m_aggr_table.add_aggr_column(expr, AGGR_COLUMN);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);

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



aggr_type_t hash_group_node_t::get_aggr_type(const char* token)
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
        ret = m_children->next(m_sub_rowset, mem);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);

        if (m_sub_rowset->count == 0) break;

        ret = m_aggr_table.build(m_sub_rowset, mem);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);

        if (m_sub_rowset->count < SEGMENT_SIZE) break;
    }

    return RT_SUCCEEDED;
}

result_t hash_group_node_t::next(rowset_t* rs, mem_stack_t* mem)
{
    assert(rs->mode == AGGR_TABLE_MODE);
    aggr_rowset_t* ars = (aggr_rowset_t*)rs;

    result_t ret;
    if UNLIKELY(m_first) {
        ret = build(mem);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);
        m_first = false;
    }

    return m_aggr_table.next(ars);
}

