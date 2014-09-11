// hash_group_node.h by scott.zgeng@gmail.com 2014.08.29


#ifndef  __HASH_GROUP_NODE_H__
#define  __HASH_GROUP_NODE_H__



#include "exec_node.h"
#include "pod_hash_map.h"

// TODO(scott.zgeng):  聚合产生的临时表需要有表结构定义，便于在表达式中查询，表达式的产生全部移到expression去

class row_segement_t
{
public:
    row_segement_t();
    virtual ~row_segement_t();

public:
    result_t add_column(expr_base_t* expr);
    result_t next(rowset_t* rs, mem_stack_t* mem, mem_handle_t& result);

    db_uint32 row_len() const {
        return m_row_len;
    }

public:
    struct expr_item_t {
        expr_base_t* expr;
        db_uint32 offset;
        db_uint32 size;        
    };

    db_uint32 m_row_len;
    pod_vector<expr_item_t, 16> m_columns;
};


struct group_op_base_t
{
    group_op_base_t() { offset = 0; }
    virtual ~group_op_base_t() {}
    virtual db_uint32 calc_hash(db_uint32 hash_val, void* row) = 0;
    virtual db_bool is_equal(void* left_row, void* right_row) = 0;

    db_uint32 offset;
};


struct aggr_op_base_t
{
    virtual ~aggr_op_base_t() {}
    virtual void update(void* dst_row, void* src_row) = 0;

    db_uint32 offset;
};




class hash_group_node_t;
class aggr_table_t
{
private:
    struct hash_node_t {
        db_uint32 hash_val;
        db_byte* key;
        db_byte* value;
        hash_node_t* next;
    };
    
public:
    aggr_table_t();
    ~aggr_table_t();

public:
    result_t init(hash_group_node_t* group_node, rowset_mode_t mode, db_uint32 table_count, db_uint32 row_count);
    result_t add_group_column(expr_base_t* expr);
    result_t add_aggr_column(expr_base_t* expr, aggr_type_t aggr_type);
    void init_complete();

    result_t build(rowset_t* rs, mem_stack_t* mem);
    result_t next(aggr_rowset_t* ars);

private:
    group_op_base_t* create_group_op(data_type_t type);
    aggr_op_base_t* create_aggr_op(data_type_t type, aggr_type_t aggr_type);

    template<aggr_type_t AT>
    aggr_op_base_t* create_min_max_aggr_op(data_type_t type);
    template<aggr_type_t AT>
    aggr_op_base_t* create_sum_avg_aggr_op(data_type_t type);

    
    expr_base_t* create_cast_expr(expr_base_t* expr, aggr_type_t aggr_type);
    expr_base_t* conv_aggr_expr(expr_base_t* expr, aggr_type_t aggr_type);
    
    
    result_t insert_update(db_byte* group_row, db_byte* aggr_row);
    result_t insert(db_uint32 hash_val, db_byte* group_row, db_byte* aggr_row);
    void update(db_byte* dst_row, db_byte* src_row);
    db_uint32 calc_hash(db_byte* row);
    db_bool is_equal(db_byte* left_row, db_byte* right_row);

private:
    mem_row_region_t m_group_table;
    mem_row_region_t m_aggr_table;

    mem_region_t m_hash_region;
    mem_row_region_t::iterator m_iterator;

    row_segement_t m_group_rows;
    row_segement_t m_aggr_rows;

    pod_vector<group_op_base_t*> m_group_ops;
    pod_vector<aggr_op_base_t*> m_aggr_ops;

    hash_group_node_t* m_group_node;
    hash_node_t** m_hash_table;
    db_uint32 m_hash_size;    
    db_uint32 m_row_count;

    db_uint32 m_row_count_per_page;
    db_uint32 m_row_idx;

    rowset_mode_t m_mode;
    db_uint32 m_table_count;
};



class hash_group_node_t : public node_base_t
{
public:
    static const db_uint32 MAX_GROUP_COLUMNS = 16;
    static const db_uint32 MAX_AGGR_COLUMNS = 16;

public:
    hash_group_node_t(statement_t* stmt, node_base_t* children);
    virtual ~hash_group_node_t();

public:
    virtual result_t init(Parse* parse, Select* select);
    virtual void uninit();
    virtual result_t next(rowset_t* rs, mem_stack_t* mem);
    

    virtual rowset_mode_t rowset_mode() const {
        return AGGR_TABLE_MODE;
    }

    virtual db_uint32 table_count() const {
        return 1;
    }

    statement_t* statement() const {
        return m_stmt;
    }

private:
    result_t add_aggr_sub_expr(expr_factory_t& factory, Expr* expr);
    
    result_t build(mem_stack_t* mem);

private:   
    node_base_t* m_children;
    statement_t* m_stmt;

    aggr_table_t m_aggr_table;
    
    db_bool m_first;
    rowset_t* m_sub_rowset;
};


#endif //__HASH_GROUP_NODE_H__


