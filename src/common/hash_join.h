// hash_join.h by scott.zgeng@gmail.com 2014.11.23


#ifndef  __HASH_JOIN_H__
#define  __HASH_JOIN_H__

#include "define.h"
#include "thread.h"
#include "mem_pool.h"
#include "pod_vector.h"
#include "executor.h"

template<typename K, typename V>
struct hash_join_node_t
{
    unsigned int hash_val;
    hash_join_node_t* next;
    K key;
    V value;
};

typedef hash_join_node_t<db_int64, db_int64> hash_node_test_t;


class current_hash_join_t;
class current_hash_join_task : public runnable_t
{
public:
    current_hash_join_task();
    virtual ~current_hash_join_task();

    result_t init(current_hash_join_t* join_handle);
    virtual void run();

    void build_hash_node();
    void build_hash_table();    
    void probe_table();

public:
    current_hash_join_t* m_join_handle;
    db_int64 m_right_row_count;
    db_int64 m_left_row_count;

    //mem_row_region_t m_row_region;
    //mem_pool_t m_mem_pool;
    db_int64 m_status;
    db_int64 m_matched_count;

    hash_node_test_t* m_hash_nodes;
    db_int64* m_left_tables;
    db_uint32 m_idx;
};


class current_hash_join_t
{

public:    
    result_t test(int argc, char* argv[]);

    void set_task_completed();

    void task_start(db_int32 status);
    void wait();

private:
    pod_vector<current_hash_join_task*> m_tasks;
    db_int32 m_running_task;
    mutex_t m_lock;
    executor_t m_executor;

public:
    db_int64 m_hash_table_size;
    hash_node_test_t** m_hash_table;
};




#endif //__HASH_JOIN_H__


