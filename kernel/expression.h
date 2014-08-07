// expression.h by scott.zgeng@gmail.com 2014.08.5

#ifndef  __EXPRESSION__
#define  __EXPRESSION__

#include "define.h"
#include "variant.h"



extern "C" {
#include "../sql/sqliteInt.h"
#include "../sql/vdbeInt.h"
}



db_int32 data_type_size(data_type_t type);

static const db_int32 SEGMENT_SIZE = 1024;


class expr_base_t;
class stack_segment_t;
class query_pack_t
{
public:
    query_pack_t();
    ~query_pack_t();

public:
    result_t generate_data(db_int32 table_id, db_int32 column_id);
    const stack_segment_t alloc_segment(expr_base_t* expr);
    void free_segment(const stack_segment_t& segment);

private:
    db_int8 m_buffer[SEGMENT_SIZE * 1024 * 2];
    db_int32 m_offset;
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
    virtual ~expr_base_t() {}

    virtual result_t init(Expr* expr) { return RT_SUCCEEDED; }
    virtual bool has_null() { return false; }

    virtual result_t calc(query_pack_t* pack, const stack_segment_t& result) = 0;
    virtual data_type_t data_type() = 0;


private:
    static expr_base_t* create_instance(Expr* expr, expr_base_t* left, expr_base_t* right);
};




// NOTE(scott.zgeng): 没有使用操作符重载，觉得没太大必要
template<int OP_TYPE, typename T, typename RT>
struct unary_operator
{
    result_t execute(T* a, RT* out);
};


template<int OP_TYPE, typename T, typename RT>
class unary_expr_t : public expr_base_t
{
public:
    unary_expr_t(expr_base_t* children) { m_children = children; }
    virtual ~unary_expr_t() {}

    virtual data_type_t data_type() { variant_type<RT> type; return type(); }

    virtual result_t calc(query_pack_t* pack, const stack_segment_t& result) {
        assert(m_left);

        const stack_segment_t& left_segment = pack->alloc_segment(m_left);
        IF_RETURN_FAILED(left_segment == NULL_SEGMENT);

        result_t rt = m_left->calc(pack, left_segment);
        IF_RETURN_FAILED(rt != RT_SUCCEEDED);

        RT* dst = (RT*)result.ptr();
        T* src = (T*)left_segment.ptr();

        unary_operator<OP_TYPE, T, RT> op;
        return op.execute(src, dst);
    }

private:
    expr_base_t* m_children;
};


template<int OP_TYPE, typename T, typename RT>
struct binary_primitive { RT operator() (T a, T b);};


#define BINARY_PRIMITIVE(OP_TYPE, SYMBOL)  \
    template<typename T, typename RT> struct binary_primitive<OP_TYPE, T, RT> { RT operator() (T a, T b) { return a SYMBOL b; } }


BINARY_PRIMITIVE(TK_EQ, ==);
BINARY_PRIMITIVE(TK_NE, !=);
BINARY_PRIMITIVE(TK_GT, >);
BINARY_PRIMITIVE(TK_GE, >=);
BINARY_PRIMITIVE(TK_LT, <);
BINARY_PRIMITIVE(TK_LE, <=);

BINARY_PRIMITIVE(TK_ADD, +);
BINARY_PRIMITIVE(TK_MINUS, -);
BINARY_PRIMITIVE(TK_STAR, *);
BINARY_PRIMITIVE(TK_SLASH, /);



// NOTE(scott.zgeng): 没有使用操作符重载，觉得没太大必要
template<int OP_TYPE, typename T, typename RT>
struct binary_operator
{
    result_t execute(T* a, T* b, RT* out) {
        binary_primitive<OP_TYPE, T, RT> binary_prim;
        for (int i = 0; i < SEGMENT_SIZE; i++) {
            out[i] = binary_prim(a[i], b[i]);
        }
        return RT_SUCCEEDED;
    }
};


template<int OP_TYPE, typename T, typename RT>
class binary_expr_t : public expr_base_t
{
public:
    binary_expr_t(expr_base_t* left, expr_base_t* right) {
        m_left = left;
        m_right = right;
    }

