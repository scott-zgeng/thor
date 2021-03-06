// expression.h by scott.zgeng@gmail.com 2014.08.5

#ifndef  __EXPRESSION_H__
#define  __EXPRESSION_H__



extern "C" {
#include "sqliteInt.h"
#include "vdbeInt.h"
}


#include "define.h"
#include "variant.h"
#include "column.h"
#include "column_table.h"
#include "rowset.h"


static const db_uint32 DEFAULT_EXPR_NUM = 32;
class expr_base_t;
typedef pod_vector<expr_base_t*, DEFAULT_EXPR_NUM> expr_list_t;


class expr_base_t
{
public:
    virtual ~expr_base_t() {
    }

    virtual result_t init(Expr* expr, void* param) {
        return RT_SUCCEEDED; 
    }

    virtual bool has_null() { 
        return false; 
    }

    virtual data_type_t data_type() = 0;
    virtual result_t calc(rowset_t* rs, mem_stack_t* mem, mem_handle_t& result) = 0;
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

    virtual result_t calc(rowset_t* rs, mem_stack_t* mem, mem_handle_t& result) {

        mem->alloc_memory(sizeof(RT)* SEGMENT_SIZE, result);

        mem_handle_t lresult;
        result_t ret = m_children->calc(rs, mem, lresult);
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

BINARY_PRIMITIVE(TK_PLUS, + );
BINARY_PRIMITIVE(TK_MINUS, - );
BINARY_PRIMITIVE(TK_STAR, * );
BINARY_PRIMITIVE(TK_SLASH, / );



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

    virtual result_t calc(rowset_t* rs, mem_stack_t* mem, mem_handle_t& result) {

        result_t ret;
        mem_handle_t lresult;
        mem_handle_t rresult;

        mem->alloc_memory(sizeof(RT)* SEGMENT_SIZE, result);

        ret = m_left->calc(rs, mem, lresult);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);

        ret = m_right->calc(rs, mem, rresult);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);

        RT* dst = (RT*)result.ptr();
        T* a = (T*)lresult.ptr();
        T* b = (T*)rresult.ptr();

        binary_operator<OP_TYPE, T, RT> binary_op;
        return binary_op(a, b, dst, rs->count);
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

    virtual result_t init(Expr* expr, void* param) {
        return m_children->init(expr, param);
    }

    virtual bool has_null() { 
        return m_children->has_null(); 
    }

    virtual data_type_t data_type() { 
        variant_type<RT> type; 
        return type(); 
    }

