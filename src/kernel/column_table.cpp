// column_table.cpp by scott.zgeng@gmail.com 2014.08.12


#include "column_table.h"
#include "expression.h"
#include "smart_pointer.h"
#include "parse_ctx.h"
#include "mem_pool.h"





//-----------------------------------------------------------------------------
// cursor_t
//-----------------------------------------------------------------------------
cursor_t::cursor_t()
{
    m_curr_segment_id = 0;
    m_segment_count = 0;
    m_row_count = 0;
    m_table = NULL;
}

cursor_t::~cursor_t()
{
}


result_t cursor_t::next_segment(row_set_t* rows)
{
    if (m_curr_segment_id >= m_segment_count) {
        rows->set_count(0);
        return RT_SUCCEEDED;
    }
    
    rows->set_mode(row_set_t::SEGMENT_MODE);
    db_int32 row_count = SEGMENT_SIZE;
    if (m_curr_segment_id + 1 == m_segment_count) {
        row_count = m_row_count - m_curr_segment_id * SEGMENT_SIZE;
    }

    rows->set_count(row_count);
    rowid_t* row_list = rows->data();

    rowid_t temp_row = m_curr_segment_id * SEGMENT_SIZE;
    for (db_int32 i = 0; i < row_count; i++) {
        row_list[i] = temp_row;
        temp_row++;
    }

    m_curr_segment_id++;    
    return RT_SUCCEEDED;
}



//-----------------------------------------------------------------------------
// column_table_t
//-----------------------------------------------------------------------------

static data_type_t get_data_type(Column* column)
{
    if (column->affinity == SQLITE_AFF_INTEGER) {

        if (strcmp(column->zType, "tinyint") == 0)
            return DB_INT8;
        else if (strcmp(column->zType, "smallint") == 0)
            return DB_INT16;
        else if (strcmp(column->zType, "int") == 0)
            return DB_INT32;
        else if (strcmp(column->zType, "bigint") == 0)
            return DB_INT64;
        else
            return DB_UNKNOWN;
    } 
    
    return DB_UNKNOWN;
}


result_t column_table_t::init(Table* table, db_int32 table_id)
{    
    m_table_name = table->zName;    
    m_table_id = table_id;
 
    for (db_int32 i = 0; i < table->nCol; i++) {
        Column* col = table->aCol + i;
        data_type_t type = get_data_type(col);        
        IF_RETURN_FAILED(type == DB_UNKNOWN);

        result_t ret = add_column(type);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);
    }

    return RT_SUCCEEDED;
}


result_t column_table_t::add_column(data_type_t type)
{
    smart_pointer<column_base_t> new_column = column_base_t::create_column(type);        
    IF_RETURN_FAILED(new_column.ptr() == NULL);

    result_t ret = new_column->init(get_mem_pool(), 0);
    IF_RETURN_FAILED(ret != RT_SUCCEEDED);

    bool is_succ = m_columns.push_back(new_column.ptr());
    IF_RETURN_FAILED(!is_succ);

    new_column.detach();
    return RT_SUCCEEDED;
}


column_table_t::column_table_t()
{
}

column_table_t::~column_table_t()
{
}

result_t column_table_t::init_cursor(cursor_t* cursor)
{    
    cursor->m_curr_segment_id = 0;
    cursor->m_table = this;
    cursor->m_segment_count = get_segment_count();
    return RT_SUCCEEDED;
}
 
