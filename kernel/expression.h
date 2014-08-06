// expression.h by scott.zgeng@gmail.com 2014.08.5

#ifndef  __EXPRESSION__
#define  __EXPRESSION__

#include "define.h"



extern "C" {
#include "../sql/sqliteInt.h"
#include "../sql/vdbeInt.h"
}



int32 data_type_size(data_type_t type);

static const int32 SEGMENT_SIZE = 1024;

// NOTE(scott.zgeng): 根据数据类型定义元的数据类型
template<data_type_t TYPE> 
struct variant {};


template<> struct variant<DB_INT8>     { int8 v; };
template<> struct variant<DB_INT16>     { int16 v; };
template<> struct variant<DB_INT32>     { int32 v; };
template<> struct variant<DB_INT64>     { int64 v; };
template<> struct variant<DB_FLOAT>     { float v; };
template<> struct variant<DB_DOUBLE>    { double v; };
template<> struct variant<DB_STRING>    { char* v; };


template<data_type_t TYPE, data_type_t RT_TYPE>
struct variant_convertor {};


template<>
struct variant_convertor<DB_INT8, DB_INT64>
{
    int8 operator() (int64 v) { return (int8)v; }
};

template<> 
struct variant_convertor<DB_INT16, DB_INT64>
{
    int16 operator() (int64 v) { return (int16)v; }
};

template<>
struct variant_convertor<DB_INT32, DB_INT64>
{
    int32 operator() (int64 v) { return (int32)v; }
};

template<>
struct variant_convertor<DB_INT64, DB_INT64>
{
    int64 operator() (int64 v) { return v; }
};


class expr_base_t;
class stack_segment_t;
class query_pack_t
{
public:
    query_pack_t();
    ~query_pack_t();

public:
    result_t generate_data(int32 table_id, int32 column_id);
    const stack_segment_t alloc_segment(expr_base_t* expr);
    void free_segment(const stack_segment_t& segment);

private:
    int8 m_buffer[SEGMENT_SIZE * 1024 * 2];
    int32 m_offset;
};


class stack_segment_t
{
public:
    static const stack_segment_t NULL_SEGMENT;

public:
    stack_segment_t(query_pack_t* pack, void* data) {
        DB_TRACE("enter stack segment, %p", data);

        m_pack = pack;
        m_data = data;
    }

    ~stack_segment_t() {
        DB_TRACE("free stack segment, %p", m_data);

        if (m_data != NULL) {
            m_pack->free_segment(*this);
        }
    }

    void* ptr() const { return m_data; }

private:
    query_pack_t* m_pack;
    void* m_data;
};




class expr_base_t
{
public:
    static result_t build(Expr* expr, expr_base_t** root);

public:
    expr_base_t() { m_left = NULL; m_right = NULL; }
    virtual ~expr_base_t() {}

    virtual result_t init(Expr* expr) { return RT_SUCCEEDED; }
    virtual bool has_null() { return false; }

    virtual result_t calc(query_pack_t* pack, const stack_segment_t& result) = 0;
    virtual data_type_t data_type() = 0;


    expr_base_t* m_left;
    expr_base_t* m_right;

private:
    static expr_base_t* create_instance(Expr* expr, expr_base_t* left, expr_base_t* right);
};




// NOTE(scott.zgeng): 没有使用操作符重载，觉得没太大必要
template<int OP_TYPE, data_type_t TYPE, data_type_t RT_TYPE>
struct unary_operator
{
    result_t execute(variant<RT_TYPE>* dst, variant<TYPE>* src);
};


template<int OP_TYPE, data_type_t TYPE, data_type_t RT_TYPE>
class unary_expr_t : public expr_base_t
{
public:
    typename variant<TYPE> src_type;
    typename variant<RT_TYPE> dst_type;

public:
    virtual ~unary_expr_t() {}

    virtual data_type_t data_type() { return RT_TYPE; }

