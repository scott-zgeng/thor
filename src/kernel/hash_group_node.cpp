// hash_group_node.cpp by scott.zgeng@gmail.com 2014.08.29


#include "hash_group_node.h"
#include "statement.h"


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

template<typename T>
class aggr_op_t<T, AGGR_FUNC_SUM> : public aggr_op_base_t
{
    virtual void update(void* dst_row, void* src_row) {
        *(T*)dst_row += *(T*)src_row;
    }
};


template<typename T>
class aggr_op_t<T, AGGR_FUNC_MIN> : public aggr_op_base_t
{
    virtual void update(void* dst_row, void* src_row) {
        T& left = *(T*)dst_row;
        T& right = *(T*)src_row;

        if (left > right) {
            left = right;
        }
    }
};

template<>
class aggr_op_t<db_string, AGGR_FUNC_MIN> : public aggr_op_base_t
{
    virtual void update(void* dst_row, void* src_row) {
        db_string left = (db_string)dst_row;
        db_string right = (db_string)src_row;
        if (strcmp(left, right) > 0) {
            left = right;
        }
    }
};


template<typename T>
class aggr_op_t<T, AGGR_FUNC_MAX> : public aggr_op_base_t
{
    virtual void update(void* dst_row, void* src_row) {
        T& left = *(T*)dst_row;
        T& right = *(T*)src_row;

        if (left < right) left = right;
    }
};

template<>
class aggr_op_t<db_string, AGGR_FUNC_MAX> : public aggr_op_base_t
{
    virtual void update(void* dst_row, void* src_row) {
        db_string left = (db_string)dst_row;
        db_string right = (db_string)src_row;
        if (strcmp(left, right) < 0) {
            left = right;
        }
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




//----------------------------------------------------------
// aggr_table_t
//----------------------------------------------------------
aggr_table_t::aggr_table_t()
{
    m_hash_size = 0;
    m_hash_table = NULL;
    m_group_node = NULL;
    m_row_count = 0;
    m_mode = UNKNOWN_MODE;
    m_table_count = 0;
}

aggr_table_t::~aggr_table_t() 
{
    if (m_hash_table != NULL) {
        free(m_hash_table);
        m_hash_table = NULL;
    }
}


result_t aggr_table_t::init(hash_group_node_t* group_node, rowset_mode_t mode, db_uint32 table_count, db_uint32 row_count)
{
    m_group_node = group_node;
    m_mode = mode;
    m_table_count = table_count;

    m_hash_size = 393241; // TODO(scott.zgeng): add cala hash size function
    m_hash_table = (hash_node_t**)malloc(m_hash_size * sizeof(hash_node_t*));
    IF_RETURN_FAILED(m_hash_table == NULL);

    memset(m_hash_table, 0, sizeof(hash_node_t*)* m_hash_size);

    return RT_SUCCEEDED;
}

void aggr_table_t::init_complete()
{    
    database_t* db = m_group_node->statement()->database();
    m_group_table.init(db->get_mem_pool(), m_group_rows.row_len());
    m_aggr_table.init(db->get_mem_pool(), m_aggr_rows.row_len());
    m_hash_region.init(db->get_mem_pool());

    // TODO(scott.zgeng): 以下代码仅用来验证可行性，后面需要重构掉
    m_group_node->statement()->m_aggr_table_def = &m_aggr_rows;
}


result_t aggr_table_t::add_group_column(expr_base_t* expr) 
{
    group_op_base_t* op = create_group_op(expr->data_type());
    db_bool is_succ = m_group_ops.push_back(op);
    IF_RETURN_FAILED(!is_succ);
    op->offset = m_group_rows.row_len();

    return m_group_rows.add_column(expr);
}


expr_base_t* aggr_table_t::create_cast_expr(expr_base_t* expr, aggr_type_t aggr_type) 
{

    switch (expr->data_type())
    {
    case DB_INT8:
    case DB_INT16:
    case DB_INT32:
        return expr_factory_t::create_cast(DB_INT64, expr);
    case DB_FLOAT:
        return expr_factory_t::create_cast(DB_DOUBLE, expr);
    case DB_INT64:
    case DB_DOUBLE:
        return expr;
    default:
        return NULL;
    }
}


expr_base_t* aggr_table_t::conv_aggr_expr(expr_base_t* expr, aggr_type_t aggr_type) 
{
    switch (aggr_type) {
    case AGGR_FUNC_COUNT:
        return new expr_aggr_count_t(expr);
    case AGGR_FUNC_SUM:    
        return create_cast_expr(expr, aggr_type);
    default:
        return expr;
    }
}

result_t aggr_table_t::add_aggr_column(expr_base_t* expr, aggr_type_t aggr_type) 
{
    db_uint32 offset = m_aggr_rows.row_len();

    db_bool is_avg = (aggr_type == AGGR_FUNC_AVG);
    if (is_avg) aggr_type = AGGR_FUNC_SUM;

    expr_base_t* wrapper = conv_aggr_expr(expr, aggr_type);
    IF_RETURN_FAILED(wrapper == NULL);

    result_t ret = m_aggr_rows.add_column(wrapper);
    IF_RETURN_FAILED(ret != RT_SUCCEEDED);

    aggr_op_base_t* op = create_aggr_op(wrapper->data_type(), aggr_type);
    db_bool is_succ = m_aggr_ops.push_back(op);
    IF_RETURN_FAILED(!is_succ);
    op->offset = offset;

    // NOTE(scott.zgeng): AVG实际是有SUM和COUNT两列组合起来的
    if (is_avg) {
        result_t ret = add_aggr_column(expr, AGGR_FUNC_COUNT);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);
    }

    return RT_SUCCEEDED;
}

result_t aggr_table_t::build(rowset_t* rs, mem_stack_t* mem)
{
    result_t ret;
    mem_handle_t group_handle;
    ret = m_group_rows.next(rs, mem, group_handle);
    IF_RETURN_FAILED(ret != RT_SUCCEEDED);

    mem_handle_t aggr_handle;
    ret = m_aggr_rows.next(rs, mem, aggr_handle);
    IF_RETURN_FAILED(ret != RT_SUCCEEDED);

    db_byte* group_row = (db_byte*)group_handle.ptr();
    db_byte* aggr_row = (db_byte*)aggr_handle.ptr();

    for (db_uint32 i = 0; i < rs->count; i++) {
        ret = insert_update(group_row, aggr_row);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);

        group_row += m_group_rows.row_len();
        aggr_row += m_aggr_rows.row_len();
    }

