// expression.cpp by scott.zgeng@gmail.com 2014.08.5


#include <assert.h>
#include "expression.h"
#include "exec_node.h"
#include "statement.h"

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


aggr_type_t get_aggr_type(const char* token)
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


expr_factory_t::expr_factory_t(statement_t* stmt, node_base_t* node)
{
    m_stmt = stmt;
    m_mode = node->rowset_mode();
    m_table_count = node->table_count();    
}


data_type_t expr_factory_t::get_column_type(const char* name, db_int32 column_id)
{
    column_table_t* table = m_stmt->database()->find_table(name);
    if (table == NULL) return DB_UNKNOWN;
    column_base_t* column = table->get_column(column_id);
    if (column == NULL) return DB_UNKNOWN;
    return column->data_type();
}


expr_aggr_t* expr_factory_t::create_aggr_column(Expr* expr)
{
    assert(expr->pTab->zName != NULL);
    data_type_t type = get_column_type(expr->pTab->zName, expr->iColumn);

    switch (type)
    {
    case DB_INT8:
        return new expr_aggr_column_t<db_int8>();
    case DB_INT16:
        return new expr_aggr_column_t<db_int16>();
    case DB_INT32:
        return new expr_aggr_column_t<db_int32>();
    case DB_INT64:
        return new expr_aggr_column_t<db_int64>();
    case DB_FLOAT:
        return new expr_aggr_column_t<db_float>();
    case DB_DOUBLE:
        return new expr_aggr_column_t<db_double>();
    case DB_STRING:
        return new expr_aggr_column_t<db_string>();
    default:
        return NULL;
    }
}



expr_aggr_t* expr_factory_t::create_aggr_function(Expr* expr, db_uint32 index)
{
    row_segement_t::expr_item_t& item = m_stmt->m_aggr_table_def->m_columns[index];
    data_type_t data_type = item.expr->data_type();
    aggr_type_t aggr_type = get_aggr_type(expr->u.zToken);

    if (aggr_type == AGGR_FUNC_COUNT) {
        return new expr_aggr_column_t<db_int64>();
    }
    
    switch (data_type)
    {
    case DB_INT8:
        return new expr_aggr_column_t<db_int8>();
    case DB_INT16:
        return new expr_aggr_column_t<db_int16>();
    case DB_INT32:
        return new expr_aggr_column_t<db_int32>();
    case DB_INT64:
        return new expr_aggr_column_t<db_int64>();
    case DB_FLOAT:
        return new expr_aggr_column_t<db_float>();
    case DB_DOUBLE:
        return new expr_aggr_column_t<db_double>();
    case DB_STRING:
        return new expr_aggr_column_t<db_string>();
    default:
        return NULL;
    }
}


expr_base_t* expr_factory_t::create_column(Expr* expr)
{    
    assert(expr->pTab->zName != NULL);
    data_type_t type = get_column_type(expr->pTab->zName, expr->iColumn);

    switch (type)        
    {
    case DB_INT8:
        return new expr_column_t<db_int8>();
    case DB_INT16:
        return new expr_column_t<db_int16>();
    case DB_INT32:
        return new expr_column_t<db_int32>();
    case DB_INT64:
        return new expr_column_t<db_int64>();
    case DB_FLOAT:
        return new expr_column_t<db_float>();
    case DB_DOUBLE:
        return new expr_column_t<db_double>();
    case DB_STRING:
        return new expr_column_t<db_string>();
    default:
        return NULL;
    }
}

expr_base_t* expr_factory_t::create_integer(Expr* expr)
{
    db_int64 value = expr->u.iValue;

    if (CHAR_MIN <= value && value <= CHAR_MAX)
        return new expr_integer_t<db_int8>;
    else if (SHRT_MIN <= value && value <= SHRT_MAX)
        return new expr_integer_t<db_int16>;
    else if (INT_MIN <= value && value <= INT_MAX)
        return new expr_integer_t<db_int32>;
    else
        return new expr_integer_t<db_int64>;
}


