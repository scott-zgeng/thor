// expression.cpp by scott.zgeng@gmail.com 2014.08.5


#include <assert.h>
#include "expression.h"


int32 data_type_size(data_type_t type)
{
    switch (type)
    {
    case DB_INT8:
        return sizeof(int8);
    case DB_INT16:
        return sizeof(int16);
    case DB_INT32:
        return sizeof(int32);
    case DB_INT64:
        return sizeof(int64);
    case DB_FLOAT:
        return sizeof(float);
    case DB_DOUBLE:
        return sizeof(double);
    case DB_STRING:
        return sizeof(char*);
    default:
        return (-1);
    }
}


//-----------------------------------------------------------------------------
// query_pack_t
//-----------------------------------------------------------------------------
query_pack_t::query_pack_t()
{
    m_offset = 0;
}

query_pack_t::~query_pack_t()
{

}


result_t query_pack_t::generate_data(int32 table_id, int32 column_id)
{
    return RT_SUCCEEDED;
}

const stack_segment_t query_pack_t::alloc_segment(expr_base_t* expr)
{
    int32 size = (data_type_size(expr->data_type()) + expr->has_null()) * SEGMENT_SIZE;

    if (m_offset + size > sizeof(m_buffer))
        return stack_segment_t::NULL_SEGMENT;

    void* ptr = m_buffer + m_offset;
    m_offset += size;

    return stack_segment_t(this, ptr);
}

void query_pack_t::free_segment(const stack_segment_t& frame)
{
    int8* ptr = (int8*)frame.ptr();
    assert(ptr >= m_buffer && ptr < m_buffer + sizeof(m_buffer));
    m_offset = m_buffer - ptr;
}


//-----------------------------------------------------------------------------
// stack_frame_t
//-----------------------------------------------------------------------------
const stack_segment_t stack_segment_t::NULL_SEGMENT = { NULL, NULL};


//-----------------------------------------------------------------------------
// expr_base_t
//-----------------------------------------------------------------------------


data_type_t get_table_column_type(int32 table_id, int32 column_id)
{
    return DB_INT32;
}



static expr_base_t* create_expression_column(Expr* expr)
{
    data_type_t type = get_table_column_type(0, 0);

    switch (type)        
    {
    case DB_INT8:
        return new expr_column_t<DB_INT8>();
    case DB_INT16:
        return new expr_column_t<DB_INT16>();
    case DB_INT32:
        return new expr_column_t<DB_INT32>();
    case DB_INT64:
        return new expr_column_t<DB_INT64>();
    default:
        return NULL;
    }
}

static expr_base_t* create_expression_integer(Expr* expr)
{
    int64 value = expr->u.iValue;

    if (CHAR_MIN >= value && value <= CHAR_MAX)
        return new expr_integer_t<DB_INT8>;
    else if (SHRT_MIN >= value && value <= SHRT_MAX)
        return new expr_integer_t<DB_INT16>;
    else if (INT_MIN >= value && value <= INT_MAX)
        return new expr_integer_t<DB_INT32>;
    else
        return new expr_integer_t<DB_INT64>;
}


template<int OP_TYPE>
static expr_base_t* create_expression_logic_impl(data_type_t type, expr_base_t* left, expr_base_t* right)
{
    switch (type)
    {    
    case DB_INT8:
        return new binary_expr_t<OP_TYPE, DB_INT8, DB_INT8>(left, right);
    case DB_INT16:
        return new binary_expr_t<OP_TYPE, DB_INT16, DB_INT8>(left, right);
    case DB_INT32:
        return new binary_expr_t<OP_TYPE, DB_INT32, DB_INT8>(left, right);
    case DB_INT64:
        return new binary_expr_t<OP_TYPE, DB_INT64, DB_INT8>(left, right);
    case DB_FLOAT:
        return new binary_expr_t<OP_TYPE, DB_FLOAT, DB_INT8>(left, right);
    case DB_DOUBLE:
        return new binary_expr_t<OP_TYPE, DB_DOUBLE, DB_INT8>(left, right);
    case DB_STRING:
        return new binary_expr_t<OP_TYPE, DB_STRING, DB_INT8>(left, right);
    default:
        return NULL;
    }
}

template<int OP_TYPE>
static expr_base_t* create_expression_arith_impl(data_type_t type)
{
    switch (type)
    {
    case DB_INT64:
        return new binary_expr_t<OP_TYPE, DB_INT64, DB_INT64>();
    case DB_DOUBLE:
        return new binary_expr_t<OP_TYPE, DB_DOUBLE, DB_DOUBLE>();
    default:
        return NULL;
    }
}


