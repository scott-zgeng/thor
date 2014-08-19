// column_table.h by scott.zgeng@gmail.com 2014.08.12


#ifndef  __COLUMN_TABLE_H__
#define  __COLUMN_TABLE_H__


#include "define.h"

#include "pod_vector.h"
#include "pod_hash_map.h"
#include "mem_pool.h"


struct Table;
class row_set_t;
class column_table_t;


class cursor_t
{
public:
    friend column_table_t;
public:
    cursor_t();
    ~cursor_t();

public:
    result_t next_segment(row_set_t* rows);

private:
    db_int32 m_segment_id;
};


class column_t
{
public:
    column_t();
    ~column_t();

public:
    result_t init(data_type_t type);
    data_type_t type() const { return m_type; }

private:
    data_type_t m_type;
    db_int32 m_size;
};


typedef fstring<MAX_TAB_NAME_LEN + 1> table_name_t;


class column_table_t
{
public:
    column_table_t();
    ~column_table_t();

public:
    result_t init(Table* table, db_int32 table_id);
    result_t add_column(data_type_t type);

    result_t init_cursor(cursor_t* cursor);
    column_t* get_column(db_int32 idx);
    result_t get_segment_values(db_int32 column_id, db_int32 segment_id, void** values);
    result_t get_random_values(rowid_t* rows, db_int32 count, void* values);

private:
    db_int32 m_table_id;
    table_name_t m_table_name;
    pod_vector<column_t*> m_columns;
    db_int32 m_row_count;
};


class database_t
{
public:
    typedef pod_hash_map<table_name_t, column_table_t*, 49157> table_map_t;

public:
    static database_t instance;
    static const db_int32 MAX_TABLE_NUM = 1024;

public:
    database_t(): m_table_map(true) { 
        memset(m_tables, 0, sizeof(m_tables));
    }

public:
    result_t init();
    result_t build_table(Table* table);
    column_table_t* find_table(const char* table_name) const;
    column_table_t* get_table(db_int32 table_id) const;

private:
    db_int32 find_idle_entry();

    table_map_t m_table_map;
    column_table_t* m_tables[MAX_TABLE_NUM];

    mem_pool m_mem_pool;
};


#endif //__COLUMN_TABLE_H__

