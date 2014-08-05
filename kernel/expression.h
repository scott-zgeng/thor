// expression.h by scott.zgeng@gmail.com 2014.08.5

#ifndef  __EXPRESSION__
#define  __EXPRESSION__

#include "define.h"



extern "C" {
#include "../sql/sqliteInt.h"
#include "../sql/vdbeInt.h"
}



int32 data_type_size(data_type_t type);



class stack_segment_t;
class query_pack_t
{    
public:
    static const int SEGMENT_SIZE = 1024;
public:
    query_pack_t();
    ~query_pack_t();

public:
    result_t generate_data(int32 table_id, int32 column_id);
    const stack_segment_t alloc_segment(data_type_t type, bool has_null);
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
    stack_segment_t(query_pack_t* pack, void* data, data_type_t type, bool has_null) {
        DB_TRACE("enter stack segment, %p", data);     

        m_pack = pack;
        m_data = data;
        m_type = type;
        m_has_null = has_null;
    }

    ~stack_segment_t() {
        DB_TRACE("free stack segment, %p", m_data);

        if (m_data != NULL) {
            m_pack->free_segment(*this);
        }
    }

    void* ptr() const {
        return m_data;
    }

    data_type_t data_type() const {
        return m_type;
    }

    bool has_null() const {
        return m_has_null;
    }

private:
    query_pack_t* m_pack;
    void* m_data;    
    data_type_t m_type;
    bool m_has_null;
};




class expr_base_t
{
public:
    static result_t build(Expr* expr, expr_base_t** root);
    
public:
    expr_base_t() {
        m_left = NULL;
        m_right = NULL;
    }
    virtual ~expr_base_t() {}

    virtual result_t init(Expr* expr) = 0;
    virtual const char* name() = 0;
    virtual result_t calc(query_pack_t* pack, const stack_segment_t& result) = 0;
    
    virtual data_type_t data_type() = 0;
    virtual bool has_null() { return true; }

    expr_base_t* m_left;
    expr_base_t* m_right;

private:
    static expr_base_t* create_instance(Expr* expr);
};


class leaf_expr_t : public expr_base_t
{
public:
    virtual result_t calc(query_pack_t* pack, const stack_segment_t& result) {
        return execute(pack, result);
    }

    virtual result_t execute(query_pack_t* pack, const stack_segment_t& result) = 0;
};


class left_unary_expr_t : public expr_base_t
{
public:
    virtual result_t calc(query_pack_t* pack, const stack_segment_t& result) {
        assert(m_left);

        const stack_segment_t& left_segment = pack->alloc_segment(m_left->data_type(), m_left->has_null());
        m_left->calc(pack, left_segment);

        return execute(pack, result, left_segment);
    }

    virtual result_t execute(query_pack_t* pack, const stack_segment_t& result,
        const stack_segment_t& left) = 0;
};

// NOTE(scott.zgeng): 用的较少，但还是有的
class right_unary_expr_t : public expr_base_t
{
public:
    virtual result_t calc(query_pack_t* pack, const stack_segment_t& result) {
        assert(m_right);

        const stack_segment_t& right_segment = pack->alloc_segment(m_right->data_type(), m_right->has_null());
        m_right->calc(pack, right_segment);

        return execute(pack, result, right_segment);
    }

    virtual result_t execute(query_pack_t* pack, const stack_segment_t& result,
        const stack_segment_t& right) = 0;
};


class binary_expr_t : public expr_base_t
{
public:
    virtual result_t calc(query_pack_t* pack, const stack_segment_t& result) {
        assert(m_left);
        assert(m_right);
        result_t ret;
        const stack_segment_t& left_segment = pack->alloc_segment(m_left->data_type(), m_left->has_null());
        ret = m_left->calc(pack, left_segment);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);

        const stack_segment_t& right_segment = pack->alloc_segment(m_right->data_type(), m_right->has_null());
        ret = m_right->calc(pack, right_segment);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);

        return execute(pack, result, left_segment, right_segment);
    }

    virtual result_t execute(query_pack_t* pack, const stack_segment_t& result,
        const stack_segment_t& left, const stack_segment_t& right) = 0;
};



class expr_gt_t : public binary_expr_t
{
public:
    expr_gt_t();
    virtual ~expr_gt_t();

public:
    virtual result_t init(Expr* expr);
    virtual const char* name() { return "EXPR_GT"; }
    virtual result_t execute(query_pack_t* pack, const stack_segment_t& result,
        const stack_segment_t& left, const stack_segment_t& right);

    virtual data_type_t data_type();
    virtual bool has_null() { return false; }

};


class expr_plus_t : public binary_expr_t
{
public:
    expr_plus_t();
    virtual ~expr_plus_t();

public:
    virtual result_t init(Expr* expr);
    virtual const char* name() { return "EXPR_PLUS"; }

    virtual result_t execute(query_pack_t* pack, const stack_segment_t& result,
        const stack_segment_t& left, const stack_segment_t& right);

    virtual data_type_t data_type();    
};



class expr_multiple_t : public binary_expr_t
{
public:
    expr_multiple_t();
    virtual ~expr_multiple_t();

public:
    virtual result_t init(Expr* expr);
    virtual const char* name() { return "EXPR_MULTIPLE"; }

    virtual result_t execute(query_pack_t* pack, const stack_segment_t& result,
        const stack_segment_t& left, const stack_segment_t& right);

    virtual data_type_t data_type();
    
};



class expr_column_t : public leaf_expr_t
{
public:
    expr_column_t();
    virtual ~expr_column_t();

public:
    virtual result_t init(Expr* expr);
    virtual const char* name() { return "EXPR_COLUMN"; }
    virtual result_t execute(query_pack_t* pack, const stack_segment_t& result);

    virtual data_type_t data_type();


private:
    int32 m_table_id;
    int32 m_column_id;
};



class expr_integer_t : public leaf_expr_t
{
public:
    expr_integer_t();
    virtual ~expr_integer_t();

public:
    virtual result_t init(Expr* expr);
    virtual const char* name() { return "EXPR_INTEGER"; }
    virtual result_t execute(query_pack_t* pack, const stack_segment_t& result);

    virtual data_type_t data_type();

};



#endif //__EXPRESSION__


