// column_table.cpp by scott.zgeng@gmail.com 2014.08.12


#include "column_table.h"
#include "expression.h"

//-----------------------------------------------------------------------------
// cursor_t
//-----------------------------------------------------------------------------
cursor_t::cursor_t()
{
}

cursor_t::~cursor_t()
{
}


result_t cursor_t::next_segment(row_set_t* rows)
{
    // FOR TEST
    if (m_segment_id > 0) {        
        return RT_FAILED;
    }

    rows->set_mode(row_set_t::SEGMENT_MODE);
    rows->set_count(10);
    rowid_t* row_list = rows->data();

    for (db_int32 i = 0; i < 10; i++) {
        row_list[i] = i;
    }

    m_segment_id++;
    // FOR TEST END

    return RT_SUCCEEDED;
}


//-----------------------------------------------------------------------------
// column_table_t
//-----------------------------------------------------------------------------
column_t::column_t()
{
}

column_t::~column_t()
{
}


data_type_t column_t::type() const 
{
    // FOR TEST
    return DB_INT32;
}


//-----------------------------------------------------------------------------
// column_table_t
//-----------------------------------------------------------------------------
column_table_t* column_table_t::find_table(const char* tab_name)
{
    // for test
    static column_table_t table;
    return &table;
}


column_table_t::column_table_t()
{
}

column_table_t::~column_table_t()
{
}

result_t column_table_t::init_cursor(cursor_t* cursor)
{
    // for test
    cursor->m_segment_id = 0;
    return RT_SUCCEEDED;
}
 
column_t* column_table_t::get_column(db_int32 idx)
{
    // for test
    static column_t col; 
    return &col;
}


result_t column_table_t::get_segment_values(db_int32 column_id, db_int32 segment_id, void** values)
{
    // for test
    static int col_values[SEGMENT_SIZE] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, };
    if (segment_id > 0)
        return RT_FAILED;

    *values = col_values;
    return RT_SUCCEEDED;
}


 
result_t column_table_t::get_random_values(rowid_t* rows, db_int32 count, void* values)
{
    assert(count <= SEGMENT_SIZE);

    // for test
    static int col_values[SEGMENT_SIZE] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, };    
    db_int32* int_val = (db_int32*)values;
    for (int i = 0; i < count; i++) {
        rowid_t curr_row = rows[i];
        int_val[i] = col_values[curr_row];
    }

    return RT_SUCCEEDED;
}

