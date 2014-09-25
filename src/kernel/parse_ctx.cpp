// parse_ctx.cpp by scott.zgeng@gmail.com 2014.07.11


#include <string.h>
#include <assert.h>

#include "parse_ctx.h"
#include "expression.h"
#include "exec_node.h"
#include "column_table.h"
#include "statement.h"
#include "project_node.h"





int sqlite3VectorCreateTable(Parse* pParse)
{
    if (!pParse->columnStorage) return SQLITE_OK;

    database_t::instance.build_table(pParse->pNewTable);
    return SQLITE_OK;
}


int sqlite3VectorSelect(Parse* pParse, Select* pSelect)
{
    if (!pParse->columnStorage)
        return SQLITE_OK;

    select_stmt_t* stmt = new select_stmt_t(&database_t::instance);
    if (stmt == NULL)
        return SQLITE_ERROR;

    result_t ret = stmt->prepare(pParse, pSelect);
    if (ret != RT_SUCCEEDED) {
        DB_TRACE("select statement prepare failed");
        return SQLITE_ERROR;
    }

    pParse->pVdbe->stmtHandle = stmt;
    return SQLITE_OK;
}



int sqlite3VectorInsert(Parse *pParse, SrcList *pTabList, Select *pSelect, IdList *pColumn, int onError)
{
    if (!pParse->columnStorage) 
        return SQLITE_OK;

    insert_stmt_t* stmt = new insert_stmt_t(&database_t::instance);
    if (stmt == NULL)
        return SQLITE_ERROR;

    result_t ret = stmt->prepare(pParse, pTabList, pSelect, pColumn, onError);
    if (ret != RT_SUCCEEDED) {
        DB_TRACE("insert statement prepare failed");
        return SQLITE_ERROR;
    }

    pParse->stmtHandle = stmt;    
    return SQLITE_OK;
}


void sqlite3VectorInsertEnd(Parse *pParse, Select *pSelect)
{    
    if (pParse->pVdbe != NULL) {
        pParse->pVdbe->stmtHandle = pParse->stmtHandle;
    }
}



void sqlite3VectorFinalize(void* stmtHandle)
{
    assert(stmtHandle != NULL);
    statement_t* stmt = (statement_t*)stmtHandle;        
    stmt->uninit();
    delete stmt;    
}


int sqlite3VectorDBInit()
{
    //result_t ret;
    //ret = database_t::instance.init();

    //if (ret != RT_SUCCEEDED)
    //    return SQLITE_ERROR;

    return SQLITE_OK;
}


int sqlite3_vector_step(sqlite3_stmt* stmt)
{
    Vdbe* v = (Vdbe*)stmt;
    if UNLIKELY(v->stmtHandle == NULL)
        return sqlite3_step(stmt);

    statement_t* v_stmt = (statement_t*)v->stmtHandle;
    return v_stmt->next();
}

project_node_t* get_statement_root_node(sqlite3_stmt* stmt)
{
    Vdbe* v = (Vdbe*)stmt;
    select_stmt_t* select_stmt = (select_stmt_t*)v->stmtHandle;
    return select_stmt->root();
}

int sqlite3_vector_row_count(sqlite3_stmt* stmt)
{
    Vdbe* v = (Vdbe*)stmt;
    select_stmt_t* select_stmt = (select_stmt_t*)v->stmtHandle;
    project_node_t* root_node = select_stmt->root();    
    return root_node->count();
}



int sqlite3_vector_column_count(sqlite3_stmt* stmt)
{
    Vdbe* v = (Vdbe*)stmt;
    select_stmt_t* select_stmt = (select_stmt_t*)v->stmtHandle;
    project_node_t* root_node = select_stmt->root();
    return root_node->column_count();

}



variant_t sqlite3_vector_column_variant(sqlite3_stmt* stmt, int col_idx, int row_idx)
{
    Vdbe* v = (Vdbe*)stmt;
    select_stmt_t* select_stmt = (select_stmt_t*)v->stmtHandle;
    project_node_t* root_node = select_stmt->root();
    data_type_t type = root_node->column_type(col_idx);
    void* row = root_node->column_data(col_idx);

    switch (type)
    {         
    case DB_INT32:
        return variant_t(((db_int32*)row)[row_idx]);
    case DB_INT64:
        return variant_t(((db_int64*)row)[row_idx]);
    case DB_FLOAT:
        return variant_t(((db_float*)row)[row_idx]);
    case DB_DOUBLE:
        return variant_t(((db_double*)row)[row_idx]);
    case DB_STRING:
        return variant_t(((db_string*)row)[row_idx]);    
    default:
        assert(false);
        return variant_t(((db_int32*)row)[row_idx]);
    }
}

int sqlite3_vector_column_int(sqlite3_stmt* stmt, int col_idx, int row_idx)
{
    Vdbe* v = (Vdbe*)stmt;
    select_stmt_t* select_stmt = (select_stmt_t*)v->stmtHandle;
    project_node_t* root_node = select_stmt->root();
    db_int32* val = (db_int32*)root_node->column_data(col_idx);
    return val[row_idx];
}


long long sqlite3_vector_column_bigint(sqlite3_stmt* stmt, int col_idx, int row_idx)
{
    Vdbe* v = (Vdbe*)stmt;
    select_stmt_t* select_stmt = (select_stmt_t*)v->stmtHandle;
    project_node_t* root_node = select_stmt->root();
    db_int64* val = (db_int64*)root_node->column_data(col_idx);
    return val[row_idx];
}


float sqlite3_vector_column_float(sqlite3_stmt* stmt, int col_idx, int row_idx)
{
    Vdbe* v = (Vdbe*)stmt;
    select_stmt_t* select_stmt = (select_stmt_t*)v->stmtHandle;
    project_node_t* root_node = select_stmt->root();
    db_float* val = (db_float*)root_node->column_data(col_idx);
    return val[row_idx];
}


double sqlite3_vector_column_double(sqlite3_stmt* stmt, int col_idx, int row_idx)
{
    Vdbe* v = (Vdbe*)stmt;
    select_stmt_t* select_stmt = (select_stmt_t*)v->stmtHandle;
    project_node_t* root_node = select_stmt->root();
    db_double* val = (db_double*)root_node->column_data(col_idx);
    return val[row_idx];
}



const char* sqlite3_vector_column_string(sqlite3_stmt* stmt, int col_idx, int row_idx)
{
    Vdbe* v = (Vdbe*)stmt;
    select_stmt_t* select_stmt = (select_stmt_t*)v->stmtHandle;
    project_node_t* root_node = select_stmt->root();
    db_string* val = (db_string*)root_node->column_data(col_idx);
    return val[row_idx];
}


int sqlite3_vector_column_type(sqlite3_stmt* stmt, int col_idx)
{
    Vdbe* v = (Vdbe*)stmt;
    select_stmt_t* select_stmt = (select_stmt_t*)v->stmtHandle;
    project_node_t* root_node = select_stmt->root();
    return root_node->column_type(col_idx);
}