template<int OP_TYPE>
expr_base_t* expr_factory_t::create_logic_op(data_type_t type, expr_base_t* left, expr_base_t* right)
{
    switch (type)
    {    
    case DB_INT8:
        return new binary_expr_t<OP_TYPE, db_int8, db_int8>(left, right);
    case DB_INT16:
        return new binary_expr_t<OP_TYPE, db_int16, db_int8>(left, right);
    case DB_INT32:
        return new binary_expr_t<OP_TYPE, db_int32, db_int8>(left, right);
    case DB_INT64:
        return new binary_expr_t<OP_TYPE, db_int64, db_int8>(left, right);
    case DB_FLOAT:
        return new binary_expr_t<OP_TYPE, db_float, db_int8>(left, right);
    case DB_DOUBLE:
        return new binary_expr_t<OP_TYPE, db_double, db_int8>(left, right);
    case DB_STRING:
        return new binary_expr_t<OP_TYPE, db_string, db_int8>(left, right);
    default:
        return NULL;
    }
}

template<int OP_TYPE>
expr_base_t* expr_factory_t::create_arith_op(data_type_t type, expr_base_t* left, expr_base_t* right)
{
    switch (type)
    {
    case DB_INT64:
        return new binary_expr_t<OP_TYPE, db_int64, db_int64>(left, right);
    case DB_DOUBLE:
        return new binary_expr_t<OP_TYPE, db_double, db_double>(left, right);
    default:
        return NULL;
    }
}


expr_base_t* expr_factory_t::create_binary(Expr* expr, expr_base_t* left, expr_base_t* right)
{
    assert(left->data_type() == right->data_type());
    data_type_t type = left->data_type();

    switch (expr->op)
    {
    case TK_AND:
        return create_logic_op<TK_AND>(type, left, right);
    case TK_OR:
        return create_logic_op<TK_OR>(type, left, right);

    case TK_NE:
        return create_logic_op<TK_NE>(type, left, right);
    case TK_EQ:
        return create_logic_op<TK_EQ>(type, left, right);
    case TK_GT:
        return create_logic_op<TK_GT>(type, left, right);
    case TK_GE:
        return create_logic_op<TK_GE>(type, left, right);
    case TK_LT:
        return create_logic_op<TK_LT>(type, left, right);
    case TK_LE:
        return create_logic_op<TK_LE>(type, left, right);

    case TK_PLUS:        
        return create_arith_op<TK_PLUS>(type, left, right);
    case TK_MINUS:
        return create_arith_op<TK_MINUS>(type, left, right);
    case TK_STAR:
        return create_arith_op<TK_STAR>(type, left, right);
    case TK_SLASH:
        return create_arith_op<TK_SLASH>(type, left, right);
        
        
    default:
        return NULL;
    }
}



expr_base_t* expr_factory_t::create_instance(Expr* expr, expr_base_t* left, expr_base_t* right)
{
    switch (expr->op)
    {
    case TK_AGG_COLUMN:
    case TK_COLUMN:
        return create_column(expr);
    case TK_INTEGER:
        return create_integer(expr); 
    case TK_STRING:
        return new expr_string_t();
    default:
        return create_binary(expr, left, right);
    }
}




