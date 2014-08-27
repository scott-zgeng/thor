// database.h by scott.zgeng@gmail.com 2014.08.20


#ifndef  __DATABASE_H__
#define  __DATABASE_H__


#include "define.h"
#include "fstring.h"
#include "pod_hash_map.h"
#include "mem_pool.h"


typedef fstring<MAX_TAB_NAME_LEN + 1> table_name_t;

struct Table;
class column_table_t;

class database_t
{
public:
    typedef pod_hash_map<table_name_t, column_table_t*, 49157> table_map_t;

public:
    static database_t instance;
    static const db_int32 MAX_TABLE_NUM = 1024;

public:
    database_t() : m_table_map(true) {
        memset(m_tables, 0, sizeof(m_tables));
    }

public:
    result_t init();
    result_t build_table(Table* table);
    column_table_t* find_table(const char* table_name) const;
    column_table_t* get_table(db_int32 table_id) const;

    mem_pool_t* get_mem_pool() { 
        return &m_mem_pool; 
    }

private:
    db_int32 find_idle_entry();

    table_map_t m_table_map;
    column_table_t* m_tables[MAX_TABLE_NUM];

    mem_pool_t m_mem_pool;
};


#endif //__DATABASE_H__

