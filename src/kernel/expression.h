// expression.h by scott.zgeng@gmail.com 2014.08.5

#ifndef  __EXPRESSION_H__
#define  __EXPRESSION_H__



extern "C" {
#include "../sql/sqliteInt.h"
#include "../sql/vdbeInt.h"
}


#include "define.h"
#include "variant.h"
#include "column.h"
#include "column_table.h"
#include "rowset.h"





class expr_base_t
{
public:
    virtual ~expr_base_t() {
    }

    virtual result_t init(Expr* expr) { 
        return RT_SUCCEEDED; 
    }

    virtual bool has_null() { 
        return false; 
    }

    virtual data_type_t data_type() = 0;
    virtual result_t calc(rowset_t* rows, mem_stack_t* ctx, mem_handle_t& result) = 0;
};



template<int OP_TYPE, typename T, typename RT>
struct unary_operator {
    result_t operator()(T* a, RT* out);
};



template<int OP_TYPE, typename T, typename RT>
class unary_expr_t : public expr_base_t 
{
public:
    unary_expr_t(expr_base_t* children) { 
        m_children = children; 
    }

    virtual ~unary_expr_t() {
    }

    virtual data_type_t data_type() { 
        variant_type<RT> type; 
        return type(); 
    }

    virtual result_t calc(rowset_t* rows, mem_stack_t* ctx, mem_handle_t& result) {

        ctx->alloc_memory(sizeof(RT)* SEGMENT_SIZE, result);

        mem_handle_t lresult;
        result_t ret = m_children->calc(rows, ctx, lresult);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);

        RT* dst = (RT*)result.ptr();
        T* src = (T*)lresult.ptr();

        unary_operator<OP_TYPE, T, RT> unary_op;
        return unary_op(src, dst);
    }

private:
    expr_base_t* m_children;
};


template<int OP_TYPE, typename T, typename RT>
struct binary_primitive 
{
    RT operator() (T a, T b);
};


#define BINARY_PRIMITIVE(OP_TYPE, SYMBOL)  \
template<typename T, typename RT> \
struct binary_primitive<OP_TYPE, T, RT> { \
    RT operator() (T a, T b) { \
        return a SYMBOL b; \
    } \
}


BINARY_PRIMITIVE(TK_AND, &&);
BINARY_PRIMITIVE(TK_OR, ||);

BINARY_PRIMITIVE(TK_EQ, ==);
BINARY_PRIMITIVE(TK_NE, !=);
BINARY_PRIMITIVE(TK_GT, >);
BINARY_PRIMITIVE(TK_GE, >=);
BINARY_PRIMITIVE(TK_LT, <);
BINARY_PRIMITIVE(TK_LE, <=);

BINARY_PRIMITIVE(TK_PLUS, +);
BINARY_PRIMITIVE(TK_MINUS, -);
BINARY_PRIMITIVE(TK_STAR, *);
BINARY_PRIMITIVE(TK_SLASH, /);



#define STRING_BINARY_PRIMITIVE(OP_TYPE, SYMBOL)  \
template<> \
struct binary_primitive<OP_TYPE, db_string, db_int8> {\
    db_int8 operator() (db_string a, db_string b) {\
        return (strcmp(a, b) SYMBOL 0); \
    } \
}


STRING_BINARY_PRIMITIVE(TK_EQ, ==);
STRING_BINARY_PRIMITIVE(TK_NE, !=);
STRING_BINARY_PRIMITIVE(TK_GT, >);
STRING_BINARY_PRIMITIVE(TK_GE, >=);
STRING_BINARY_PRIMITIVE(TK_LT, <);
STRING_BINARY_PRIMITIVE(TK_LE, <=);



template<int OP_TYPE, typename T, typename RT>
struct binary_operator
{
    result_t operator()(T* a, T* b, RT* out, db_int32 count) {
        // TODO(scott.zgeng): 增加SIMD指令的运算

        binary_primitive<OP_TYPE, T, RT> binary_prim;

        for (int i = 0; i < count; i++) {
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
        assert(left);
        assert(right);

        m_left = left;
        m_right = right;
    }

    virtual ~binary_expr_t() {
    }

    virtual data_type_t data_type() { 
        variant_type<RT> type; 
        return type(); 
    }

    virtual result_t calc(rowset_t* rows, mem_stack_t* mem, mem_handle_t& result) {

        result_t ret;
        mem_handle_t lresult;
        mem_handle_t rresult;

        mem->alloc_memory(sizeof(RT)* SEGMENT_SIZE, result);

        ret = m_left->calc(rows, mem, lresult);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);

        ret = m_right->calc(rows, mem, rresult);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);

        RT* dst = (RT*)result.ptr();
        T* a = (T*)lresult.ptr();
        T* b = (T*)rresult.ptr();

        binary_operator<OP_TYPE, T, RT> binary_op;
        return binary_op(a, b, dst, rows->count());
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
    expr_convert_t(expr_base_t* children) { 
        m_children = children; 
    }

    virtual ~expr_convert_t() {
    }

    virtual result_t init(Expr* expr) { 
        return m_children->init(expr); 
    }

    virtual bool has_null() { 
        return m_children->has_null(); 
    }

