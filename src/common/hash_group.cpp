// hash_group.cpp by scott.zgeng@gmail.com 2014.11.23

#include <stdlib.h>
#include <stdio.h>

#include "hash_group.h"
#include "executor.h"
#include "pod_hash_map.h"
#include "cas_wrapper.h"


static db_int64 TABLE_MODE = 0;
static db_int64 THREAD_NUM = 1;
static db_int64 HASH_TABLE_SIZE = 1024 * 1024 * 10;
static db_int64 TABLE_SIZE = 1024 * 1024 * 40;
static db_int64 GROUP_NUM = 1024 * 1024;



current_hash_group_task::current_hash_group_task()
{
    static db_int32 s_idx = 0;
    m_idx = s_idx;
    s_idx++;

}

current_hash_group_task::~current_hash_group_task()
{

}



int current_hash_group_task::local_insert_update(hash_group_node_test_t* new_node)
{
    db_uint32 hash_table_size = LOCAL_HASH_TABLE_SIZE;
    hash_group_node_test_t** hash_table = m_local_hash;

    hash_group_node_test_t** pos = hash_table + (new_node->hash_val % hash_table_size);
    hash_group_node_test_t* curr_node;
    
    curr_node = *pos;    
    while (curr_node != NULL) {
        if (curr_node->hash_val == new_node->hash_val && curr_node->key == new_node->key) {
            update_data(curr_node, new_node);
            return 0;
        }
        curr_node = curr_node->next;
    }

    if (m_local_node_size >= TEST_SEGMENT_SIZE) {        
        return -1;
    }

    // insert new node
    m_local_nodes[m_local_node_size] = new_node;
    m_local_node_size++;

    new_node->next = curr_node;
    *pos = new_node;        
    return 1;
}


void current_hash_group_task::local_update_data(hash_group_node_test_t* curr_node, hash_group_node_test_t* new_node)
{   
    {  // max
        db_int64* dst_pos = &curr_node->value1;    
        db_int64 curr_val = *dst_pos;
        db_int64 new_val = curr_val > new_node->value1 ? curr_val : new_node->value1;
        *dst_pos = new_val;    
    }


    {  // count 
        db_int64* dst_pos = &curr_node->value2;        
        db_int64 curr_val = *dst_pos;
        db_int64 new_val = curr_val + new_node->value2;
        *dst_pos = new_val;        
    }
}



bool current_hash_group_task::insert_update(hash_group_node_test_t* new_node) 
{
    db_uint32 hash_table_size = m_group_handle->m_hash_table_size;
    hash_group_node_test_t** hash_table = m_group_handle->m_hash_table;

    hash_group_node_test_t** pos = hash_table + (new_node->hash_val % hash_table_size);

    hash_group_node_test_t* curr_node;
    hash_group_node_test_t* temp_node;

    bool is_succ;
LOOP:
    curr_node = *pos;
    temp_node = curr_node;
    while (temp_node != NULL) {
        if (temp_node->hash_val == new_node->hash_val && temp_node->key == new_node->key) {
            update_data(temp_node, new_node);
            return false;
        }
        temp_node = temp_node->next;
    }

    // insert new node
    new_node->next = curr_node;
    is_succ = cas_ptr((volatile void**)pos, curr_node, new_node);
    if (!is_succ) goto LOOP;
    return true;
}


void current_hash_group_task::build()
{
    hash_group_node_test_t* new_node = NULL;
    //if UNLIKELY(new_node == NULL) {
    //    DB_TRACE("alloc_node FAILED");
    //    return;
    //}

    hash_function<db_int64> hash_func;
    db_bool is_insert = true;
    int local_ret;
    for (db_int64 i = 0; i < m_table_row_count; i++) {

        if (is_insert) {
            new_node = alloc_node();
            if UNLIKELY(new_node == NULL) {
                DB_TRACE("alloc_node FAILED");
                return;
            }
        }

        is_insert = false;

        new_node->key = m_table_partition[i].key;
        new_node->value1 = m_table_partition[i].value1;
        new_node->value2 = m_table_partition[i].value2;
        new_node->hash_val = hash_func(new_node->key);

        // 先放到local中比较
        local_ret = local_insert_update(new_node);
        if (local_ret == 0) {// 是UPDATE
            continue;
        }
        else if (local_ret == 1) { // 是insert
            is_insert = true;
            continue;
        }
        else {
            // 到这说明溢出了                
            is_insert = insert_update(new_node);
        }        
    }

    // 最后把本地的输出到全局的即可
    for (db_int64 i = 0; i < m_local_node_size; i++) {
        new_node = m_local_nodes[i];
        insert_update(new_node);        
    }


}



void current_hash_group_task::update_data(hash_group_node_test_t* curr_node, hash_group_node_test_t* new_node) 
{
    db_bool is_succ;

    {  // max
        db_int64* dst_pos = &curr_node->value1;        
        do {
            db_int64 curr_val = *dst_pos;
            db_int64 new_val = curr_val > new_node->value1 ? curr_val : new_node->value1;
            is_succ = cas_int64(dst_pos, curr_val, new_val);
        } while (!is_succ);
    }


    {  // count 
        db_int64* dst_pos = &curr_node->value2;
        do {
            db_int64 curr_val = *dst_pos;
            db_int64 new_val = curr_val + new_node->value2;
            is_succ = cas_int64(dst_pos, curr_val, new_val);
        } while (!is_succ);
    }
}



