// database.cpp by scott.zgeng@gmail.com 2014.08.20


#include "parse_ctx.h"
#include "database.h"
#include "smart_pointer.h"
#include "column_table.h"
#include "service_config.h"

//-----------------------------------------------------------------------------
// database_t
//-----------------------------------------------------------------------------
database_t database_t::instance;

result_t database_t::init(service_config_t* config)
{
    if (m_kernel_db != NULL) 
        return RT_SUCCEEDED;
        
    db_size mem_size = config->mem_pool_size * 1024 * 1024;        
    result_t ret = m_mem_pool.init(mem_size);
    IF_RETURN_FAILED(ret != RT_SUCCEEDED);
    
    db_int32 ret2 = sqlite3_open(NULL, &m_kernel_db);
    IF_RETURN_FAILED(ret2 != SQLITE_OK);

    ret = m_executor.init();

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

    result_t ret = new_table->init(this, table, table_id);
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




