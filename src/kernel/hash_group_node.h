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
    
protected:
    struct expr_item_t {
        expr_base_t* expr;
        db_uint32 offset;
        db_uint32 size;
    };

    db_uint32 m_row_len;
    pod_vector<expr_item_t, 16> m_columns;
};





class group_columns_t
{
public:
    result_t init(database_t* db);
    result_t add_column(expr_base_t* expr);
    result_t next(rowset_t* rows, mem_stack_t* mem);


private:
    row_segement_t m_rows;
};



// init()
// insert();

// for group
// hash()
// compare()

// for aggr
// update()



//
//template<db_uint32 INIT_SIZE>
//class nsm_columns_t
//{
//public:
//    nsm_columns_t();
//    ~nsm_columns_t();
//
//public:    
//    result_t add_column(expr_base_t* expr) {
//        expr->data_type();  
//
//    }
//    
//
//
//
//};
////
//
//class aggr_table_t
//{ 
//
//public:
//    aggr_table_t(database_t* db) :  {
//        m_mem_region.init(db->get_mem_pool());
//    }
//
//
//    result_t insert_update() {
//
//    }
//
//
//    void insert();
//    void update();
//
//
//private:
//   
//    mem_region_t m_mem_region;
//};




class hash_group_node_t : public node_base_t
{
public:
    static const db_uint32 MAX_GROUP_COLUMNS = 16;
    static const db_uint32 MAX_AGGR_COLUMNS = 16;
public:
    enum aggr_type_t {
        AGGR_UNKNOWN = 0,
        AGGR_COLUMN = 1,
        AGGR_FUNC_COUNT = 2,        
        AGGR_FUNC_SUM = 3,
        AGGR_FUNC_AVG = 4,
        AGGR_FUNC_MIN = 5,
        AGGR_FUNC_MAX = 6,
    };

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
    struct group_item_t {
        expr_base_t* expr;
        void* values;
    };

    struct aggr_item_t {
        expr_base_t* expr;
        aggr_type_t type;
        void* values;
    };

    node_base_t* m_children;
    database_t* m_database;
    //aggr_table_t m_aggr_table;

    pod_vector<group_item_t, MAX_GROUP_COLUMNS> m_group_columns;
    pod_vector<aggr_item_t, MAX_AGGR_COLUMNS> m_aggr_columns;
    
    db_bool m_first;
    rowset_t m_rowset;
};



#endif //__HASH_GROUP_NODE_H__


