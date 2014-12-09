// hash_join.cpp by scott.zgeng@gmail.com 2014.11.23

#include "hash_join.h"
#include "executor.h"
#include "pod_hash_map.h"
#include "cas_wrapper.h"


static db_int64 TABLE_MODE = 0;
static db_int64 THREAD_NUM = 1;
static db_int64 LEFT_TABLE_COUNT = 1024 * 1024 * 40;
static db_int64 RIGHT_TABLE_COUNT = 1024 * 1024 * 40;


current_hash_join_task::current_hash_join_task()
{
    static db_uint32 s_idx = 0;
    m_idx = s_idx;
    s_idx++;

    m_join_handle = NULL;
    m_status = 0;
    m_matched_count = 0;
}

current_hash_join_task::~current_hash_join_task()
{
}


result_t current_hash_join_task::init(current_hash_join_t* join_handle) 
{
    m_join_handle = join_handle;    
    
    m_right_row_count = RIGHT_TABLE_COUNT / THREAD_NUM;
    m_left_row_count = LEFT_TABLE_COUNT / THREAD_NUM;

    m_hash_nodes = (hash_node_test_t*)malloc(m_right_row_count * sizeof(hash_node_test_t));
    for (db_uint32 i = 0; i < m_right_row_count; i++) {
        if (TABLE_MODE == 0)
            m_hash_nodes[i].key = i + m_idx * m_right_row_count;
        else
            m_hash_nodes[i].key = rand() % RIGHT_TABLE_COUNT;

        m_hash_nodes[i].value = i;
    }

    m_left_tables = (db_int64*)malloc(m_left_row_count * sizeof(db_int64));
    for (db_uint32 i = 0; i < m_left_row_count; i++) {
        if (TABLE_MODE == 0)
            m_left_tables[i] = i + m_idx * m_left_row_count;
        else
            m_left_tables[i] = rand() % LEFT_TABLE_COUNT;
    }

    //db_size pool_size = 1024 * 1024 * 20; //MB
    //ret = m_mem_pool.init(pool_size);
    //IF_RETURN_FAILED(ret != RT_SUCCEEDED);

    //m_right_row_count = 1024 * 1024 * 1; // 设计的最大容量
    //m_left_row_count = 1024 * 1024 * 2;

    //m_row_region.init(&m_mem_pool, sizeof(hash_node_test_t));    

    return RT_SUCCEEDED;
}

void current_hash_join_task::run()
{
    switch (m_status)
    {
    case 1:
        build_hash_node();
        break;
    case 2:
        build_hash_table();
        break;
    case 3:
        probe_table();
        break;
    default:
        DB_TRACE("INVALID STATUS");
        break;
    }
    m_join_handle->set_task_completed();
}


void current_hash_join_task::build_hash_node()
{
    hash_function<int> int32_hash;
    for (db_uint32 i = 0; i < m_right_row_count; i++) {
        hash_node_test_t* new_node = m_hash_nodes + i;

        
        new_node->hash_val = int32_hash(new_node->key);
        new_node->next = NULL;
    }
}


void current_hash_join_task::build_hash_table()
{
    //mem_row_region_t::iterator it(&m_row_region);

    hash_node_test_t** hash_table = m_join_handle->m_hash_table;
    db_uint32 hash_table_size = m_join_handle->m_hash_table_size;
    hash_node_test_t** hash_pos;
    hash_node_test_t* hash_node;
    hash_node_test_t* old_entry;

    bool succ;

    for (db_uint32 i = 0; i < m_right_row_count; i++) {
        hash_node = m_hash_nodes + i;

        hash_pos = hash_table + (hash_node->hash_val % hash_table_size);
        do {
            old_entry = *hash_pos;
            hash_node->next = old_entry;
            succ = cas_ptr((volatile void**)hash_pos, old_entry, hash_node);
        } while (!succ);
    }
}

