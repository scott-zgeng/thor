// test_mem_pool.h by scott.zgeng@gmail.com 2014.08.19


#include <gtest/gtest.h>

#include "../src/kernel/mem_pool.h"




TEST(test_mem_pool, case1)
{
    // for test
    result_t ret;
    
    mem_pool one;
    mem_pool* pool = &one;

    DB_TRACE("POOL INIT");
    ret = pool->init(1024 * 48);
    EXPECT_TRUE(ret == RT_SUCCEEDED);
    
    DB_TRACE("POOL ALLOC 16");
    void* ptr_16 = pool->alloc_page(SEGMENT_SIZE * 16);
    EXPECT_TRUE(ptr_16 != NULL);

    DB_TRACE("POOL ALLOC 8");
    void* ptr_8 = pool->alloc_page(SEGMENT_SIZE * 8);
    EXPECT_TRUE(ptr_8 != NULL);

    DB_TRACE("POOL ALLOC 4");
    void* ptr_4 = pool->alloc_page(SEGMENT_SIZE * 4);
    EXPECT_TRUE(ptr_4 != NULL);

    DB_TRACE("POOL ALLOC 8_2");
    void* ptr_8_2 = pool->alloc_page(SEGMENT_SIZE * 8);
    EXPECT_TRUE(ptr_8_2 != NULL);
    
    DB_TRACE("POOL ALLOC 4_2");
    void* ptr_4_2 = pool->alloc_page(SEGMENT_SIZE * 4);
    EXPECT_TRUE(ptr_4_2 != NULL);
    
    DB_TRACE("POOL ALLOC 1");
    void* ptr_1 = pool->alloc_page(SEGMENT_SIZE);
    EXPECT_TRUE(ptr_1 != NULL);

    DB_TRACE("POOL ALLOC 2");
    void* ptr_2 = pool->alloc_page(SEGMENT_SIZE * 2);
    EXPECT_TRUE(ptr_2 != NULL);

    DB_TRACE("POOL FREE 4");
    pool->free_page(ptr_4);

    DB_TRACE("POOL FREE 8_2");
    pool->free_page(ptr_8_2);

    DB_TRACE("POOL ALLOC 2_2");
    void* ptr_2_2 = pool->alloc_page(SEGMENT_SIZE * 2);
    EXPECT_TRUE(ptr_2_2 != NULL);

    DB_TRACE("POOL FREE 8");
    pool->free_page(ptr_8);

    DB_TRACE("POOL FREE 4_2");
    pool->free_page(ptr_4_2);

    DB_TRACE("POOL FREE 1");
    pool->free_page(ptr_1);

    DB_TRACE("POOL FREE 2");
    pool->free_page(ptr_2);

    DB_TRACE("POOL FREE 2_2");
    pool->free_page(ptr_2_2);

    DB_TRACE("POOL COMPLETE");
}

