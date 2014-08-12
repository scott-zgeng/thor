// column_table.h by scott.zgeng@gmail.com 2014.08.12


#ifndef  __COLUMN_TABLE_H__
#define  __COLUMN_TABLE_H__


#include "define.h"



// sqlite3StartTable
// sqlite3EndTable

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
    data_type_t type() const;
};



class column_table_t
{
public:
    static column_table_t* find_table(const char* tab_name);

public:
    column_table_t();
    ~column_table_t();

public:
    result_t init_cursor(cursor_t* cursor);
    column_t* get_column(db_int32 idx);
    result_t get_segment_values(db_int32 column_id, db_int32 segment_id, void** values);
    result_t get_random_values(rowid_t* rows, db_int32 count, void* values);


};




#endif //__COLUMN_TABLE_H__

