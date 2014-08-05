// expression.cpp by scott.zgeng@gmail.com 2014.08.5


#include <assert.h>
#include "expression.h"


int32 data_type_size(data_type_t type)
{
    switch (type)
    {
    case TYPE_INT8:
        return sizeof(int8);
    case TYPE_INT16:
        return sizeof(int16);
    case TYPE_INT32:
        return sizeof(int32);
    case TYPE_INT64:
        return sizeof(int64);
    case TYPE_FLOAT:
        return sizeof(float);
    case TYPE_DOUBLE:
        return sizeof(double);
    case TYPE_STRING:
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

const stack_segment_t query_pack_t::alloc_segment(data_type_t type, bool has_null)
{
    int32 size = (data_type_size(type) + (int32)has_null)* SEGMENT_SIZE;

    if (m_offset + size > sizeof(m_buffer))
        return stack_segment_t::NULL_SEGMENT;

    void* ptr = m_buffer + m_offset;
    m_offset += size;

    return stack_segment_t(this, ptr, type, has_null);
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
const stack_segment_t stack_segment_t::NULL_SEGMENT = { NULL, NULL, TYPE_UNKNOWN, false };


//-----------------------------------------------------------------------------
// expr_base_t
//-----------------------------------------------------------------------------
result_t expr_base_t::build(Expr* expr, expr_base_t** root)
{
    assert(expr);
    result_t ret;

    expr_base_t* expr_node = create_instance(expr);
    IF_RETURN_FAILED(expr_node == NULL);

    ret = expr_node->init(expr);
    IF_RETURN_FAILED(expr_node == NULL);

    if (expr->pLeft != NULL) {
        ret = build(expr->pLeft, &expr_node->m_left);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);
    }

    if (expr->pRight != NULL) {
        ret = build(expr->pRight, &expr_node->m_right);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);
    }

    *root = expr_node;
    return RT_SUCCEEDED;
}


expr_base_t* expr_base_t::create_instance(Expr* expr)
{
    switch (expr->op)
    {
    case TK_COLUMN:
        return new expr_column_t();
    case TK_INTEGER:
        return new expr_integer_t();
    case TK_GT:
        return new expr_gt_t();
    case TK_PLUS:
        return new expr_plus_t();
    case TK_STAR:
        return new expr_multiple_t();

    default:
        return NULL;
    }
}


//-----------------------------------------------------------------------------
// expr_gt_t
//-----------------------------------------------------------------------------
expr_gt_t::expr_gt_t()
{
}

expr_gt_t::~expr_gt_t()
{
}

result_t expr_gt_t::init(Expr* expr)
{
    return RT_SUCCEEDED;
}

int8 s_zero_null_segment[query_pack_t::SEGMENT_SIZE] = { 0, };


result_t expr_gt_t::execute(query_pack_t* pack, const stack_segment_t& result,
    const stack_segment_t& left, const stack_segment_t& right)
{
    int8* left_null = s_zero_null_segment;
    if (left.has_null()) {
        left_null = (int8*)left.ptr();
    }

    // 获取两个运算符中数据类型较大的类型

    switch (left.data_type())
    {
    default:
        break;
    }

    return RT_SUCCEEDED;
}

data_type_t expr_gt_t::data_type()
{
    // TODO(scott.zgeng): 后续可以优化成BITMAP
    return TYPE_INT8;
}


//-----------------------------------------------------------------------------
// expr_plus_t
//-----------------------------------------------------------------------------
expr_plus_t::expr_plus_t()
{
}

expr_plus_t::~expr_plus_t()
{
}

result_t expr_plus_t::init(Expr* expr)
{
    return RT_SUCCEEDED;
}

result_t expr_plus_t::execute(query_pack_t* pack, const stack_segment_t& result,
    const stack_segment_t& left, const stack_segment_t& right)
{
    return RT_SUCCEEDED;
}


data_type_t expr_plus_t::data_type()
{
    return TYPE_UNKNOWN;
}

//-----------------------------------------------------------------------------
// expr_multiple_t
//-----------------------------------------------------------------------------
expr_multiple_t::expr_multiple_t()
{
}

expr_multiple_t::~expr_multiple_t()
{
}

result_t expr_multiple_t::init(Expr* expr)
{
    return RT_SUCCEEDED;
}

result_t expr_multiple_t::execute(query_pack_t* pack, const stack_segment_t& result,
    const stack_segment_t& left, const stack_segment_t& right)
{
    return RT_SUCCEEDED;
}

data_type_t expr_multiple_t::data_type()
{
    return TYPE_UNKNOWN;
}

//-----------------------------------------------------------------------------
// expr_column_t
//-----------------------------------------------------------------------------
expr_column_t::expr_column_t()
{
}

expr_column_t::~expr_column_t()
{
}

result_t expr_column_t::init(Expr* expr)
{
    m_table_id = expr->iTable;

    return RT_SUCCEEDED;
}

result_t expr_column_t::execute(query_pack_t* pack, const stack_segment_t& result)
{
    return RT_SUCCEEDED;
}

data_type_t expr_column_t::data_type()
{
    return TYPE_UNKNOWN;
}
//-----------------------------------------------------------------------------
// expr_integer_t
//-----------------------------------------------------------------------------
expr_integer_t::expr_integer_t()
{
}

expr_integer_t::~expr_integer_t()
{
}

result_t expr_integer_t::init(Expr* expr)
{
    return RT_SUCCEEDED;
}

result_t expr_integer_t::execute(query_pack_t* pack, const stack_segment_t& result)
{
    return RT_SUCCEEDED;
}

data_type_t expr_integer_t::data_type()
{
    return TYPE_INT32;
}