data_type_t expr_factory_t::cast_type(int op_type, data_type_t left, data_type_t right)
{
    // NOTE(scott.zgeng): 逻辑运算转为能覆盖两者范围的数据类型
    const static data_type_t s_logic_matrix[DB_MAX_TYPE][DB_MAX_TYPE] = {
        //                unknown     int8        int16       int32       int64       float       double      string
        /* unknown */   { DB_UNKNOWN, DB_UNKNOWN, DB_UNKNOWN, DB_UNKNOWN, DB_UNKNOWN, DB_UNKNOWN, DB_UNKNOWN, DB_UNKNOWN }, 
        /* int8 */      { DB_UNKNOWN, DB_INT8,    DB_INT16,   DB_INT32,   DB_INT64,   DB_FLOAT,   DB_DOUBLE,  DB_UNKNOWN },
        /* int16 */     { DB_UNKNOWN, DB_INT16,   DB_INT16,   DB_INT32,   DB_INT64,   DB_FLOAT,   DB_DOUBLE,  DB_UNKNOWN }, 
        /* int32 */     { DB_UNKNOWN, DB_INT32,   DB_INT32,   DB_INT32,   DB_INT64,   DB_DOUBLE,  DB_DOUBLE,  DB_UNKNOWN },  
        /* int64 */     { DB_UNKNOWN, DB_INT64,   DB_INT64,   DB_INT64,   DB_INT64,   DB_DOUBLE,  DB_DOUBLE,  DB_UNKNOWN }, 
        /* float */     { DB_UNKNOWN, DB_FLOAT,   DB_FLOAT,   DB_DOUBLE,  DB_DOUBLE , DB_FLOAT,   DB_DOUBLE,  DB_UNKNOWN }, 
        /* double */    { DB_UNKNOWN, DB_DOUBLE,  DB_DOUBLE,  DB_DOUBLE,  DB_DOUBLE,  DB_DOUBLE,  DB_DOUBLE,  DB_UNKNOWN },  
        /* string */    { DB_UNKNOWN, DB_UNKNOWN, DB_UNKNOWN, DB_UNKNOWN, DB_UNKNOWN, DB_UNKNOWN, DB_UNKNOWN, DB_STRING  }, 
    };

    // NOTE(scott.zgeng): 算数运算全部转为最大的数据类型计算，整形转换为INT64, 浮点型转换为DOUBLE
    const static data_type_t s_arith_matrix[DB_MAX_TYPE][DB_MAX_TYPE] = {
        //                unknown     int8        int16       int32       int64       float       double      string
        /* unknown */   { DB_UNKNOWN, DB_UNKNOWN, DB_UNKNOWN, DB_UNKNOWN, DB_UNKNOWN, DB_UNKNOWN, DB_UNKNOWN, DB_UNKNOWN },   
        /* int8 */      { DB_UNKNOWN, DB_INT64,   DB_INT64,   DB_INT64,   DB_INT64,   DB_DOUBLE,  DB_DOUBLE,  DB_UNKNOWN }, 
        /* int16 */     { DB_UNKNOWN, DB_INT64,   DB_INT64,   DB_INT64,   DB_INT64,   DB_DOUBLE,  DB_DOUBLE,  DB_UNKNOWN }, 
        /* int32 */     { DB_UNKNOWN, DB_INT64,   DB_INT64,   DB_INT64,   DB_INT64,   DB_DOUBLE,  DB_DOUBLE,  DB_UNKNOWN }, 
        /* int64 */     { DB_UNKNOWN, DB_INT64,   DB_INT64,   DB_INT64,   DB_INT64,   DB_DOUBLE,  DB_DOUBLE,  DB_UNKNOWN }, 
        /* float */     { DB_UNKNOWN, DB_DOUBLE,  DB_DOUBLE,  DB_DOUBLE,  DB_DOUBLE,  DB_DOUBLE,  DB_DOUBLE,  DB_UNKNOWN },
        /* double */    { DB_UNKNOWN, DB_DOUBLE,  DB_DOUBLE,  DB_DOUBLE,  DB_DOUBLE,  DB_DOUBLE,  DB_DOUBLE,  DB_UNKNOWN },
        /* string */    { DB_UNKNOWN, DB_UNKNOWN, DB_UNKNOWN, DB_UNKNOWN, DB_UNKNOWN, DB_UNKNOWN, DB_UNKNOWN, DB_UNKNOWN },  
    };

    switch (op_type)
    {
    case TK_AND:
    case TK_OR:

    case TK_NE:
    case TK_EQ:
    case TK_GT:
    case TK_GE:
    case TK_LT:
    case TK_LE:
        return s_logic_matrix[left][right];

    case TK_PLUS:
    case TK_MINUS:
    case TK_STAR:
    case TK_SLASH:
        return s_arith_matrix[left][right];

    default:
        return DB_UNKNOWN;
    }
}


template<typename T>
expr_base_t* expr_factory_t::create_convert_expr_impl(data_type_t rt_type, expr_base_t* children)
{
    switch (rt_type)
    {
    case DB_INT16:
        return new expr_convert_t<T, db_int16>(children);
    case DB_INT32:
        return new expr_convert_t<T, db_int32>(children);
    case DB_INT64:
        return new expr_convert_t<T, db_int64>(children);
    case DB_FLOAT:
        return new expr_convert_t<T, db_float>(children);
    case DB_DOUBLE:
        return new expr_convert_t<T, db_double>(children);
    
    default:
        return NULL;
    }
}

