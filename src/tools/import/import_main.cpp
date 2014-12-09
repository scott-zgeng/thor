// for the test

#include "hash_join.h"
#include "hash_group.h"

//int main(int argc, char* argv[])
//{
//    DB_TRACE("TEST PARALLETING HASH JOIN");
//    current_hash_join_t::main();
//    return 0;
//}

//int main(int argc, char* argv[])
//{
//    DB_TRACE("TEST PARALLETING HASH JOIN");
//    current_hash_group_t instance;
//    instance.test(argc, argv);
//    return 0;
//}


#include <immintrin.h>
#include <nmmintrin.h>
#include <x86intrin.h>
#include "define.h"


static db_int64 VECTOR_SIZE = 0;
static db_int64 SIMD_WIDTH = 4;

class test_expr_t
{
public:
    virtual void calc(int* buffer, int*& result) = 0;
    virtual void reset() = 0;
};



class test_add_expr_t : public  test_expr_t
{
public:
    test_add_expr_t(test_expr_t* left, test_expr_t* right) {
        m_left = left;
        m_right = right;
    }

    virtual void reset() {
        m_left->reset();
        m_right->reset();
    }

    virtual void calc(int* buffer, int*& result) {
        int* left_buffer = buffer + VECTOR_SIZE;
        int* l_result;
        m_left->calc(left_buffer, l_result);

        int* r_result;
        int* right_buffer = left_buffer + VECTOR_SIZE;
        m_right->calc(right_buffer, r_result);

        result = buffer;

        for (db_uint32 i = 0; i < VECTOR_SIZE; i++) {
            result[i] = l_result[i] + r_result[i];
        }
    }


    test_expr_t* m_left;
    test_expr_t* m_right;
};




class test_simd_add_expr_t : public  test_expr_t
{
public:
    test_simd_add_expr_t(test_expr_t* left, test_expr_t* right) {
        m_left = left;
        m_right = right;
    }

    virtual void reset() {
        m_left->reset();
        m_right->reset();
    }


    virtual void calc(int* buffer, int*& result) {
        int* left_buffer = buffer + VECTOR_SIZE;
        int* l_result;
        m_left->calc(left_buffer, l_result);

        int* r_result;
        int* right_buffer = left_buffer + VECTOR_SIZE;
        m_right->calc(right_buffer, r_result);

        result = buffer;

        __m128i* left = (__m128i*)l_result;
        __m128i* right = (__m128i*)r_result;
        __m128i* s_result = (__m128i*)result;

        for (db_uint32 i = 0; i < VECTOR_SIZE / SIMD_WIDTH; i++) {            
            s_result[i] = _mm_add_epi32(left[i], right[i]);
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

    virtual void reset() {
        m_idx = 0;
    }


    virtual void calc(int* buffer, int*& result) {
        result = m_column + m_idx;
        //result = buffer;
        //memcpy(buffer, m_column + m_idx, sizeof(int) * VECTOR_SIZE);        
        m_idx += VECTOR_SIZE;
    }

    db_int32 m_idx;
    db_int32* m_column;
};


result_t test_expr(int argc, char* argv[])
{
    if (argc != 5) {
        DB_TRACE("invalid param");
        return RT_SUCCEEDED;
    }

    
    static db_int64 TABLE_COUNT = atoll(argv[1]);
    static db_int64 LOOP_COUNT = atoll(argv[2]);
    static db_int64 SEGMENT_BIT = atoll(argv[3]);
    static db_int64 USE_SIMD = atoll(argv[4]);

    DB_TRACE("TABLE_COUNT = %lld", TABLE_COUNT);
    DB_TRACE("LOOP_COUNT = %lld", LOOP_COUNT);
    DB_TRACE("SEGMENT_BIT = %lld", SEGMENT_BIT);
    DB_TRACE("USE_SIMD = %lld", USE_SIMD);


    VECTOR_SIZE = (1 << SEGMENT_BIT);
    
    if (TABLE_COUNT % VECTOR_SIZE != 0) {
        DB_TRACE("invalid param TABLE_COUNT");
        return RT_SUCCEEDED;
    }

    DB_TRACE("VECTOR_SIZE = %lld", VECTOR_SIZE);
    DB_TRACE("SIMD_WIDTH = %lld", SIMD_WIDTH);

    if (VECTOR_SIZE < SIMD_WIDTH && USE_SIMD) {
        DB_TRACE("invalid param SIMD_WIDTH");
        return RT_SUCCEEDED;
    }
    
    db_char* buffer = (db_char*)malloc(VECTOR_SIZE * sizeof(int)* 4);
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

    DB_TRACE("start epxr test");
    int* result;
    for (db_int64 n = 0; n < LOOP_COUNT; n++) {
        add_expr->reset();
        for (db_int64 i = 0; i < TABLE_COUNT / VECTOR_SIZE; i++) {
            add_expr->calc(buf_ptr, result);
        }
    }
    DB_TRACE("epxr test ok");

    return RT_SUCCEEDED;
}


int main(int argc, char* argv[])
{
    DB_TRACE("test expr");
    test_expr(argc, argv);
    return 0;
}