    m_iterator.init(&m_aggr_table);

    return RT_SUCCEEDED;
}

result_t aggr_table_t::next(aggr_rowset_t* ars) 
{
    db_uint32 i;
    for (i = 0; i < SEGMENT_SIZE; i++) {
        void* ptr = m_iterator.next();
        if (ptr == NULL) break;
        ars->rows[i] = ptr;
    }

    ars->count = i;
    return RT_SUCCEEDED;
}

result_t aggr_table_t::insert_update(db_byte* group_row, db_byte* aggr_row) 
{
    db_uint32 hash_val = calc_hash(group_row);

    hash_node_t* node = m_hash_table[hash_val % m_hash_size];
    while (node != NULL) {
        if (node->hash_val == hash_val && is_equal(node->key, group_row)) {
            update(node->value, aggr_row);
            return RT_SUCCEEDED;
        }
        node = node->next;
    }

    return insert(hash_val, group_row, aggr_row);
}

result_t aggr_table_t::insert(db_uint32 hash_val, db_byte* group_row, db_byte* aggr_row) 
{
    // TODO(scott.zgeng): 目前只做了浅拷贝，后续看是否需要改为深拷贝

    hash_node_t* hash_node = (hash_node_t*)m_hash_region.alloc(sizeof(hash_node_t));
    IF_RETURN_FAILED(hash_node == NULL);

    hash_node->hash_val = hash_val;

    hash_node->key = (db_byte*)m_group_table.alloc();
    IF_RETURN_FAILED(hash_node->key == NULL);
    memcpy(hash_node->key, group_row, m_group_rows.row_len());

    hash_node->value = (db_byte*)m_aggr_table.alloc();
    IF_RETURN_FAILED(hash_node->value == NULL);
    memcpy(hash_node->value, aggr_row, m_aggr_rows.row_len());

    hash_node_t*& entry = m_hash_table[hash_val % m_hash_size];
    hash_node->next = entry;

    entry = hash_node;
    m_row_count++;

    return RT_SUCCEEDED;
}

void aggr_table_t::update(db_byte* dst_row, db_byte* src_row)
{
    for (db_uint32 i = 0; i < m_aggr_ops.size(); i++) {
        aggr_op_base_t* op = m_aggr_ops[i];
        op->update(dst_row + op->offset, src_row + op->offset);
    }
}

db_uint32 aggr_table_t::calc_hash(db_byte* row)
{
    db_uint32 hash_val = 0;
    for (db_uint32 i = 0; i <m_group_ops.size(); i++) {
        group_op_base_t* op = m_group_ops[i];
        hash_val = op->calc_hash(hash_val, row + op->offset);
    }
    return hash_val;
}


db_bool aggr_table_t::is_equal(db_byte* left_row, db_byte* right_row) 
{
    for (db_uint32 i = 0; i < m_group_ops.size(); i++) {
        group_op_base_t* op = m_group_ops[i];
        if (!op->is_equal(left_row + op->offset, right_row + op->offset)) {
            return false;
        }
    }
    return true;
}


//----------------------------------------------------------
// hash_group_node_t
//----------------------------------------------------------
hash_group_node_t::hash_group_node_t(statement_t* stmt, node_base_t* children)
{
    m_children = children;
    m_stmt = stmt;
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
    ret = m_aggr_table.init(this, m_children->rowset_mode(), m_children->table_count(), 0);
    IF_RETURN_FAILED(ret != RT_SUCCEEDED);
    
    expr_factory_t factory(m_stmt, m_children);
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

