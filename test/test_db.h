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

    ret = do_ddl_command(db, "create table test1 (f1 int, f2 varchar(25), f3 bigint);");
    EXPECT_TRUE(ret == SQLITE_DONE);

    ret = do_ddl_command(db, "create table test2 (f1 int);");
    EXPECT_TRUE(ret == SQLITE_DONE);

    for (int i = 0; i < 30; i++) {
        char buff[1024];
        sprintf_s(buff, 1023, "insert into test1 values (%d, 'hello world, %d', %d);", i, i, i);
        ret = do_dml_command(db, buff);
        EXPECT_TRUE(ret == SQLITE_DONE);    
    }

    
    //const char sql[] = "select a.f1, count(a.f1) from test1 a, test2 b where a.f1 = b.f1 group by a.f1 order by a.f1;";
    //const char sql[] = "select f1, (f1 + 2), f2 from test1 where f1 > 1 and f1 <= 1044 and f2 <> 'test12' and f3 < 3";
    //const char sql[] = "select f1, (f1 + 2), f2, f3 + 3 from test1 where f2 <> 'hello world, 0' and f1 < 15 and f1 > 10 and f3 > 12";

    const char sql[] = "select f1, f2, sum(f3) + 1 / count(f3) + f3 as avg_f,  max(f3), min(f3), avg(f3), count(*), count(f3), sum(f3),  sum(f3) from test1 where f1 > 2 group by f1 order by f1";

    sqlite3_stmt* stmt;
    const char* tail;
    ret = sqlite3_vector_prepare(db, sql, strlen(sql), &stmt, &tail);
    EXPECT_TRUE(ret == SQLITE_OK);


    do {
        db_int32 rc = sqlite3_vector_step(stmt);
        db_int32 row_count = sqlite3_vector_row_count(stmt);
        
        db_uint32 column_count = sqlite3_vector_column_count(stmt);

        for (db_int32 row_idx = 0; row_idx < row_count; row_idx++) {
            printf("ROW[%d]: ", row_idx);
            for (db_uint32 col_id = 0; col_id < column_count; col_id++) {
                data_type_t type = (data_type_t)sqlite3_vector_column_type(stmt, col_id);
                switch (type)
                {
                case DB_UNKNOWN:
                    break;
                case DB_INT8:
                    break;
                case DB_INT16:
                    break;
                case DB_INT32:                    
                    printf("%d ", sqlite3_vector_column_int(stmt, col_id, row_idx));
                    break;
                case DB_INT64:
                    printf("%lld ", sqlite3_vector_column_bigint(stmt, col_id, row_idx));
                    break;
                case DB_FLOAT:
                    break;
                case DB_DOUBLE:
                    break;
                case DB_STRING:
                    printf("%s ", sqlite3_vector_column_string(stmt, col_id, row_idx));
                    break;
                default:
                    break;
                }
            }

            printf("\n");
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

