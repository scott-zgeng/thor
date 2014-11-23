// hash_group.h by scott.zgeng@gmail.com 2014.11.23


#ifndef  __HASH_GROUP_H__
#define  __HASH_GROUP_H__


#include "define.h"
#include "thread.h"
#include "mem_pool.h"
#include "pod_vector.h"
#include "executor.h"

#define TEST_SEGMENT_SIZE (8192)

struct hash_group_node_test_t
{
    unsigned int hash_val;
    hash_group_node_test_t* next;
    db_int64 key;
    db_int64 value1;
    db_int64 value2;
};


struct table_record_test_t
{
    db_int64 key;
    db_int64 value1;
    db_int64 value2;
};


class current_hash_group_t;
class current_hash_group_task : public runnable_t
{
public:
    current_hash_group_task();
    virtual ~current_hash_group_task();

    result_t init(current_hash_group_t* group_handle);

    void build();
    virtual void run();
    hash_group_node_test_t* alloc_node();

    bool insert_update(hash_group_node_test_t* new_node);
    void update_data(hash_group_node_test_t* curr_node, hash_group_node_test_t* new_node);
    
    int local_insert_update(hash_group_node_test_t* new_node);
    void local_update_data(hash_group_node_test_t* curr_node, hash_group_node_test_t* new_node);

public:
    current_hash_group_t* m_group_handle;    
    table_record_test_t* m_table_partition;
    db_int32 m_idx;
    db_int64 m_table_row_count;

    static const db_int64 LOCAL_HASH_TABLE_SIZE = TEST_SEGMENT_SIZE * 2 + 1;
    hash_group_node_test_t* m_local_hash[LOCAL_HASH_TABLE_SIZE];

    db_int64 m_local_node_size;
    hash_group_node_test_t* m_local_nodes[TEST_SEGMENT_SIZE];

    hash_group_node_test_t* m_hash_node_pool;
    db_int64 m_hash_node_pool_size;
    db_int64 m_hash_node_pool_idx;
    

};


class current_hash_group_t
{

public:   
    result_t test(int argc, char* argv[]);

    void set_task_completed();

    void task_start();
    void wait();

private:
    pod_vector<current_hash_group_task*> m_tasks;
    db_int32 m_running_task;
    mutex_t m_lock;
    executor_t m_executor;

public:
    db_uint32 m_hash_table_size;
    hash_group_node_test_t** m_hash_table;
};



#endif //__HASH_GROUP_H__



