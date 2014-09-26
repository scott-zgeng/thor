// expression.cpp by scott.zgeng@gmail.com 2014.08.5


#include <assert.h>
#include <limits.h>
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
    m_expr_idx = 0;
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
    db_uint32 index = m_expr_idx;
    row_segement_t::expr_item_t& item = m_stmt->m_aggr_table_def->m_columns[index];  
    data_type_t type = item.expr->data_type();

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



result_t expr_factory_t::adjust_binary_sub_type(data_type_t dst_type, expr_base_t*& left, expr_base_t*& right)
{
    data_type_t left_type = left->data_type();
    data_type_t right_type = right->data_type();

    if (left_type != dst_type) {
        expr_base_t* temp = create_cast(dst_type, left);
        IF_RETURN_FAILED(temp == NULL);
        left = temp;
    }

    if (right_type != dst_type) {
        expr_base_t* temp = create_cast(dst_type, right);
        IF_RETURN_FAILED(temp == NULL);
        right = temp;
    }

    assert(left->data_type() == right->data_type());
    return RT_SUCCEEDED;
}


template<int OP_TYPE>
expr_base_t* expr_factory_t::create_binary_logic(expr_base_t* left, expr_base_t* right)
{
    assert(left != NULL && right != NULL);
    
    data_type_t type = cast_type(OP_TYPE, left->data_type(), right->data_type());
    if (type == DB_UNKNOWN) return NULL;

    result_t ret = adjust_binary_sub_type(type, left, right);
    if (ret != RT_SUCCEEDED) return NULL;

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
expr_base_t* expr_factory_t::create_binary_arith(expr_base_t* left, expr_base_t* right)
{
    assert(left != NULL && right != NULL);

    data_type_t type = cast_type(OP_TYPE, left->data_type(), right->data_type());
    if (type == DB_UNKNOWN) return NULL;

    result_t ret = adjust_binary_sub_type(type, left, right);
    if (ret != RT_SUCCEEDED) return NULL;

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


expr_base_t* expr_factory_t::create_instance(Expr* expr, expr_base_t* left, expr_base_t* right)
{
    switch (expr->op)
    {
    case TK_AGG_COLUMN:
    case TK_COLUMN:
        return create_column(expr);
    case TK_INTEGER:
        return create_integer(expr); 
    case TK_FLOAT:
        return new expr_float_t();
    case TK_STRING:
        return new expr_string_t();

    case TK_AND:
        return create_binary_logic<TK_AND>(left, right);
    case TK_OR:
        return create_binary_logic<TK_OR>(left, right);
    case TK_NE:
        return create_binary_logic<TK_NE>(left, right);
    case TK_EQ:
        return create_binary_logic<TK_EQ>(left, right);
    case TK_GT:
        return create_binary_logic<TK_GT>(left, right);
    case TK_GE:
        return create_binary_logic<TK_GE>(left, right);
    case TK_LT:
        return create_binary_logic<TK_LT>(left, right);
    case TK_LE:
        return create_binary_logic<TK_LE>(left, right);

    case TK_PLUS:
        return create_binary_arith<TK_PLUS>(left, right);
    case TK_MINUS:
        return create_binary_arith<TK_MINUS>(left, right);
    case TK_STAR:
        return create_binary_arith<TK_STAR>(left, right);
    case TK_SLASH:
        return create_binary_arith<TK_SLASH>(left, right);

    default:
        return NULL;
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

expr_base_t* expr_factory_t::create_dummy()
{
    return new expr_dummy_t();
}


result_t expr_factory_t::build_normal(Expr* expr, expr_base_t** root)
{    
    result_t ret;

    if (expr == NULL) {
        *root = NULL;
        return RT_SUCCEEDED;
    }

    expr_base_t* left = NULL;
    if (expr->pLeft != NULL) {
        ret = build_normal(expr->pLeft, &left);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);
    }

    expr_base_t* right = NULL;
    if (expr->pRight != NULL) {
        ret = build_normal(expr->pRight, &right);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);
    }

    expr_base_t* curr = create_instance(expr, left, right);
    IF_RETURN_FAILED(curr == NULL);

    ret = curr->init(expr, NULL);
    IF_RETURN_FAILED(ret != RT_SUCCEEDED);

    *root = curr;
    return RT_SUCCEEDED;
}


result_t expr_factory_t::build_aggr(Expr* expr, expr_base_t** root)
{    
    result_t ret;

    if (expr->op == TK_AGG_COLUMN || expr->op == TK_AGG_FUNCTION) {
        aggr_type_t aggr_type = get_aggr_type(expr->u.zToken);

        expr_base_t* curr_expr;
        expr_aggr_t* aggr_expr = create_aggr_column(expr);
        row_segement_t::expr_item_t& item = m_stmt->m_aggr_table_def->m_columns[m_expr_idx];
        aggr_expr->m_offset = item.offset;
        m_expr_idx++;
        curr_expr = aggr_expr;

        if (aggr_type == AGGR_FUNC_AVG) {
            expr_aggr_t* aggr_expr2 = create_aggr_column(expr);
            row_segement_t::expr_item_t& item = m_stmt->m_aggr_table_def->m_columns[m_expr_idx];
            aggr_expr2->m_offset = item.offset;
            m_expr_idx++;

            curr_expr = create_binary_arith<TK_SLASH>(aggr_expr, aggr_expr2);
        }

        *root = curr_expr;
        return aggr_expr->init(expr, NULL);
    }

    expr_base_t* left = NULL;
    if (expr->pLeft != NULL) {
        ret = build_aggr(expr->pLeft, &left);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);
    }

    expr_base_t* right = NULL;
    if (expr->pRight != NULL) {
        ret = build_aggr(expr->pRight, &right);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);
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
        result_t ret = build_impl(src->a[i].pExpr, &expr);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);
        db_bool is_succ = dst->push_back(expr);
        IF_RETURN_FAILED(!is_succ);
    }

    return RT_SUCCEEDED;
}