    virtual ~binary_expr_t() {}

    virtual data_type_t data_type() { variant_type<RT> type; return type(); }

    virtual result_t calc(query_pack_t* pack, const stack_segment_t& result) {
        assert(m_left);
        assert(m_right);

        result_t rt;
        const stack_segment_t& left_segment = pack->alloc_segment(m_left);
        rt = m_left->calc(pack, left_segment);
        IF_RETURN_FAILED(rt != RT_SUCCEEDED);

        const stack_segment_t& right_segment = pack->alloc_segment(m_right);
        rt = m_right->calc(pack, right_segment);
        IF_RETURN_FAILED(rt != RT_SUCCEEDED);

        RT* dst = (RT*)result.ptr();        
        T* src1 = (T*)left_segment.ptr();
        T* src2 = (T*)right_segment.ptr();

        binary_operator<OP_TYPE, T, RT> op;
        return op.execute(src1, src2, dst);
    }

private:
    expr_base_t* m_left;
    expr_base_t* m_right;
};


// NOTE(scott.zgeng): 目前只支持从小的数据类型往大的数据类型转换
template<typename T, typename RT>
class expr_convert_t : public expr_base_t
{
public:
    expr_convert_t(expr_base_t* children) { m_children = children; }
    virtual ~expr_convert_t() {}

    virtual result_t init(Expr* expr) { return m_children->init(expr); }
    virtual bool has_null() { return m_children->has_null(); }

    virtual data_type_t data_type() { variant_type<RT> type; return type(); }

    virtual result_t calc(query_pack_t* pack, const stack_segment_t& result) {
        assert(sizeof(T) <= sizeof(RT>));

        result_t ret = m_children->calc(pack, result);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);

        RT* dst = (RT*)result.ptr();
        T* src = (T*)result.ptr();

        // TODO(scott.zgeng): 如果这里存在性能问题，可以后续改成SIMD方式
        // 因为公用数据区，所以升级数据类型是从后往前的
        variant_cast<T, RT> upcast;
        for (db_int32 i = SEGMENT_SIZE - 1; i >= 0; i--) {            
            dst[i] = upcast(src[i]);
        }

        return RT_SUCCEEDED;
    }

private:
    expr_base_t* m_children;
};


template<typename T>
class expr_column_t : public expr_base_t
{
public:
    expr_column_t() {
        m_table_id = 0;
        m_column_id = 0;
        m_seed = 0;
    }

    virtual ~expr_column_t() {}

public:
    virtual result_t init(Expr* expr) {
        return RT_SUCCEEDED;
    }

    virtual result_t calc(query_pack_t* pack, const stack_segment_t& result) {
        T* out = (T*)result.ptr();

        // TODO(scott.zgeng): 以下代码用来测试
        for (db_int32 i = 0; i < SEGMENT_SIZE; i++) {
            out[i] = m_seed;
            m_seed++;
        }

        return RT_SUCCEEDED;
    }

    virtual data_type_t data_type() { variant_type<T> type; return type(); }

private:
    db_int32 m_table_id;
    db_int32 m_column_id;

    T m_seed;
};


template<typename T>
class expr_integer_t : public expr_base_t
{
public:
    expr_integer_t() { m_value = 0; }
    virtual ~expr_integer_t() {}

public:
    virtual result_t init(Expr* expr) {
        variant_cast<db_int64, T, true> cast_force;
        m_value = cast_force(expr->u.iValue);
        return RT_SUCCEEDED;
    }

    virtual result_t calc(query_pack_t* pack, const stack_segment_t& result) {
        T* out = (T*)result.ptr();

        // TODO(scott.zgeng): 以下代码用来测试
        for (db_int32 i = 0; i < SEGMENT_SIZE; i++) {
            out[i] = m_value;
        }

        return RT_SUCCEEDED;
    }

    virtual data_type_t data_type() { variant_type<T> type; return type(); }

private:
    T m_value;   
};




#endif //__EXPRESSION__


