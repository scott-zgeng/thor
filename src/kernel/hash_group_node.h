// hash_group_node.h by scott.zgeng@gmail.com 2014.08.29


#ifndef  __HASH_GROUP_NODE_H__
#define  __HASH_GROUP_NODE_H__



#include "exec_node.h"
#include "pod_hash_map.h"



struct group_key_t
{
    void* row_addr;
};

struct aggr_row_t
{
    void* row_addr;
};


class combine_group_columns_t
{

};


class combine_aggr_columns_t
{

};



class database_t;
class aggr_table_allocator
{
public:
    void init(void* param) {
        database_t* db = (database_t*)param;
        m_mem_region.init(db->get_mem_pool());
    }

    void uninit() {
        m_mem_region.release();
    }
    
    void* alloc_bucket(size_t sz) {
        return malloc(sz);
    }

    void free_bucket(void* p) {
        free(p);
    }

    void* alloc_node(size_t sz) {
        return m_mem_region.alloc(sz);        
    }

    void free_node(void* p) {        
    }
private:
    mem_region_t m_mem_region;
};



class aggr_table_t
{ 
public:
    typedef pod_hash_map<group_key_t, aggr_row_t, 12289, aggr_table_allocator> hash_table_t;

public:
    aggr_table_t(database_t* db) : m_hash_table(true, db) {
        m_mem_region.init(db->get_mem_pool());
    }


    result_t insert_update() {

    }


    void insert();
    void update();


private:
    hash_table_t m_hash_table;
    mem_region_t m_mem_region;
};




class hash_group_node_t : public node_base_t
{
public:
    static const db_uint32 MAX_GROUP_COLUMNS = 16;
    static const db_uint32 MAX_AGGR_COLUMNS = 16;
public:
    enum aggr_type_t {
        AGGR_UNKNOWN = 0,
        AGGR_COUNT = 1,
        AGGR_MIN = 2,
        AGGR_MAX = 3,
        AGGR_SUM = 4,
        AGGR_AVG = 5,
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
    node_base_t* m_children;
    database_t* m_database;
    aggr_table_t m_aggr_table;

    pod_vector<expr_base_t*, MAX_GROUP_COLUMNS> m_group_columns;
    pod_vector<expr_base_t*, MAX_AGGR_COLUMNS> m_aggr_columns;

};



#endif //__HASH_GROUP_NODE_H__


