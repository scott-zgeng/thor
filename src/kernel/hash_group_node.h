// hash_group_node.h by scott.zgeng@gmail.com 2014.08.29


#ifndef  __HASH_GROUP_NODE_H__
#define  __HASH_GROUP_NODE_H__



#include "exec_node.h"
#include "pod_hash_map.h"


class expr_aggr_count_t: public expr_base_t
{
public:
    expr_aggr_count_t(expr_base_t* children) {
        m_children = children;
    }

    virtual ~expr_aggr_count_t() {
    }

    virtual result_t init(Expr* expr) {
        return RT_SUCCEEDED;
    }

    virtual bool has_null() {
        return false;
    }

    virtual data_type_t data_type() {
        return DB_INT64;
    }

    virtual result_t calc(rowset_t* rs, mem_stack_t* ctx, mem_handle_t& result) {
        ctx->alloc_memory(SEGMENT_SIZE*sizeof(db_int64), result);
        db_int64* count_result = (db_int64*)result.ptr();
        for (db_uint32 i = 0; i < rs->count; i++) {
            count_result[i] = 1;
        }
        return RT_SUCCEEDED;
    }

private:
    expr_base_t* m_children;
};



class row_segement_t
{
public:
    row_segement_t();
    virtual ~row_segement_t();

public:
    result_t add_column(expr_base_t* expr);
    result_t next(rowset_t* rs, mem_stack_t* mem, mem_handle_t result);

    db_uint32 row_len() const {
        return m_row_len;
    }

private:
    struct expr_item_t {
        expr_base_t* expr;
        db_uint32 offset;
        db_uint32 size;        
    };

    db_uint32 m_row_len;
    pod_vector<expr_item_t, 16> m_columns;
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
    aggr_table_t() {
        m_hash_size = 0;
        m_hash_table = NULL;
        m_database = NULL;
        m_row_count = 0;
        m_mode = UNKNOWN_MODE;
        m_table_count = 0;
    }

    ~aggr_table_t() {
        if (m_hash_table != NULL) {
            free(m_hash_table);
            m_hash_table = NULL;
        }        
    }
    

    result_t init(database_t* db, rowset_mode_t mode, db_uint32 table_count, db_uint32 row_count) {
        m_database = db;
        m_mode = mode;
        m_table_count = table_count;

        m_hash_size = 393241; // TODO(scott.zgeng): add cala hash size function
        m_hash_table = (hash_node_t**)malloc(m_hash_size * sizeof(hash_node_t*));
        IF_RETURN_FAILED(m_hash_table == NULL);

        memset(m_hash_table, 0, sizeof(hash_node_t*)* m_hash_size);

        return RT_SUCCEEDED;
    }

    void init_complete() {
        m_group_table.init(m_database->get_mem_pool(), m_group_rows.row_len());
        m_aggr_table.init(m_database->get_mem_pool(), m_aggr_rows.row_len());
        m_hash_region.init(m_database->get_mem_pool());
    }


    group_op_base_t* create_group_op(data_type_t type);
    aggr_op_base_t* create_aggr_op(data_type_t type, aggr_type_t aggr_type);

    template<aggr_type_t AT>
    aggr_op_base_t* create_min_max_aggr_op(data_type_t type);
    template<aggr_type_t AT>
    aggr_op_base_t* create_sum_avg_aggr_op(data_type_t type);


    result_t add_group_column(expr_base_t* expr) {
        group_op_base_t* op = create_group_op(expr->data_type());
        db_bool is_succ = m_group_ops.push_back(op);
        IF_RETURN_FAILED(!is_succ);        
        op->offset = m_group_rows.row_len();

        return m_group_rows.add_column(expr);
    }


    expr_base_t* create_cast_expr(expr_base_t* expr, aggr_type_t aggr_type) {        
        
        switch (expr->data_type())
        {
        case DB_INT8:
        case DB_INT16:
        case DB_INT32:     
            return expr_factory_t::create_cast_expr(DB_INT64, expr);
        case DB_FLOAT:
            return expr_factory_t::create_cast_expr(DB_DOUBLE, expr);
        case DB_INT64:
        case DB_DOUBLE:
            return expr;
        default:            
            return NULL;
        }        
    }


    expr_base_t* conv_aggr_expr(expr_base_t* expr, aggr_type_t aggr_type) {
        switch (aggr_type) {
        case AGGR_FUNC_COUNT:
            return new expr_aggr_count_t(expr);            
        case AGGR_FUNC_SUM:
        case AGGR_FUNC_AVG:            
            return create_cast_expr(expr, aggr_type);
        default:
            return expr;
        }
    }

    result_t add_aggr_column(expr_base_t* expr, aggr_type_t aggr_type) {
        expr_base_t* wrapper = conv_aggr_expr(expr, aggr_type); 
        IF_RETURN_FAILED(wrapper == NULL);

        result_t ret = m_aggr_rows.add_column(wrapper);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);

        aggr_op_base_t* op = create_aggr_op(wrapper->data_type(), aggr_type);
        db_bool is_succ = m_aggr_ops.push_back(op);
        IF_RETURN_FAILED(!is_succ);
        op->offset = m_aggr_rows.row_len();