    virtual result_t calc(query_pack_t* pack, const stack_segment_t& result) {
        assert(m_left);

        const stack_segment_t& left_segment = pack->alloc_segment(m_left);
        IF_RETURN_FAILED(left_segment == NULL_SEGMENT);

        result_t rt = m_left->calc(pack, left_segment);
        IF_RETURN_FAILED(rt != RT_SUCCEEDED);

        dst_type* dst = (dst_type*)result.ptr();
        src_type* src = (src_type*)left_segment.ptr();

        unary_operator<OP_TYPE, TYPE, RT_TYPE> op;
        return op.execute(dst, src);
    }
};



// NOTE(scott.zgeng): 没有使用操作符重载，觉得没太大必要
template<int OP_TYPE, data_type_t TYPE, data_type_t RT_TYPE>
struct binary_operator
{
    result_t execute(variant<RT_TYPE>* dst, variant<TYPE>* src1, variant<TYPE>* src2);
};


template<int OP_TYPE, data_type_t TYPE, data_type_t RT_TYPE>
class binary_expr_t : public expr_base_t
{
public:
    typename variant<TYPE> src_type;
    typename variant<RT_TYPE> dst_type;

public:
    virtual ~binary_expr_t() {}

    virtual data_type_t data_type() { return RT_TYPE; }

    virtual result_t calc(query_pack_t* pack, const stack_segment_t& result) {
        assert(m_left);
        assert(m_right);

        result_t ret;
        const stack_segment_t& left_segment = pack->alloc_segment(m_left);
        ret = m_left->calc(pack, left_segment);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);

        const stack_segment_t& right_segment = pack->alloc_segment(m_right);
        ret = m_right->calc(pack, right_segment);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);

        dst_type* dst = (dst_type*)result.ptr();
        src_type* src1 = (src_type*)left_segment.ptr();
        src_type* src2 = (src_type*)right_segment.ptr();

        binary_operator<OP_TYPE, TYPE, RT_TYPE> op;
        return op.execute(dst, src1, src2);
    }
};


// NOTE(scott.zgeng): 目前只支持从小的数据类型往大的数据类型转换
template<data_type_t TYPE, data_type_t RT_TYPE>
class expr_convert_t : public expr_base_t
{
public:
    typename variant<TYPE> src_type;
    typename variant<RT_TYPE> dst_type;

public:
    virtual ~expr_convert_t() {}

    virtual data_type_t data_type() { return RT_TYPE; }

    virtual result_t calc(query_pack_t* pack, const stack_segment_t& result) {
        assert(sizeof(variant<RT_TYPE>) >= sizeof(variant<TYPE>));

        result_t ret = m_left->calc(pack, result);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);

        int8* ptr = (int8*)result.ptr();
        dst_type* dst = (dst_type*)ptr;
        src_type* src = (src_type*)ptr;

        // TODO(scott.zgeng): 如果这里存在性能问题，可以后续改成SIMD方式
        // 因为公用数据区，所以升级数据类型是从后往前的
        for (int32 i = SEGMENT_SIZE - 1; i >= 0; i--) {
            dst[i].v = src[i].v;
        }

        return RT_SUCCEEDED;
    }

};



template<data_type_t TYPE>
class expr_column_t : public expr_base_t
{
public:
    expr_column_t();
    virtual ~expr_column_t();

public:
    virtual result_t init(Expr* expr);
    virtual result_t calc(query_pack_t* pack, const stack_segment_t& result);

    virtual data_type_t data_type() { return TYPE; }

private:
    int32 m_table_id;
    int32 m_column_id;

    variant<TYPE> m_seed;
};


template<data_type_t TYPE>
class expr_integer_t : public expr_base_t
{
public:
    expr_integer_t() { m_value.v = 0; }
    virtual ~expr_integer_t() {}

public:
    virtual result_t init(Expr* expr);
    virtual result_t calc(query_pack_t* pack, const stack_segment_t& result);

    virtual data_type_t data_type() { return TYPE; }

private:
    variant<TYPE> m_value;   
};




#endif //__EXPRESSION__


