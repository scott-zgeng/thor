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


// row_set提供的功能： 
//   有多少行数据
//   如果获取行的数据
class row_set_t
{
public:
    enum mode_t {
        SEGMENT_MODE = 0, 
        RANDOM_MODE,
    };

    row_set_t() {
        m_count = 0;
        m_segment_id = 0;
        m_mode = SEGMENT_MODE;
        m_data = NULL;
        m_data_alloc = NULL;
    }

    row_set_t(rowid_t* ptr) {
        m_count = 0;
        m_segment_id = 0;
        m_mode = SEGMENT_MODE;
        m_data = ptr;
        m_data_alloc = NULL;
    }

    ~row_set_t() {        
        if (m_data_alloc != NULL) {
            free(m_data_alloc);
        }
    }

    result_t init(db_int32 column_num) {
        assert(m_data_alloc == NULL);

        m_data_alloc = (rowid_t*)malloc(column_num * SEGMENT_SIZE * sizeof(rowid_t));
        if (NULL == m_data_alloc)
            return RT_FAILED;

        m_data = m_data_alloc;
        return RT_SUCCEEDED;
    }

    void init(rowid_t* ptr) {
        m_data = ptr;
    }

    db_int32 count() const {
        return m_count;
    }

    void set_count(db_int32 n) {
        m_count = n;
    }

    void set_mode(mode_t mode) {
        m_mode = mode;
    }

    rowid_t* data() const {
        return m_data;
    }

    mode_t mode() const {
        return m_mode;
    }

    db_int32 segment() const {
        return m_segment_id;
    }

private:
    db_int32 m_count;
    db_int32 m_segment_id;
    mode_t m_mode;
    rowid_t* m_data;
    rowid_t* m_data_alloc;
};


class mem_stack_t
{
public:
    friend class mem_handle_t;

    mem_stack_t() {
        m_begin = m_buffer;
        m_end = m_begin + sizeof(m_buffer);
        m_position = m_begin;
    }

    mem_stack_t::~mem_stack_t() {
    }

public:
    void alloc_memory(db_int32 size, mem_handle_t& handle);

    void reset() {
        m_position = m_begin;
    }

private:
    db_int8* m_begin;
    db_int8* m_end;
    db_int8* m_position;

    db_int8 m_buffer[SEGMENT_SIZE * 1024 * 2];
};


class mem_handle_t
{
public:
    mem_handle_t() {
        init(NULL, NULL);
    }

    mem_handle_t(mem_stack_t* ctx, void* data) {
        init(ctx, data);        
    }

    ~mem_handle_t() {
        uninit();
    }

    void init(mem_stack_t* ctx, void* data) {
        DB_TRACE("mem_handle init, %p", data);
        m_ctx = ctx;
        m_data = (db_int8*)data;
    }

    void uninit() {
        // NOTE(scott.zgeng): 
        //  只释放属于栈的空间，如果不是，有可能是空，也可能是应用自己的空间，属于合理用法        
        if (m_data >= m_ctx->m_buffer && m_data < m_ctx->m_end) {
            assert(m_data > m_ctx->m_position);
            m_ctx->m_position = m_data;
            DB_TRACE("mem_handle uninit, %p", m_data);
        }
        else {
            DB_TRACE("mem_handle uninit, no free %p", m_data);
        }
    }

    void* transfer() {
        db_int8* data = m_data;
        m_data = NULL;
        return data;
    }

    void* ptr() const {
        return m_data;
    }

private:
    mem_stack_t* m_ctx;
    db_int8* m_data;

};


class expr_base_t
{
public:
    // 需要增加一个优化表达式的函数
    //static result_t optimize(Expr* expr, expr_base_t** root);

    static result_t build(Expr* expr, expr_base_t** root);

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

    virtual result_t calc(row_set_t* rows, mem_stack_t* ctx, mem_handle_t& result) = 0;


private:
    static expr_base_t* create_instance(Expr* expr, expr_base_t* left, expr_base_t* right);
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

    virtual result_t calc(row_set_t* rows, mem_stack_t* ctx, mem_handle_t& result) {

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
struct binary_primitive<OP_TYPE, T, RT> \
{\
    RT operator() (T a, T b) { \
        return a SYMBOL b; \
    } \
}


BINARY_PRIMITIVE(TK_AND, &&);
BINARY_PRIMITIVE(TK_OR, ||);

BINARY_PRIMITIVE(TK_EQ, == );
BINARY_PRIMITIVE(TK_NE, != );
BINARY_PRIMITIVE(TK_GT, > );
BINARY_PRIMITIVE(TK_GE, >= );
BINARY_PRIMITIVE(TK_LT, < );
BINARY_PRIMITIVE(TK_LE, <= );

BINARY_PRIMITIVE(TK_PLUS, +);
BINARY_PRIMITIVE(TK_MINUS, -);
BINARY_PRIMITIVE(TK_STAR, *);
BINARY_PRIMITIVE(TK_SLASH, /);



// NOTE(scott.zgeng): 没有使用操作符重载，觉得没太大必要
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

    virtual result_t calc(row_set_t* rows, mem_stack_t* mem, mem_handle_t& result) {

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

    virtual result_t calc(row_set_t* rows, mem_stack_t* ctx, mem_handle_t& result) {
        assert(sizeof(T) <= sizeof(RT > ));

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

    virtual result_t calc(row_set_t* rows, mem_stack_t* ctx, mem_handle_t& result) {
        
        if (rows->mode() == row_set_t::SEGMENT_MODE) {
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
    
    virtual result_t calc(row_set_t* rows, mem_stack_t* ctx, mem_handle_t& result) {
        result.init(ctx, m_value);
        return RT_SUCCEEDED;
    }

private:
    T m_value[SEGMENT_SIZE];    
};




#endif //__EXPRESSION_H__


