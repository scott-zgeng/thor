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

    expr_base_t* curr = create_instance(expr, left, right);
    IF_RETURN_FAILED(curr == NULL);

    curr->m_left = left;
    curr->m_right = right;

    ret = curr->init(expr);
    IF_RETURN_FAILED(curr == NULL);

    *root = curr;
    return RT_SUCCEEDED;
}


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
    if (CHAR_MIN > value && value <= CHAR_MAX)
        return new expr_integer_t<DB_INT8>;
    else if (SHRT_MIN > value && value <= SHRT_MAX)
        return new expr_integer_t<DB_INT16>;
    else if (INT_MIN > value && value <= INT_MAX)
        return new expr_integer_t<DB_INT32>;
    else
        return new expr_integer_t<DB_INT64>;
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
        return NULL;
    }
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
    variant_convertor<TYPE, DB_INT64> conv;
    m_value.v = conv(expr->u.iValue);
    
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