    virtual result_t calc(rowset_t* rs, mem_stack_t* mem, mem_handle_t& result) {
        assert(sizeof(T) <= sizeof(RT));

        mem->alloc_memory(sizeof(RT)* SEGMENT_SIZE, result);

        mem_handle_t lresult;
        result_t ret = m_children->calc(rs, mem, lresult);
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


enum aggr_type_t {
    AGGR_UNKNOWN = 0,
    AGGR_COLUMN = 1,
    AGGR_FUNC_COUNT = 2,
    AGGR_FUNC_SUM = 3,
    AGGR_FUNC_AVG = 4,
    AGGR_FUNC_MIN = 5,
    AGGR_FUNC_MAX = 6,
};


class expr_aggr_t : public expr_base_t
{
public:
    virtual ~expr_aggr_t() {

    }
    db_uint32 m_offset;
};



template<typename T>
class expr_aggr_column_t : public expr_aggr_t
{
public:
    expr_aggr_column_t() {
        m_offset = 0;
    }

    virtual ~expr_aggr_column_t() {

    }

    virtual result_t init(Expr* expr, void* param) {    
        return RT_SUCCEEDED;
    }

    virtual data_type_t data_type() {
        variant_type<T> type;
        return type();
    }

    virtual result_t calc(rowset_t* rs, mem_stack_t* mem, mem_handle_t& result) {
        assert(rs->mode == AGGR_TABLE_MODE);
        aggr_rowset_t* ars = (aggr_rowset_t*)rs;

        mem->alloc_memory(sizeof(T)* SEGMENT_SIZE, result);
        T* values = (T*)result.ptr();

        for (db_uint32 i = 0; i < ars->count; i++) {            
            values[i] = *(T*)((db_byte*)ars->rows[i] + m_offset);
        }

        return RT_SUCCEEDED;
    }

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
    virtual result_t init(Expr* expr, void* param) {
        // TODO(scott.zgeng): instance不允许直接使用
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

    virtual result_t calc(rowset_t* rs, mem_stack_t* mem, mem_handle_t& result) {        
        assert(rs->mode == SINGLE_TABLE_MODE);

        single_rowset_t* srs = (single_rowset_t*)rs;
        if (srs->is_scan()) {
            void* values = NULL;            
            result_t ret = m_table->get_segment_values(m_column_id, srs->segment_id, &values);
            IF_RETURN_FAILED(ret != RT_SUCCEEDED);
            result.init(mem, values);
        } else {
            mem->alloc_memory(sizeof(T)* SEGMENT_SIZE, result);
            m_table->get_random_values(m_column_id, srs->rows, srs->count, result.ptr());
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
    virtual ~expr_integer_t() {}

public:
    virtual result_t init(Expr* expr, void* param) {
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
    
    virtual result_t calc(rowset_t* rs, mem_stack_t* mem, mem_handle_t& result) {
        result.init(mem, m_value);
        return RT_SUCCEEDED;
    }

private:
    T m_value[SEGMENT_SIZE];    
};


class expr_float_t : public expr_base_t
{

public:    
    virtual ~expr_float_t() {}

public:
    virtual result_t init(Expr* expr, void* param) {        
        db_double v = atof(expr->u.zToken);
        for (db_int32 i = 0; i < SEGMENT_SIZE; i++) {
            m_value[i] = v;
        }
        return RT_SUCCEEDED;
    }

    virtual data_type_t data_type() {        
        return DB_DOUBLE;
    }

    virtual result_t calc(rowset_t* rs, mem_stack_t* mem, mem_handle_t& result) {
        result.init(mem, m_value);
        return RT_SUCCEEDED;
    }

private:
    db_double  m_value[SEGMENT_SIZE];
};

class expr_string_t : public expr_base_t
{
public:
    virtual ~expr_string_t() {}

    virtual result_t init(Expr* expr, void* param) {
        strncpy_ex(m_string, expr->u.zToken, sizeof(m_value));
        for (db_int32 i = 0; i < SEGMENT_SIZE; i++) {
            m_value[i] = m_string;
        }

        return RT_SUCCEEDED;
    }

    virtual data_type_t data_type() {
        return DB_STRING;
    }

    virtual result_t calc(rowset_t* rs, mem_stack_t* mem, mem_handle_t& result) {
        result.init(mem, m_value);
        return RT_SUCCEEDED;
    }

private:
    db_char m_string[256];
    db_char* m_value[SEGMENT_SIZE];
};


class expr_aggr_count_t : public expr_base_t
{
public:
    expr_aggr_count_t(expr_base_t* children) {
        m_children = children;
    }

    virtual ~expr_aggr_count_t() {
    }

    virtual result_t init(Expr* expr, void* param) {
        return RT_SUCCEEDED;
    }

    virtual bool has_null() {
        return false;
    }

    virtual data_type_t data_type() {
        return DB_INT64;
    }

    virtual result_t calc(rowset_t* rs, mem_stack_t* mem, mem_handle_t& result) {
        mem->alloc_memory(SEGMENT_SIZE*sizeof(db_int64), result);
        db_int64* count_result = (db_int64*)result.ptr();
        for (db_uint32 i = 0; i < rs->count; i++) {
            count_result[i] = 1;
        }
        return RT_SUCCEEDED;
    }

private:
    expr_base_t* m_children;
};



class expr_dummy_t : public expr_base_t
{
public:
    virtual ~expr_dummy_t() {}

public:
    virtual result_t init(Expr* expr, void* param) {
        memset(m_value, 0, sizeof(m_value));        
        return RT_SUCCEEDED;
    }

    virtual data_type_t data_type() {        
        return DB_INT32;
    }

    virtual result_t calc(rowset_t* rs, mem_stack_t* mem, mem_handle_t& result) {
        result.init(mem, m_value);
        return RT_SUCCEEDED;
    }

private:
    db_int32 m_value[SEGMENT_SIZE];
};




class statement_t;
class node_base_t;
class expr_factory_t
{
public:
    expr_factory_t(statement_t* stmt, rowset_mode_t mode, db_uint32 table_count) {
        m_stmt = stmt;
        m_mode = mode;
        m_table_count = table_count;
        m_expr_idx = 0;
    }

    expr_factory_t(statement_t* stmt, node_base_t* node);

public:
    // 需要增加一个优化表达式的函数
    //result_t optimize(Expr* expr, expr_base_t** root);

    result_t build(Expr* expr, expr_base_t** root) {        
        return build_impl(expr, root);        
    }

    result_t build_list(ExprList* src, expr_list_t* dst);

    static expr_base_t* create_cast(data_type_t rt_type, expr_base_t* children);    
    static expr_base_t* create_dummy();
    
    result_t build_scan_table(Expr* expr, expr_base_t** root, const db_char* table_name);

    static db_bool check_or(Expr* expr);
    static Expr* find_join_condition(Expr* expr, const db_char* table1, const db_char* table2);
    
    enum expr_type_t {
        EXPR_CONST = 1,
        EXPR_COLUMN = 2,
        EXPR_OTHER = 3,
    };

    static expr_type_t check_column(Expr* expr, const db_char* table_name);

private:
    template<typename T> static expr_base_t* create_convert_expr_impl(data_type_t rt_type, expr_base_t* children);

    expr_base_t* create_instance(Expr* expr, expr_base_t* left, expr_base_t* right);    
        
    template<int OP_TYPE> expr_base_t* create_binary_logic(expr_base_t* left, expr_base_t* right);
    template<int OP_TYPE> expr_base_t* create_binary_arith(expr_base_t* left, expr_base_t* right);

    expr_base_t* create_integer(Expr* expr);
    expr_base_t* create_column(Expr* expr);

    expr_aggr_t* create_aggr_column(Expr* expr);
    expr_aggr_t* create_aggr_function(Expr* expr);

private:
    data_type_t get_column_type(const char* name, db_int32 column_id);    
    data_type_t cast_type(int op_type, data_type_t left, data_type_t right);
    
    result_t adjust_binary_sub_type(data_type_t dst_type, expr_base_t*& left, expr_base_t*& right);

private:
    result_t build_impl(Expr* expr, expr_base_t** root) {
        switch (m_mode)
        {
        case SINGLE_TABLE_MODE:
        case MULTI_TABLE_MODE:
            return build_normal(expr, root);
        case AGGR_TABLE_MODE:
            return build_aggr(expr, root);
        default:
            return RT_FAILED;
        }
    }

    result_t build_normal(Expr* expr, expr_base_t** root);
    result_t build_aggr(Expr* expr, expr_base_t** root);




    


private:
    statement_t* m_stmt;
    rowset_mode_t m_mode;
    db_uint32 m_table_count;   
    db_uint32 m_expr_idx;
};


db_uint32 calc_data_len(data_type_t type);
aggr_type_t get_aggr_type(const char* token);

#endif //__EXPRESSION_H__


