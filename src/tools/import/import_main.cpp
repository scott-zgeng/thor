// for the test

#include "hash_join.h"
#include "hash_group.h"

//int main(int argc, char* argv[])
//{
//    DB_TRACE("TEST PARALLETING HASH JOIN");
//    current_hash_join_t::main();
//    return 0;
//}

int main(int argc, char* argv[])
{
    DB_TRACE("TEST PARALLETING HASH JOIN");
    current_hash_group_t instance;
    instance.test(argc, argv);
    return 0;
}



static db_int64 TEST_EXPR_SEGMENT_SIZE = (1 << 13);


static db_uint32 VECTOR_SIZE = 0;

class test_expr_t
{
public:
    virtual void calc(int* result) = 0;
};



class test_add_expr_t : public  test_expr_t
{
public:
    test_add_expr_t(test_expr_t* left, test_expr_t* right) {
        m_left = left;
        m_right = right;
    }

    virtual void calc(int* buffer) {
        int* left_buffer = buffer + VECTOR_SIZE;
        m_left->calc(left_buffer);

        int* right_buffer = left_buffer + VECTOR_SIZE;
        m_left->calc(right_buffer);

        for (db_uint32 i = 0; i < VECTOR_SIZE; i++) {
            buffer[i] = left_buffer[i] * right_buffer[i];
        }
    }

    test_expr_t* m_left;
    test_expr_t* m_right;
};


#include <immintrin.h>
#include <x86intrin.h>


class test_simd_add_expr_t : public  test_expr_t
{
public:
    test_simd_add_expr_t(test_expr_t* left, test_expr_t* right) {
        m_left = left;
        m_right = right;
    }

    virtual void calc(int* buffer) {
        int* left_buffer = buffer + VECTOR_SIZE;
        m_left->calc(left_buffer);

        int* right_buffer = left_buffer + VECTOR_SIZE;
        m_right->calc(right_buffer);

        for (db_uint32 i = 0; i < VECTOR_SIZE / 8; i++) {
            buffer[i * 8] = _mm256_add_epi32(*(_m256i*)&left_buffer[i * 8], *(_m256i*)&right_buffer[i * 8])
        }
    }

    test_expr_t* m_left;
    test_expr_t* m_right;
};


class test_col_expr_t : public  test_expr_t
{
public:
    test_col_expr_t(int* column) {
        m_column = column;
        m_idx = 0;
    }

    virtual void calc(int* buffer) {
        memcpy(buffer, m_column + m_idx, sizeof(int) * VECTOR_SIZE);        
        m_idx += VECTOR_SIZE;
    }

    db_int32 m_idx;
    db_int32* m_column;
};





#include "define.h"

result_t test_expr(int argc, char* argv[])
{
    if (argc != 5) {
        DB_TRACE("invalid param");
        return RT_SUCCEEDED;
    }

    static db_int64 TABLE_MODE = atoll(argv[1]);
    static db_int64 TABLE_COUNT = atoll(argv[2]);
    static db_int64 SEGMENT_BIT = atoll(argv[3]);
    static db_int64 USE_SIMD = atoll(argv[4]);


    db_int64 TEST_EXPR_SEGMENT_SIZE = (1 << SEGMENT_BIT);
    
    db_char* buffer = (db_char*)malloc(TEST_EXPR_SEGMENT_SIZE * sizeof(int)* 4);
    IF_RETURN_FAILED(buffer == NULL);    
    int* buf_ptr = (int*) (((long long)(buffer + 15) / 16) * 16);    

    int* column1 = (int*)malloc(TABLE_COUNT*sizeof(int));
    int* column2 = (int*)malloc(TABLE_COUNT*sizeof(int));

    for (db_int64 i = 0; i < TABLE_COUNT; i++) {
        column1[i] = i;
        column2[i] = TABLE_COUNT - i;
    }


    test_expr_t* col1 = new test_col_expr_t(column1);
    IF_RETURN_FAILED(col1 == NULL);
    test_expr_t* col2 = new test_col_expr_t(column2);
    IF_RETURN_FAILED(col2 == NULL);

    
    test_expr_t* add_expr;    
    if (USE_SIMD)
        add_expr = new test_simd_add_expr_t(col1, col2);
    else 
        add_expr = new test_add_expr_t(col1, col2);

    IF_RETURN_FAILED(add_expr == NULL);


    for (db_int64 i = 0; i < TABLE_COUNT / TEST_EXPR_SEGMENT_SIZE; i++) {
        add_expr->calc(buf_ptr);
    }

}




