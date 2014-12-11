// test_db.h by scott.zgeng@gmail.com 2014.08.17





#include <string.h>
#include <gtest/gtest.h>

#include "sqlite3.h"
#include "define.h"



inline int do_ddl_command(sqlite3* db, const char* sql)
{
    int len = strlen(sql);
    int ret;

    sqlite3_stmt* stmt;
    const char* tail;
    ret = sqlite3_vector_prepare(db, sql, len, &stmt, &tail);
    IF_RETURN(ret, ret != SQLITE_OK);

    while (true) {
        ret = sqlite3_step(stmt);
        if (ret != SQLITE_ROW) {
            sqlite3_finalize(stmt);
            return ret;
        }
    }

    sqlite3_finalize(stmt);
    return SQLITE_DONE;
}



inline int do_dml_command(sqlite3* db, const char* sql)
{
    int len = strlen(sql);
    int ret;

    sqlite3_stmt* stmt;
    const char* tail;
    ret = sqlite3_vector_prepare(db, sql, len, &stmt, &tail);
    IF_RETURN(ret, ret != SQLITE_OK);

    while (true) {
        ret = sqlite3_vector_step(stmt);
        if (ret != SQLITE_ROW) {
            sqlite3_vector_finalize(stmt);
            return ret;
        }
    }

    sqlite3_vector_finalize(stmt);
    return SQLITE_DONE;
}


TEST(test_db, case1)
{
    
}

