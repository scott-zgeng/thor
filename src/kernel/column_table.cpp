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


result_t cursor_t::next_segment(single_rowset_t* rs)
{
    if (m_curr_segment_id >= m_segment_count) {
        rs->count = 0;
        return RT_SUCCEEDED;
    }
   
    db_int32 row_count = SEGMENT_SIZE;
    if (m_curr_segment_id + 1 == m_segment_count) {
        row_count = m_row_count - m_curr_segment_id * SEGMENT_SIZE;
    }

    rs->segment_id = m_curr_segment_id;
    rs->count = row_count;
    rowid_t* rows = rs->rows;

    rowid_t temp_row = m_curr_segment_id * SEGMENT_SIZE;
    for (db_int32 i = 0; i < row_count; i++) {
        rows[i] = temp_row;
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

    } else if (column->affinity == SQLITE_AFF_TEXT) {
        return DB_STRING;

    } else {
        return DB_UNKNOWN;
    }
}


result_t column_table_t::init(database_t* db, Table* table, db_int32 table_id)
{    
    assert(db != NULL);

    m_database = db;
    m_table_name = table->zName;    
    m_table_id = table_id;
 
    for (db_int32 i = 0; i < table->nCol; i++) {
        Column* col = table->aCol + i;
        data_type_t type = get_data_type(col);
        IF_RETURN_FAILED(type == DB_UNKNOWN);

        db_uint32 data_len = col->szEst * 4;
        result_t ret = add_column(type, data_len);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);
    }

    return RT_SUCCEEDED;
}


result_t column_table_t::add_column(data_type_t type, db_uint32 len)
{
    smart_pointer<column_base_t> new_column(column_base_t::create_column(type));
    IF_RETURN_FAILED(new_column.is_null());

    result_t ret = new_column->init(get_mem_pool(), len);
    IF_RETURN_FAILED(ret != RT_SUCCEEDED);

    bool is_succ = m_columns.push_back(new_column.ptr());
    IF_RETURN_FAILED(!is_succ);

    new_column.detach();
    return RT_SUCCEEDED;
}


column_table_t::column_table_t()
{
    m_table_id = -1;
    m_row_count = 0;
    m_database = NULL;
}

column_table_t::~column_table_t()
{
}

result_t column_table_t::init_cursor(cursor_t* cursor)
{    
    cursor->m_curr_segment_id = 0;
    cursor->m_table = this;
    cursor->m_segment_count = get_segment_count();
    cursor->m_row_count = get_row_count();
    return RT_SUCCEEDED;
}
 