void current_hash_join_task::probe_table()
{   
    hash_node_test_t** hash_table = m_join_handle->m_hash_table;
    db_uint32 hash_table_size = m_join_handle->m_hash_table_size;
    hash_node_test_t* entry;

    hash_function<int> int32_hash;
    db_uint32 hash_val;
    for (db_uint32 i = 0; i < m_left_row_count; i++) {
        db_uint32 key = m_left_tables[i];
        hash_val = int32_hash(key);

        entry = hash_table[hash_val % hash_table_size];
        while (entry != NULL) {
            if (entry->hash_val == hash_val && entry->key == key) {
                m_matched_count++;
            }

            entry = entry->next;
        }     
    }
}


void current_hash_join_t::set_task_completed()
{
    m_lock.lock();
    m_running_task--;
    m_lock.unlock();
}


void current_hash_join_t::wait()
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

void current_hash_join_t::task_start(db_int32 status)
{
    m_running_task = m_tasks.size();
    for (db_uint32 i = 0; i < m_tasks.size(); i++) {
        m_tasks[i]->m_status = status;
        bool is_succ = m_executor.execute(m_tasks[i]);
        if (!is_succ) {
            DB_TRACE("current_hash_join_t::task_start %d failed,", i);
        }        
    }
}




result_t current_hash_join_t::test(int argc, char* argv[])
{
    if (argc != 5) {
        DB_TRACE("invalid param");
        return RT_SUCCEEDED;
    }

    TABLE_MODE = atoll(argv[1]);
    THREAD_NUM = atoll(argv[2]);
    LEFT_TABLE_COUNT = atoll(argv[3]);
    RIGHT_TABLE_COUNT = atoll(argv[4]);

    DB_TRACE("TABLE_MODE = %lld", TABLE_MODE);
    DB_TRACE("THREAD_NUM = %lld", THREAD_NUM);
    DB_TRACE("LEFT_TABLE_COUNT = %lld", LEFT_TABLE_COUNT);
    DB_TRACE("RIGHT_TABLE_COUNT = %lld", RIGHT_TABLE_COUNT);

    result_t ret;
    
    DB_TRACE("start test");
    db_uint32 thread_count = THREAD_NUM;
    db_uint32 task_queue_count = 1024;

    ret = m_executor.init(thread_count, task_queue_count);
    IF_RETURN_FAILED(ret != RT_SUCCEEDED);

    for (db_uint32 i = 0; i < thread_count; i++) {
        current_hash_join_task* task = new current_hash_join_task();
        IF_RETURN_FAILED(task == NULL);

        ret = task->init(this);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);

        bool is_succ = m_tasks.push_back(task);
        IF_RETURN_FAILED(!is_succ);
    }

    DB_TRACE("start build hash node");
    task_start(1);
    wait();
    DB_TRACE("build hash node ok");

    db_int64 total_row_count = 0;
    for (db_uint32 i = 0; i < thread_count; i++) {
        total_row_count += m_tasks[i]->m_right_row_count;
    }
    DB_TRACE("build hash num %lld", total_row_count);

    m_hash_table_size = total_row_count + total_row_count / 2 + 1;

    m_hash_table = (hash_node_test_t**)malloc(m_hash_table_size * sizeof(void*));
    IF_RETURN_FAILED(m_hash_table == NULL);
    memset(m_hash_table, 0, m_hash_table_size * sizeof(void*));

    DB_TRACE("hash table size %lld", m_hash_table_size);
    DB_TRACE("start build hash table");
    task_start(2);
    wait();
    DB_TRACE("build hash table ok ");


    DB_TRACE("start probe hash table");
    task_start(3);
    wait();

    db_uint32 total_matched_count = 0;
    for (db_uint32 i = 0; i < thread_count; i++) {
        total_matched_count += m_tasks[i]->m_matched_count;
    }
    DB_TRACE("probe hash table, total_matched_count = %d", total_matched_count);

    return RT_SUCCEEDED;
}

