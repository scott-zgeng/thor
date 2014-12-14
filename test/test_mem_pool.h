// test_mem_pool.h by scott.zgeng@gmail.com 2014.08.19


#include <gtest/gtest.h>

#include "mem_pool.h"


TEST(bitmap, case1)
{

    db_byte bitmap[1] = { 0, };

    db_bool is_used;
    bitmap_t::set(bitmap, 3);
    bitmap_t::set(bitmap, 4);
    bitmap_t::set(bitmap, 7);

    is_used = bitmap_t::get(bitmap, 3);
    EXPECT_TRUE(is_used);

    is_used = bitmap_t::get(bitmap, 2);
    EXPECT_TRUE(!is_used);

    is_used = bitmap_t::get(bitmap, 4);
    EXPECT_TRUE(is_used);

    is_used = bitmap_t::get(bitmap, 7);
    EXPECT_TRUE(is_used);

    bitmap_t::clean(bitmap, 4);
    bitmap_t::clean(bitmap, 3);
    bitmap_t::clean(bitmap, 2);

    is_used = bitmap_t::get(bitmap, 3);
    EXPECT_TRUE(!is_used);

    is_used = bitmap_t::get(bitmap, 2);
    EXPECT_TRUE(!is_used);

    is_used = bitmap_t::get(bitmap, 4);
    EXPECT_TRUE(!is_used);

    is_used = bitmap_t::get(bitmap, 7);
    EXPECT_TRUE(is_used);
}



TEST(test_mem_pool, case1)
{
    // for test
    result_t ret;
    
    mem_pool_t the_pool;
    mem_pool_t* pool = &the_pool;

    DB_TRACE("POOL INIT");
    ret = pool->init(1024 * 48);
    EXPECT_TRUE(ret == RT_SUCCEEDED);
    
    DB_TRACE("POOL ALLOC 16");
    void* ptr_16 = pool->alloc_page(SEGMENT_SIZE * 16);
    EXPECT_TRUE(ptr_16 != NULL);
    memset(ptr_16, 0xef, SEGMENT_SIZE * 16);

    DB_TRACE("POOL ALLOC 8");
    void* ptr_8 = pool->alloc_page(SEGMENT_SIZE * 8);
    EXPECT_TRUE(ptr_8 != NULL);
    memset(ptr_8, 0xef, SEGMENT_SIZE * 8);

    DB_TRACE("POOL ALLOC 4");
    void* ptr_4 = pool->alloc_page(SEGMENT_SIZE * 4);
    EXPECT_TRUE(ptr_4 != NULL);
    memset(ptr_4, 0xef, SEGMENT_SIZE * 4);

    DB_TRACE("POOL ALLOC 8_2");
    void* ptr_8_2 = pool->alloc_page(SEGMENT_SIZE * 8);
    EXPECT_TRUE(ptr_8_2 != NULL);
    memset(ptr_8_2, 0xef, SEGMENT_SIZE * 8);
    
    DB_TRACE("POOL ALLOC 4_2");
    void* ptr_4_2 = pool->alloc_page(SEGMENT_SIZE * 4);
    EXPECT_TRUE(ptr_4_2 != NULL);
    memset(ptr_4_2, 0xef, SEGMENT_SIZE * 4);
    
    DB_TRACE("POOL ALLOC 1");
    void* ptr_1 = pool->alloc_page(SEGMENT_SIZE);
    EXPECT_TRUE(ptr_1 != NULL);
    memset(ptr_1, 0xef, SEGMENT_SIZE);

    DB_TRACE("POOL ALLOC 2");
    void* ptr_2 = pool->alloc_page(SEGMENT_SIZE * 2);
    EXPECT_TRUE(ptr_2 != NULL);
    memset(ptr_2, 0xef, SEGMENT_SIZE * 2);

    DB_TRACE("POOL FREE 4");
    pool->free_page(ptr_4);

    DB_TRACE("POOL FREE 8_2");
    pool->free_page(ptr_8_2);

    DB_TRACE("POOL ALLOC 2_2");
    void* ptr_2_2 = pool->alloc_page(SEGMENT_SIZE * 2);
    EXPECT_TRUE(ptr_2_2 != NULL);
    memset(ptr_2_2, 0xef, SEGMENT_SIZE * 2);

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

TEST(test_mem_row_region, case1)
{
	result_t ret;

	mem_pool_t the_pool;

	ret = the_pool.init(mem_pool_t::MAX_PAGE_SIZE*4);
	EXPECT_TRUE(ret == RT_SUCCEEDED);

	db_uint32 len = mem_region_t::MAX_ALLOC_SIZE;
	mem_row_region_t row_region;
	row_region.init(&the_pool, len);

	void* p1 = row_region.alloc();
	EXPECT_TRUE(p1 != NULL);

	void* p2 = row_region.alloc();
	EXPECT_TRUE(p2 != NULL);

	void* p3 = row_region.alloc();
	EXPECT_TRUE(p3 != NULL);
	
	void* p4 = row_region.alloc();
	EXPECT_TRUE(p4 != NULL);

	void* p5 = row_region.alloc();
	EXPECT_TRUE(p5 == NULL);

	mem_row_region_t::iterator it;
	it.init(&row_region);

	void* rp1 = it.next();
	EXPECT_TRUE(rp1 == p4);
	
	void* rp2 = it.next();
	EXPECT_TRUE(rp2 == p3);

	void* rp3 = it.next();
	EXPECT_TRUE(rp3 == p2);

	void* rp4 = it.next();
	EXPECT_TRUE(rp4 == p1);


}




TEST(test_mem_row_region, case2)
{
	result_t ret;

	mem_pool_t the_pool;

	ret = the_pool.init(mem_pool_t::MAX_PAGE_SIZE*4);
	EXPECT_TRUE(ret == RT_SUCCEEDED);

	db_uint32 len = mem_region_t::MAX_ALLOC_SIZE/3;
	mem_row_region_t row_region;
	row_region.init(&the_pool, len);

	db_uint32 count = 0;
	while (true) {
		void* p = row_region.alloc();
		if (p == NULL) break;

		count++;
	}

	EXPECT_TRUE(count == 12);

	mem_row_region_t::iterator it;
	it.init(&row_region);

	db_uint32 fetch_count = 0;
	while (true) {
		void* p = it.next();
		if (p == NULL) break;

		fetch_count++;
	}

	EXPECT_TRUE(count == fetch_count);
	

}

