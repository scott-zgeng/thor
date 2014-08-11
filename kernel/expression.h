// expression.h by scott.zgeng@gmail.com 2014.08.5

#ifndef  __EXPRESSION__
#define  __EXPRESSION__

#include "define.h"
#include "variant.h"



extern "C" {
#include "../sql/sqliteInt.h"
#include "../sql/vdbeInt.h"
}

// row_set�ṩ�Ĺ��ܣ� 
//   �ж���������
//   �����ȡ�е�����
struct row_set_t
{
    db_int32 count;
    db_int32 mode;

    struct row_set_item_t 
    {
        db_int32 mode;
        rowid_t rows[SEGMENT_SIZE];
    } a[1];


public:
    row_set_t() {
        row_count = 0;
        m_ptr = NULL;
    }

    row_set_t(void* ptr) {
        row_count = 0;
        m_ptr = ptr;
    }

    void* ptr() const {
        return m_ptr;
    }


    db_int32 mode;
    db_int32 row_count;
    void* m_ptr;
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
        m_ctx = NULL;
        m_data = NULL;
    }

    ~mem_handle_t() {
        // NOTE(scott.zgeng): 
        //  ֻ�ͷ�����ջ�Ŀռ䣬������ǣ��п����ǿգ�Ҳ������Ӧ���Լ��Ŀռ䣬���ں����÷�        
        if (m_data >= m_ctx->m_buffer && m_data < m_ctx->m_end) {
            assert(m_data > m_ctx->m_position);
            m_ctx->m_position = m_data;
            DB_TRACE("mem_handle free, %p", m_data);

        } else {
            DB_TRACE("mem_handle no free, %p", m_data);
        }
    }

    void init(mem_stack_t* ctx, void* data) {
        DB_TRACE("mem_handle init, %p", data);
        m_ctx = ctx;
        m_data = (db_int8*)data;
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
    // ��Ҫ����һ���Ż����ʽ�ĺ���
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

    virtual data_type_t type() = 0;

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

    virtual data_type_t type() { 
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



// NOTE(scott.zgeng): û��ʹ�ò��������أ�����û̫���Ҫ
template<int OP_TYPE, typename T, typename RT>
struct binary_operator
{
    result_t operator()(T* a, T* b, RT* out, db_int32 count) {
        // TODO(scott.zgeng): ����SIMDָ�������

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

    virtual data_type_t type() { 
        variant_type<RT> type; 
        return type(); 
    }

    virtual result_t calc(row_set_t* rows, mem_stack_t* ctx, mem_handle_t& result) {

        result_t ret;
        mem_handle_t lresult;
        mem_handle_t rresult;

        ctx->alloc_memory(sizeof(RT)* SEGMENT_SIZE, result);

        ret = m_left->calc(rows, ctx, lresult);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);

        ret = m_right->calc(rows, ctx, rresult);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);

        RT* dst = (RT*)result.ptr();
        T* a = (T*)lresult.ptr();
        T* b = (T*)rresult.ptr();

        binary_operator<OP_TYPE, T, RT> binary_op;
        return binary_op(a, b, dst, ctx->row_count());
    }

private:
    expr_base_t* m_left;
    expr_base_t* m_right;
};


// NOTE(scott.zgeng): Ŀǰֻ֧�ִ�С�����������������������ת��
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

    virtual data_type_t type() { 
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

        // TODO(scott.zgeng): �����������������⣬���Ժ����ĳ�SIMD��ʽ        
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
        m_table = table_t::find_table(expr->pTab->zName);
        IF_RETURN_FAILED(m_table == NULL);

        m_column_id = expr->iColumn;

        return RT_SUCCEEDED;
    }

    virtual data_type_t type() { 
        variant_type<T> type; 
        return type(); 
    }

    virtual result_t calc(row_set_t* rows, mem_stack_t* ctx, mem_handle_t& result) {

        // �����ɨ��ģʽ��������SEGMENT_ID�Ϳ��Ի�ȡ����
        // ��������ģʽ��������ROWID ���飬��ȡ��Ӧ����

        
        // result_ptr = table.get_from_segment(rows.segment);
        // ctx->
        if (rows->is_scan()) {
            void* ptr = m_table->get_segment(rows->curr_segment());
            IF_RETURN_FAILED(ptr == NULL);
            result.init(ctx, ptr);
        } else {
            ctx->alloc_memory(sizeof(rowid_t)* SEGMENT_SIZE, result);
            m_table->fill_value(rows, result->ptr());
        }

        return RT_SUCCEEDED;
    }


private:
    db_int32 m_column_id;
    table_t* m_table;
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

    virtual data_type_t type() { 
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




#endif //__EXPRESSION__