hash_group_node_test_t* current_hash_group_task::alloc_node()
{
    if (m_hash_node_pool_idx == m_hash_node_pool_size)
        return NULL;

    hash_group_node_test_t* temp = m_hash_node_pool + m_hash_node_pool_idx;
    m_hash_node_pool_idx++;
    return temp;
}



result_t current_hash_group_task::init(current_hash_group_t* group_handle)
{
    m_table_row_count = TABLE_SIZE / THREAD_NUM;

    m_group_handle = group_handle;
    m_table_partition = (table_record_test_t*)malloc(sizeof(table_record_test_t)*m_table_row_count);

    for (db_int64 i = 0; i < m_table_row_count; i++) {
        if (TABLE_MODE == 0)
            m_table_partition[i].key = (i + m_idx * m_table_row_count) % GROUP_NUM;
        else
            m_table_partition[i].key = rand() % GROUP_NUM;

        m_table_partition[i].value1 = rand() % m_table_row_count;
        m_table_partition[i].value2 = 1;
    }

    // node_pool;
    m_hash_node_pool_size = GROUP_NUM * 2;
    m_hash_node_pool = (hash_group_node_test_t*)malloc(m_hash_node_pool_size * sizeof(hash_group_node_test_t));
    IF_RETURN_FAILED(m_hash_node_pool == NULL);
    m_hash_node_pool_idx = 0;

    // local hash table
    memset(m_local_hash, 0, sizeof(void*) * LOCAL_HASH_TABLE_SIZE);

    // local hash node
    m_local_node_size = 0;
    memset(m_local_nodes, 0, sizeof(void*) * TEST_SEGMENT_SIZE);


    return RT_SUCCEEDED;
}


void current_hash_group_task::run()
{
    build();
    m_group_handle->set_task_completed();
}




void current_hash_group_t::set_task_completed()
{
    m_lock.lock();
    m_running_task--;
    m_lock.unlock();
}


void current_hash_group_t::wait()
{
    while (true) {
        m_lock.lock();
        volatile db_int32 running_task = m_running_task;
        m_lock.unlock();

        if (running_task == 0)
            return;

        usleep(10);
    }
}

void current_hash_group_t::task_start()
{
    m_running_task = m_tasks.size();
    for (db_uint32 i = 0; i < m_tasks.size(); i++) {        
        bool is_succ = m_executor.execute(m_tasks[i]);
        if (!is_succ) {
            DB_TRACE("current_hash_join_t::current_hash_group_t %d failed,", i);
        }
    }
}


result_t current_hash_group_t::test(int argc, char* argv[])
{
    result_t ret;

    if (argc != 6) {
        DB_TRACE("invalid param");
        return RT_SUCCEEDED;
    }

    TABLE_MODE = atoll(argv[1]);
    THREAD_NUM = atoll(argv[2]);
    HASH_TABLE_SIZE = atoll(argv[3]);
    TABLE_SIZE = atoll(argv[4]); 
    GROUP_NUM = atoll(argv[5]);

    
    DB_TRACE("TABLE_MODE = %lld", TABLE_MODE);
    DB_TRACE("THREAD_NUM = %lld", THREAD_NUM);
    DB_TRACE("HASH_TABLE_SIZE = %lld", HASH_TABLE_SIZE);
    DB_TRACE("TABLE_SIZE = %lld", TABLE_SIZE);
    DB_TRACE("GROUP_NUM = %lld", GROUP_NUM);
    

    m_hash_table_size = HASH_TABLE_SIZE;
    m_hash_table = (hash_group_node_test_t**)malloc(m_hash_table_size * sizeof(void*));
    IF_RETURN_FAILED(m_hash_table == NULL);

    memset(m_hash_table, 0, m_hash_table_size * sizeof(void*));


    ret = m_executor.init(THREAD_NUM, 1024);
    IF_RETURN_FAILED(ret != RT_SUCCEEDED);

    for (db_uint32 i = 0; i < THREAD_NUM; i++) {
        current_hash_group_task* task = new current_hash_group_task();
        IF_RETURN_FAILED(task == NULL);

        ret = task->init(this);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);

        bool is_succ = m_tasks.push_back(task);
        IF_RETURN_FAILED(!is_succ);
    }

    DB_TRACE("start build hash group");
    task_start();
    wait();
    DB_TRACE("build hash group ok");

    db_int64 total_node_count = 0;
    db_int64 total_aggr_count = 0;
    for (db_int64 i = 0; i < m_hash_table_size; i++) {
        hash_group_node_test_t* entry = m_hash_table[i];
        while (entry != NULL) {
            total_node_count++;
            total_aggr_count += entry->value2;
            entry = entry->next;
        }        
    }
    DB_TRACE("group count %lld, count %lld", total_node_count, total_aggr_count);

    sleep(1);

    return RT_SUCCEEDED;
}
