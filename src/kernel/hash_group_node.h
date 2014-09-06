// hash_group_node.h by scott.zgeng@gmail.com 2014.08.29


#ifndef  __HASH_GROUP_NODE_H__
#define  __HASH_GROUP_NODE_H__



#include "exec_node.h"
#include "pod_hash_map.h"


class row_segement_t
{
public:
    row_segement_t();
    virtual ~row_segement_t();

public:
    result_t add_column(expr_base_t* expr);
    result_t next(rowset_t* rows, mem_stack_t* mem, mem_handle_t result);

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
    }

    ~aggr_table_t() {
        if (m_hash_table != NULL) {
            free(m_hash_table);
            m_hash_table = NULL;
        }        
    }

    result_t init(database_t* db, db_uint32 row_count) {
        m_group_region.init(db->get_mem_pool());
        m_aggr_region.init(db->get_mem_pool());
        m_hash_region.init(db->get_mem_pool());

        m_hash_size = 393241; // TODO(scott.zgeng): add cala hash size function
        m_hash_table = (hash_node_t**)malloc(m_hash_size * sizeof(hash_node_t*));
        IF_RETURN_FAILED(m_hash_table == NULL);        

        memset(m_hash_table, 0, sizeof(hash_node_t*) * m_hash_size);

        return RT_SUCCEEDED;
    }


    group_op_base_t* create_group_op(data_type_t type);
    aggr_op_base_t* create_aggr_op(data_type_t type, aggr_type_t aggr_type);

    result_t add_group_column(expr_base_t* expr) {
        group_op_base_t* op = create_group_op(expr->data_type());
        db_bool is_succ = m_group_ops.push_back(op);
        IF_RETURN_FAILED(!is_succ);        
        op->offset = m_group_rows.row_len();

        return m_group_rows.add_column(expr);
    }

    result_t add_aggr_column(expr_base_t* expr, aggr_type_t aggr_type) {
        aggr_op_base_t* op = create_aggr_op(expr->data_type(), aggr_type);
        db_bool is_succ = m_aggr_ops.push_back(op);
        IF_RETURN_FAILED(!is_succ);        
        op->offset = m_aggr_rows.row_len();

        return m_aggr_rows.add_column(expr);
    }

    result_t next(rowset_t* rows, mem_stack_t* mem) {
        result_t ret;
        mem_handle_t group_handle;
        ret = m_group_rows.next(rows, mem, group_handle);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);

        mem_handle_t aggr_handle;
        ret = m_aggr_rows.next(rows, mem, aggr_handle);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);
            
        db_byte* group_row = (db_byte*)group_handle.ptr();
        db_byte* aggr_row = (db_byte*)aggr_handle.ptr();

        for (db_uint32 i = 0; i < rows->count(); i++) {
            ret = insert_update(group_row, aggr_row);
            IF_RETURN_FAILED(ret != RT_SUCCEEDED);
            
            group_row += m_group_rows.row_len();
            aggr_row += m_aggr_rows.row_len();
        }

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

        hash_node->key = (db_byte*)m_group_region.alloc(m_group_rows.row_len());
        IF_RETURN_FAILED(hash_node->key == NULL);
        memcpy(hash_node->key, group_row, m_group_rows.row_len());

        hash_node->value = (db_byte*)m_aggr_region.alloc(m_aggr_rows.row_len());
        IF_RETURN_FAILED(hash_node->value == NULL);
        memcpy(hash_node->value, aggr_row, m_aggr_rows.row_len());

        hash_node_t*& entry = m_hash_table[hash_val % m_hash_size];
        hash_node->next = entry;
        entry = hash_node;

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
    mem_region_t m_group_region;
    mem_region_t m_aggr_region;
    mem_region_t m_hash_region;

    row_segement_t m_group_rows;
    row_segement_t m_aggr_rows;

    pod_vector<group_op_base_t*> m_group_ops;
    pod_vector<aggr_op_base_t*> m_aggr_ops;

    hash_node_t** m_hash_table;
    db_uint32 m_hash_size;    
};



class hash_group_node_t : public node_base_t
{
public:
    static const db_uint32 MAX_GROUP_COLUMNS = 16;
    static const db_uint32 MAX_AGGR_COLUMNS = 16;
public:


public:
    hash_group_node_t(database_t* db, node_base_t* children);
    virtual ~hash_group_node_t();

public:
    virtual result_t init(Parse* parse, Select* select);
    virtual void uninit();
    virtual result_t next(rowset_t* rows, mem_stack_t* mem);
    virtual db_int32 rowid_size();

private:
    result_t add_aggr_sub_expr(expr_factory_t& factory, Expr* expr);
    aggr_type_t get_aggr_type(const char* token);
    result_t build(mem_stack_t* mem);

private:   
    node_base_t* m_children;
    database_t* m_database;

    aggr_table_t m_aggr_table;
    
    db_bool m_first;
    rowset_t m_rowset;
};


#endif //__HASH_GROUP_NODE_H__