static expr_base_t* create_expression_binary(Expr* expr, expr_base_t* left, expr_base_t* right)
{
    assert(left->data_type() == right->data_type());
    data_type_t type = left->data_type();

    switch (expr->op)
    {
    //case TK_NE:
    //    return create_expression_logic_impl<TK_NE>(type);
    //case TK_EQ:
    //    return create_expression_logic_impl<TK_EQ>(type);
    case TK_GT:
        return create_expression_logic_impl<TK_GT>(type, left, right);
    //case TK_GE:
    //    return create_expression_logic_impl<TK_GE>(type);
    //case TK_LT:
    //    return create_expression_logic_impl<TK_LT>(type);
    //case TK_LE:
    //    return create_expression_logic_impl<TK_LE>(type);

    case TK_PLUS:
    case TK_MINUS:
    case TK_STAR:
    case TK_SLASH:
        
    default:
        return NULL;
    }
}



expr_base_t* expr_base_t::create_instance(Expr* expr, expr_base_t* left, expr_base_t* right)
{
    switch (expr->op)
    {
    case TK_COLUMN:
        return create_expression_column(expr);
    case TK_INTEGER:
        return create_expression_integer(expr); 
    default:
        return create_expression_binary(expr, left, right);
    }
}




data_type_t convert_type(int op_type, data_type_t left, data_type_t right)
{
    // NOTE(scott.zgeng): 逻辑运算转为能覆盖两者范围的数据类型
    const static data_type_t s_logic_matrix[DB_TYPE_SIZE][DB_TYPE_SIZE] = {
        //unknown       int8            int16           int32           int64           float           double          string
        { DB_UNKNOWN, DB_UNKNOWN, DB_UNKNOWN, DB_UNKNOWN, DB_UNKNOWN, DB_UNKNOWN, DB_UNKNOWN, DB_UNKNOWN },   // unknown
        { DB_UNKNOWN, DB_INT8, DB_INT16, DB_INT32, DB_INT64, DB_FLOAT, DB_DOUBLE, DB_UNKNOWN },   // int8
        { DB_UNKNOWN, DB_INT16, DB_INT16, DB_INT32, DB_INT64, DB_FLOAT, DB_DOUBLE, DB_UNKNOWN },   // int16
        { DB_UNKNOWN, DB_INT32, DB_INT32, DB_INT32, DB_INT64, DB_DOUBLE, DB_DOUBLE, DB_UNKNOWN },   // int32
        { DB_UNKNOWN, DB_INT64, DB_INT64, DB_INT64, DB_INT64, DB_DOUBLE, DB_DOUBLE, DB_UNKNOWN },   // int64
        { DB_UNKNOWN, DB_FLOAT, DB_FLOAT, DB_DOUBLE, DB_DOUBLE, DB_FLOAT, DB_DOUBLE, DB_UNKNOWN },   // float
        { DB_UNKNOWN, DB_DOUBLE, DB_DOUBLE, DB_DOUBLE, DB_DOUBLE, DB_DOUBLE, DB_DOUBLE, DB_UNKNOWN },   // double
        { DB_UNKNOWN, DB_UNKNOWN, DB_UNKNOWN, DB_UNKNOWN, DB_UNKNOWN, DB_UNKNOWN, DB_UNKNOWN, DB_STRING },   // string
    };

    // NOTE(scott.zgeng): 算数运算全部转为最大的数据类型计算，整形转换为INT64, 浮点型转换为DOUBLE
    const static data_type_t s_arith_matrix[DB_TYPE_SIZE][DB_TYPE_SIZE] = {
        //unknown     int8       int16           int32           int64           float           double          string
        { DB_UNKNOWN, DB_UNKNOWN, DB_UNKNOWN, DB_UNKNOWN, DB_UNKNOWN, DB_UNKNOWN, DB_UNKNOWN, DB_UNKNOWN },   // unknown
        { DB_UNKNOWN, DB_INT64, DB_INT64, DB_INT64, DB_INT64, DB_DOUBLE, DB_DOUBLE, DB_UNKNOWN },   // int8
        { DB_UNKNOWN, DB_INT64, DB_INT64, DB_INT64, DB_INT64, DB_DOUBLE, DB_DOUBLE, DB_UNKNOWN },   // int16
        { DB_UNKNOWN, DB_INT64, DB_INT64, DB_INT64, DB_INT64, DB_DOUBLE, DB_DOUBLE, DB_UNKNOWN },   // int32
        { DB_UNKNOWN, DB_INT64, DB_INT64, DB_INT64, DB_INT64, DB_DOUBLE, DB_DOUBLE, DB_UNKNOWN },   // int64
        { DB_UNKNOWN, DB_DOUBLE, DB_DOUBLE, DB_DOUBLE, DB_DOUBLE, DB_DOUBLE, DB_DOUBLE, DB_UNKNOWN },   // float
        { DB_UNKNOWN, DB_DOUBLE, DB_DOUBLE, DB_DOUBLE, DB_DOUBLE, DB_DOUBLE, DB_DOUBLE, DB_UNKNOWN },   // double
        { DB_UNKNOWN, DB_UNKNOWN, DB_UNKNOWN, DB_UNKNOWN, DB_UNKNOWN, DB_UNKNOWN, DB_UNKNOWN, DB_UNKNOWN },   // string
    };

    switch (op_type)
    {
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


template<data_type_t TYPE>
expr_base_t* create_convert_expr_impl(data_type_t rt_type, expr_base_t* children)
{
    switch (rt_type)
    {
    case DB_INT16:
        return new expr_convert_t<TYPE, DB_INT16>(children);
    case DB_INT32:
        return new expr_convert_t<TYPE, DB_INT32>(children);
    case DB_INT64:
        return new expr_convert_t<TYPE, DB_INT64>(children);
    case DB_FLOAT:
        return new expr_convert_t<TYPE, DB_FLOAT>(children);
    case DB_DOUBLE:
        return new expr_convert_t<TYPE, DB_DOUBLE>(children);
    case DB_INT8:
    default:
        return NULL;
    }
}

expr_base_t* create_convert_expr(data_type_t type, data_type_t rt_type, expr_base_t* children)
{    
    switch (type)
    {    
    case DB_INT8:
        return create_convert_expr_impl<DB_INT8>(rt_type, children);
    case DB_INT16:
        return create_convert_expr_impl<DB_INT16>(rt_type, children);
    case DB_INT32:
        return create_convert_expr_impl<DB_INT32>(rt_type, children);        
    case DB_INT64:
        return create_convert_expr_impl<DB_DOUBLE>(rt_type, children);
    case DB_FLOAT:
        return create_convert_expr_impl<DB_FLOAT>(rt_type, children);
    
    case DB_DOUBLE:        
    case DB_STRING:
    default:
        return NULL;
    }
}



result_t expr_base_t::build(Expr* expr, expr_base_t** root)
{
    assert(expr);
    result_t ret;

    expr_base_t* left = NULL;
    if (expr->pLeft != NULL) {
        ret = build(expr->pLeft, &left);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);
    }

    expr_base_t* right = NULL;
    if (expr->pRight != NULL) {
        ret = build(expr->pRight, &right);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);
    }

    if (left != NULL && right != NULL) {
        data_type_t left_type = left->data_type();
        data_type_t right_type = right->data_type();

        data_type_t ret_type = convert_type(expr->op, left_type, right_type);
        IF_RETURN_FAILED(ret_type == DB_UNKNOWN);

        if (left_type != ret_type) {
            expr_base_t* temp = create_convert_expr(left_type, ret_type, left);
            IF_RETURN_FAILED(temp == NULL);
            left = temp;
        }

        if (right_type != ret_type) {
            expr_base_t* temp = create_convert_expr(right_type, ret_type, right);            
            IF_RETURN_FAILED(temp == NULL);
            right = temp;
        }
    }

    expr_base_t* curr = create_instance(expr, left, right);
    IF_RETURN_FAILED(curr == NULL);

    ret = curr->init(expr);
    IF_RETURN_FAILED(curr == NULL);

    *root = curr;
    return RT_SUCCEEDED;
}




