// for the test

#include <stdio.h>
#include <string.h>
#include "../sql/sqlite3.h"



#include "../kernel/define.h"



static int do_command(sqlite3* db, const char* sql)
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

int main(int argc, char **argv)
{
    int ret;
    sqlite3* db;
    ret = sqlite3_open(NULL, &db);
    IF_RETURN(ret, ret != SQLITE_OK);

    ret = do_command(db, "create table test1 (f1 int);");
    IF_RETURN(ret, ret != SQLITE_DONE);

    ret = do_command(db, "create table test2 (f1 int);");
    IF_RETURN(ret, ret != SQLITE_DONE);

    ret = do_command(db, "insert into test1 values (1);");
    IF_RETURN(ret, ret != SQLITE_DONE);

    ret = do_command(db, "insert into test1 values (2);");
    IF_RETURN(ret, ret != SQLITE_DONE);

    ret = do_command(db, "insert into test2 values (1);");
    IF_RETURN(ret, ret != SQLITE_DONE);

    ret = do_command(db, "insert into test2 values (2);");
    IF_RETURN(ret, ret != SQLITE_DONE);

    //const char sql[] = "select a.f1, count(a.f1) from test1 a, test2 b where a.f1 = b.f1 group by a.f1 order by a.f1;";
    const char sql[] = "select f1, (f1 - 3) from test1 where f1 > 5 ";

    sqlite3_stmt* stmt;
    const char* tail;    
    ret = sqlite3_vector_prepare(db, sql, strlen(sql), &stmt, &tail);
    IF_RETURN(ret, ret != SQLITE_OK);

    
    while (sqlite3_vector_step(stmt) == SQLITE_ROW) {
        int int_value = sqlite3_vector_column_int(stmt, 0);
        printf("ROW %d", int_value);
        long long long_value = sqlite3_vector_column_bigint(stmt, 1);
        printf(", %lld  \n", long_value);
    }

    sqlite3_vector_finalize(stmt);

    getchar();

    return 0;
}