    virtual data_type_t data_type() { 
        variant_type<RT> type; 
        return type(); 
    }

    virtual result_t calc(rowset_t* rows, mem_stack_t* ctx, mem_handle_t& result) {
        assert(sizeof(T) <= sizeof(RT));

        ctx->alloc_memory(sizeof(RT)* SEGMENT_SIZE, result);

        mem_handle_t lresult;
        result_t ret = m_children->calc(rows, ctx, lresult);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);

        RT* dst = (RT*)result.ptr();
        T* src = (T*)lresult.ptr();

        // TODO(scott.zgeng): 如果这里存在性能问题，可以后续改成SIMD方式        
        variant_cast<T, RT> upcast;
        for (db_int32 i = 0; i < SEGMENT_SIZE; i++) {
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
        m_table = NULL;
        m_column_id = 0;
    }

    virtual ~expr_column_t() {
    }

public:
    virtual result_t init(Expr* expr) {
        m_table = database_t::instance.find_table(expr->pTab->zName);
        IF_RETURN_FAILED(m_table == NULL);

        m_column_id = expr->iColumn;

        column_base_t* column = m_table->get_column(m_column_id);
        IF_RETURN_FAILED(column == NULL || column->data_type() != data_type());        

        return RT_SUCCEEDED;
    }

    virtual data_type_t data_type() { 
        variant_type<T> type; 
        return type(); 
    }

    virtual result_t calc(rowset_t* rows, mem_stack_t* ctx, mem_handle_t& result) {
        
        if (rows->mode() == rowset_t::SCAN_MODE) {
            db_int32 segment_id = rows->segment(); 
            void* values = NULL;
            db_int32 row_count = 0;
            result_t ret = m_table->get_segment_values(m_column_id, segment_id, &values);
            IF_RETURN_FAILED(ret != RT_SUCCEEDED);
            result.init(ctx, values);
        } else {
            ctx->alloc_memory(sizeof(T)* SEGMENT_SIZE, result);
            m_table->get_random_values(m_column_id, rows->data(), rows->count(), result.ptr());
        }

        return RT_SUCCEEDED;
    }


private:
    db_int32 m_column_id;
    column_table_t* m_table;
};


template<typename T>
class expr_integer_t : public expr_base_t
{
public:
    expr_integer_t() { 
    }

    virtual ~expr_integer_t() {
    }

public:
    virtual result_t init(Expr* expr) {
        variant_cast<db_int64, T, true> cast_force;
        T v = cast_force(expr->u.iValue);
        for (db_int32 i = 0; i < SEGMENT_SIZE; i++) {
            m_value[i] = v;
        }
        return RT_SUCCEEDED;
    }

    virtual data_type_t data_type() { 
        variant_type<T> type; 
        return type(); 
    }
    
    virtual result_t calc(rowset_t* rows, mem_stack_t* ctx, mem_handle_t& result) {
        result.init(ctx, m_value);
        return RT_SUCCEEDED;
    }

private:
    T m_value[SEGMENT_SIZE];    
};

class expr_string_t : public expr_base_t
{
public:
    expr_string_t() {

    }

    virtual ~expr_string_t() {

    }

    virtual result_t init(Expr* expr) {        
        strncpy_ex(m_string, expr->u.zToken, sizeof(m_value));
        for (db_int32 i = 0; i < SEGMENT_SIZE; i++) {
            m_value[i] = m_string;
        }

        return RT_SUCCEEDED;
    }

    virtual data_type_t data_type() {
        return DB_STRING;
    }

    virtual result_t calc(rowset_t* rows, mem_stack_t* ctx, mem_handle_t& result) {
        result.init(ctx, m_value);
        return RT_SUCCEEDED;
    }

private:
    db_char m_string[256];
    db_char* m_value[SEGMENT_SIZE];
};


class expr_factory_t
{
public:
    expr_factory_t(database_t* db) {
        m_database = db;
    }

public:
    // 需要增加一个优化表达式的函数
    //result_t optimize(Expr* expr, expr_base_t** root);

    result_t build(Expr* expr, expr_base_t** root);
    expr_base_t* create_convert_expr(data_type_t type, data_type_t rt_type, expr_base_t* children);    

private:
    expr_base_t* create_instance(Expr* expr, expr_base_t* left, expr_base_t* right);    
    template<typename T> expr_base_t* create_convert_expr_impl(data_type_t rt_type, expr_base_t* children);
    expr_base_t* create_expression_binary(Expr* expr, expr_base_t* left, expr_base_t* right);
    template<int OP_TYPE> expr_base_t* create_expression_arith_impl(data_type_t type, expr_base_t* left, expr_base_t* right);
    template<int OP_TYPE> expr_base_t* create_expression_logic_impl(data_type_t type, expr_base_t* left, expr_base_t* right);
    expr_base_t* create_expression_integer(Expr* expr);
    expr_base_t* create_expression_column(Expr* expr);

private:
    data_type_t get_table_column_type(const char* name, db_int32 column_id);    
    data_type_t convert_type(int op_type, data_type_t left, data_type_t right);

private:
    database_t* m_database;
};




#endif //__EXPRESSION_H__