//-----------------------------------------------------------------------------
// expr_column_t
//-----------------------------------------------------------------------------

template<data_type_t TYPE>
expr_column_t<TYPE>::expr_column_t()
{
    m_table_id = 0;
    m_column_id = 0;
    m_seed.v = 0;
}
    
template<data_type_t TYPE>
expr_column_t<TYPE>::~expr_column_t()
{
}

template<data_type_t TYPE>
result_t expr_column_t<TYPE>::init(Expr* expr)
{
    return RT_SUCCEEDED;
}
 
template<data_type_t TYPE>
result_t expr_column_t<TYPE>::calc(query_pack_t* pack, const stack_segment_t& result)
{
    variant<TYPE>* out = (variant<TYPE>*)result.ptr();

    // TODO(scott.zgeng): 以下代码用来测试
    for (int32 i = 0; i < SEGMENT_SIZE; i++) {
        out[i].v = m_seed.v;
        m_seed.v++;
    }

    return RT_SUCCEEDED;
}


//-----------------------------------------------------------------------------
// expr_integer_t
//-----------------------------------------------------------------------------

template<data_type_t TYPE>
result_t expr_integer_t<TYPE>::init(Expr* expr)
{
    variant_cast<DB_INT64, TYPE> cast;
    m_value.v = cast(expr->u.iValue);
    
    return RT_SUCCEEDED;
}

template<data_type_t TYPE>
result_t expr_integer_t<TYPE>::calc(query_pack_t* pack, const stack_segment_t& result)
{
    variant<TYPE>* out = (variant<TYPE>*)result.ptr();

    // TODO(scott.zgeng): 以下代码用来测试
    for (int32 i = 0; i < SEGMENT_SIZE; i++) {
        out[i] = m_value;
    }

    return RT_SUCCEEDED;
}

