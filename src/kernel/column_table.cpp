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
    m_type = DB_UNKNOWN;
}

column_t::~column_t()
{
}


result_t column_t::init(data_type_t type) {
    m_type = type;        



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
    smart_pointer<column_t> new_column;
    db_bool is_succ = new_column.create_instance();
    IF_RETURN_FAILED(!is_succ);

    result_t ret = new_column->init(type);
    IF_RETURN_FAILED(ret != RT_SUCCEEDED);

    is_succ = m_columns.push_back(new_column.ptr());
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


//-----------------------------------------------------------------------------
// database_t
//-----------------------------------------------------------------------------
database_t database_t::instance;

result_t database_t::init() 
{
    result_t ret;

    // TODO(scott.zgeng): 大小先随便写一个，后面动态根据配置，或SQL来配置
    db_size mem_size = 1024 * 1024 * 100; 

    ret = m_mem_pool.init(mem_size);
    IF_RETURN_FAILED(ret != RT_SUCCEEDED);

    return RT_SUCCEEDED;
}


db_int32 database_t::find_idle_entry() 
{
    for (db_int32 i = 0; i < MAX_TABLE_NUM; i++) {
        if (m_tables[i] == NULL)
            return i;
    }
    return INVALID_INT32;
}


result_t database_t::build_table(Table* table)
{    
    db_int32 table_id = find_idle_entry();
    IF_RETURN_FAILED(table_id == INVALID_INT32);

    table_name_t table_name(table->zName);    
    table_map_t::node_pointer node = m_table_map.find_node(table_name);
    IF_RETURN_FAILED(node != NULL);

    smart_pointer<column_table_t> new_table;
    bool is_succ = new_table.create_instance();
    IF_RETURN_FAILED(!is_succ);
    
    result_t ret = new_table->init(table, table_id);
    IF_RETURN_FAILED(ret != RT_SUCCEEDED);
  
    is_succ = m_table_map.insert(table_name, new_table.ptr());
    IF_RETURN_FAILED(!is_succ);

    m_tables[table_id] = new_table.detach();
    return RT_SUCCEEDED;
}


column_table_t* database_t::find_table(const char* table_name) const
{
    table_name_t name(table_name);    
    table_map_t::node_pointer node = m_table_map.find_node(name);
    if (node == NULL) return NULL;

    return node->value();    
}

column_table_t* database_t::get_table(db_int32 table_id) const
{
    assert(0 <= table_id && table_id < MAX_TABLE_NUM);
    return m_tables[table_id];
}




