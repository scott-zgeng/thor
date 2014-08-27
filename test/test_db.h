// test_db.h by scott.zgeng@gmail.com 2014.08.17


#include <gtest/gtest.h>


#include <string.h>
#include "../src/sql/sqlite3.h"
#include "../src/kernel/define.h"




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

void test_func()
{
    int ret;
    sqlite3* db;
    ret = sqlite3_open(NULL, &db);
    EXPECT_TRUE(ret == SQLITE_OK);

    ret = do_ddl_command(db, "create table test1 (f1 int);");
    EXPECT_TRUE(ret == SQLITE_DONE);

    ret = do_ddl_command(db, "create table test2 (f1 int);");
    EXPECT_TRUE(ret == SQLITE_DONE);

    for (int i = 0; i < 2000; i++) {
        char buff[1024];
        sprintf_s(buff, 1023, "insert into test1 values (%d);", i);
        ret = do_dml_command(db, buff);
        EXPECT_TRUE(ret == SQLITE_DONE);    
    }

    /*
    for (int i = 0; i < 3000; i++) {
        char buff[1024];
        sprintf_s(buff, 1023, "insert into test2 values (%d);", i + 1000);
        ret = do_dml_command(db, buff);
        EXPECT_TRUE(ret == SQLITE_DONE);
    }
    */



    //const char sql[] = "select a.f1, count(a.f1) from test1 a, test2 b where a.f1 = b.f1 group by a.f1 order by a.f1;";
    const char sql[] = "select f1, (f1 + 2) from test1 where f1 > 20 and f1 <= 1044";

    sqlite3_stmt* stmt;
    const char* tail;
    ret = sqlite3_vector_prepare(db, sql, strlen(sql), &stmt, &tail);
    EXPECT_TRUE(ret == SQLITE_OK);


    do {
        db_int32 rc = sqlite3_vector_step(stmt);
        db_int32 row_count = sqlite3_vector_row_count(stmt);
        
        for (db_int32 i = 0; i < row_count; i++) {
            int int_value = sqlite3_vector_column_int(stmt, 0, i);
            printf("[%d] ROW = %d", i, int_value);

            db_uint64 long_value = sqlite3_vector_column_bigint(stmt, 1, i);
            printf(" %lld\n", long_value);
        }

        printf("row_count = %d\n", row_count);
        if (rc != SQLITE_ROW) break;

    } while (true);

    sqlite3_vector_finalize(stmt);    
}

TEST(test_db, case1)
{
    test_func();
}