expr_base_t* expr_factory_t::create_cast(data_type_t rt_type, expr_base_t* children)
{    
    data_type_t type = children->data_type();
    switch (type)
    {    
    case DB_INT8:
        return create_convert_expr_impl<db_int8>(rt_type, children);
    case DB_INT16:
        return create_convert_expr_impl<db_int16>(rt_type, children);
    case DB_INT32:
        return create_convert_expr_impl<db_int32>(rt_type, children);
    case DB_INT64:
        return create_convert_expr_impl<db_int64>(rt_type, children);
    case DB_FLOAT:
        return create_convert_expr_impl<db_float>(rt_type, children);

    default:
        return NULL;
    }
}




result_t expr_factory_t::build_normal(Expr* expr, db_uint32 index, expr_base_t** root)
{    
    result_t ret;

    if (expr == NULL) {
        *root = NULL;
        return RT_SUCCEEDED;
    }

    expr_base_t* left = NULL;
    if (expr->pLeft != NULL) {
        ret = build_normal(expr->pLeft, index, &left);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);
    }

    expr_base_t* right = NULL;
    if (expr->pRight != NULL) {
        ret = build_normal(expr->pRight, index, &right);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);
    }

    if (left != NULL && right != NULL) {
        data_type_t left_type = left->data_type();
        data_type_t right_type = right->data_type();

        data_type_t ret_type = cast_type(expr->op, left_type, right_type);
        IF_RETURN_FAILED(ret_type == DB_UNKNOWN);

        if (left_type != ret_type) {
            expr_base_t* temp = create_cast(ret_type, left);
            IF_RETURN_FAILED(temp == NULL);
            left = temp;
        }

        if (right_type != ret_type) {
            expr_base_t* temp = create_cast(ret_type, right);            
            IF_RETURN_FAILED(temp == NULL);
            right = temp;
        }
    }

    expr_base_t* curr = create_instance(expr, left, right);
    IF_RETURN_FAILED(curr == NULL);

    ret = curr->init(expr, NULL);
    IF_RETURN_FAILED(ret != RT_SUCCEEDED);

    *root = curr;
    return RT_SUCCEEDED;
}


result_t expr_factory_t::build_aggr(Expr* expr, db_uint32 index, expr_base_t** root)
{    
    result_t ret;

    if (expr->op == TK_AGG_COLUMN || expr->op == TK_AGG_FUNCTION) {
        expr_aggr_t* aggr_expr = (expr->op == TK_AGG_COLUMN) ?
            create_aggr_column(expr) : create_aggr_function(expr, index);
        
        row_segement_t::expr_item_t& item = m_stmt->m_aggr_table_def->m_columns[index];
        aggr_expr->m_offset = item.offset;
        *root = aggr_expr;                
        return aggr_expr->init(expr, NULL);
    }

    expr_base_t* left = NULL;
    if (expr->pLeft != NULL) {
        ret = build_aggr(expr->pLeft, index, &left);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);
    }

    expr_base_t* right = NULL;
    if (expr->pRight != NULL) {
        ret = build_aggr(expr->pRight, index, &right);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);
    }

    if (left != NULL && right != NULL) {
        data_type_t left_type = left->data_type();
        data_type_t right_type = right->data_type();

        data_type_t ret_type = cast_type(expr->op, left_type, right_type);
        IF_RETURN_FAILED(ret_type == DB_UNKNOWN);

        if (left_type != ret_type) {
            expr_base_t* temp = create_cast(ret_type, left);
            IF_RETURN_FAILED(temp == NULL);
            left = temp;
        }

        if (right_type != ret_type) {
            expr_base_t* temp = create_cast(ret_type, right);
            IF_RETURN_FAILED(temp == NULL);
            right = temp;
        }
    }

    expr_base_t* curr = create_instance(expr, left, right);
    IF_RETURN_FAILED(curr == NULL);

    ret = curr->init(expr, NULL);
    IF_RETURN_FAILED(ret != RT_SUCCEEDED);

    *root = curr;
    return RT_SUCCEEDED;
}


result_t expr_factory_t::build_list(ExprList* src, expr_list_t* dst)
{        
    for (db_int32 i = 0; i < src->nExpr; i++) {
        expr_base_t* expr = NULL;
        result_t ret = build_impl(src->a[i].pExpr, i, &expr);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);
        db_bool is_succ = dst->push_back(expr);
        IF_RETURN_FAILED(!is_succ);
    }

    return RT_SUCCEEDED;
}

