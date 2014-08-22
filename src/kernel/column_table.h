// column_table.h by scott.zgeng@gmail.com 2014.08.12


#ifndef  __COLUMN_TABLE_H__
#define  __COLUMN_TABLE_H__


#include "define.h"
#include "pod_vector.h"
#include "database.h"
#include "column.h"



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
    db_int32 m_curr_segment_id;
    db_int32 m_segment_count;
    db_int32 m_row_count;
    column_table_t* m_table;
};


struct Table;
class database_t;
class column_table_t
{
public:
    column_table_t();
    ~column_table_t();

public:
    result_t init(Table* table, db_int32 table_id);
    result_t add_column(data_type_t type);

    result_t init_cursor(cursor_t* cursor);

    column_base_t* get_column(db_uint32 column_id) {
        assert(column_id < m_columns.size());
        return m_columns[column_id];
    }

    db_uint32 get_column_count() {
        return m_columns.size();
    }

    result_t get_segment_values(db_uint32 column_id, db_int32 segment_id, void** ptr) {
        assert(column_id < m_columns.size());
        column_base_t* column = m_columns[column_id];
        void* segment = column->get_segment(segment_id);
        IF_RETURN_FAILED(segment == NULL);
        *ptr = segment;

        return RT_SUCCEEDED;
    }


    result_t get_random_values(db_uint32 column_id, rowid_t* rows, db_int32 count, void* ptr) {
        assert(column_id < m_columns.size());
        column_base_t* column = m_columns[column_id];
        column->batch_get_rows_value(rows, count, ptr);
        return RT_SUCCEEDED;
    }


    mem_pool* get_mem_pool() { 
        return m_database->get_mem_pool(); 
    }

    db_uint32 get_segment_count() {
        return m_columns[0]->get_segment_count();
    }

    db_uint32 get_row_count() {
        return m_columns[0]->get_row_count();
    }


private:
    db_int32 m_table_id;
    table_name_t m_table_name;
    pod_vector<column_base_t*> m_columns;
    db_int32 m_row_count;
    database_t* m_database;
};




#endif //__COLUMN_TABLE_H__