        // NOTE(scott.zgeng): AVG实际是有SUM和COUNT两列组合起来的
        if (aggr_type == AGGR_FUNC_AVG) {
            result_t ret = add_aggr_column(expr, AGGR_FUNC_COUNT);            
            IF_RETURN_FAILED(ret != RT_SUCCEEDED);
        }

        return RT_SUCCEEDED;
    }

    result_t build(rowset_t* rs, mem_stack_t* mem) {
        result_t ret;
        mem_handle_t group_handle;
        ret = m_group_rows.next(rs, mem, group_handle);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);

        mem_handle_t aggr_handle;
        ret = m_aggr_rows.next(rs, mem, aggr_handle);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);
            
        db_byte* group_row = (db_byte*)group_handle.ptr();
        db_byte* aggr_row = (db_byte*)aggr_handle.ptr();

        for (db_uint32 i = 0; i < rs->count; i++) {
            ret = insert_update(group_row, aggr_row);
            IF_RETURN_FAILED(ret != RT_SUCCEEDED);
            
            group_row += m_group_rows.row_len();
            aggr_row += m_aggr_rows.row_len();
        }

        m_iterator.init(&m_aggr_table);
        
        return RT_SUCCEEDED;
    }

    result_t next(aggr_rowset_t* ars) {
        db_uint32 i;          
        for (i = 0; i < SEGMENT_SIZE; i++) {
            void* ptr = m_iterator.next();
            if (ptr == NULL) break;
            ars->rows[i] = ptr;
        }
        
        ars->count = i;        
        return RT_SUCCEEDED;
    }

    result_t insert_update(db_byte* group_row, db_byte* aggr_row) {
        db_uint32 hash_val = calc_hash(group_row);

        hash_node_t* node = m_hash_table[hash_val % m_hash_size];
        while (node != NULL) {
            if (node->hash_val == hash_val && is_equal(node->key, group_row)) {
                update(node->value, aggr_row);
                return RT_SUCCEEDED;
            }
            node = node->next;
        }

        return insert(hash_val, group_row, aggr_row);
    }

    result_t insert(db_uint32 hash_val, db_byte* group_row, db_byte* aggr_row) {
        // TODO(scott.zgeng): 目前只做了浅拷贝，后续看是否需要改为深拷贝

        hash_node_t* hash_node = (hash_node_t*)m_hash_region.alloc(sizeof(hash_node_t));
        IF_RETURN_FAILED(hash_node == NULL);

        hash_node->key = (db_byte*)m_group_table.alloc();
        IF_RETURN_FAILED(hash_node->key == NULL);
        memcpy(hash_node->key, group_row, m_group_rows.row_len());

        hash_node->value = (db_byte*)m_aggr_table.alloc();
        IF_RETURN_FAILED(hash_node->value == NULL);
        memcpy(hash_node->value, aggr_row, m_aggr_rows.row_len());

        hash_node_t*& entry = m_hash_table[hash_val % m_hash_size];
        hash_node->next = entry;
        entry = hash_node;

        m_row_count++;

        return RT_SUCCEEDED;
    }

    void update(db_byte* dst_row, db_byte* src_row) {        
        for (db_uint32 i = 0; i < m_aggr_ops.size(); i++) {
            aggr_op_base_t* op = m_aggr_ops[i];
            op->update(dst_row + op->offset, src_row + op->offset);
        }        
    }

    db_uint32 calc_hash(db_byte* row) {        
        db_uint32 hash_val = 0;
        for (db_uint32 i = 0; i <m_group_ops.size(); i++) {
            group_op_base_t* op = m_group_ops[i];
            hash_val = op->calc_hash(hash_val, row + op->offset);
        }
        return hash_val;
    }

    db_bool is_equal(db_byte* left_row, db_byte* right_row) {        
        for (db_uint32 i = 0; i < m_group_ops.size(); i++) {
            group_op_base_t* op = m_group_ops[i];
            if (!op->is_equal(left_row + op->offset, right_row + op->offset)) {
                return false;
            }
        }
        return true;
    }

private:
    mem_row_table_t m_group_table;
    mem_row_table_t m_aggr_table;

    mem_region_t m_hash_region;
    mem_row_table_t::iterator m_iterator;

    row_segement_t m_group_rows;
    row_segement_t m_aggr_rows;

    pod_vector<group_op_base_t*> m_group_ops;
    pod_vector<aggr_op_base_t*> m_aggr_ops;

    database_t* m_database;
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
    hash_group_node_t(database_t* db, node_base_t* children);
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

private:
    result_t add_aggr_sub_expr(expr_factory_t& factory, Expr* expr);
    aggr_type_t get_aggr_type(const char* token);
    result_t build(mem_stack_t* mem);

private:   
    node_base_t* m_children;
    database_t* m_database;

    aggr_table_t m_aggr_table;
    
    db_bool m_first;
    rowset_t* m_sub_rowset;
};


#endif //__HASH_GROUP_NODE_H__


