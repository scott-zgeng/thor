// mem_pool.h by scott.zgeng@gmail.com 2014.08.15


#ifndef  __MEM_POOL_H__
#define  __MEM_POOL_H__

//struct buddy;
//
//struct buddy * buddy_new(int level);
//void buddy_delete(struct buddy *);
//int buddy_alloc(struct buddy *, int size);
//void buddy_free(struct buddy *, int offset);
//int buddy_size(struct buddy *, int offset);
//void buddy_dump(struct buddy *);


#include "define.h"

class mem_pool
{
public:
    static const db_uint32 MIN_BIT = 7;
    static const db_uint32 MAX_BIT = 14;

    static const db_uint32 MIN_PAGE_SIZE = (1 << MIN_BIT);
    static const db_uint32 MAX_PAGE_SIZE = (1 << MAX_BIT);

    // 每个MIN_PAGE占用1bit来表示这个页面是否被占用，用MIN_PAGE组成的完全二叉树需要 (2 * n - 1) bits表示树的状态
    static const db_uint32 BITMAP_BIT = (MAX_BIT - MIN_BIT + 1);
    static const db_uint32 BITMAP_MASK = (1 << BITMAP_BIT) - 1;
    static const db_uint32 BITMAP_UNIT_SIZE = (1 << BITMAP_BIT) / 8;

    static const db_uint32 ENTRY_SIZE = (MAX_BIT - MIN_BIT + 1);

public:
    mem_pool();
    ~mem_pool();

public:
    result_t init(db_int64 pool_size);
    void uninit();

    void* alloc(db_uint32 size);
    void free(void* ptr);

private:
    void* alloc_inner(db_uint32 pow2);

    void set_bitmap(void* ptr, db_uint32 pow);

private:
    struct page_head_t {
        page_head_t* perv;
        page_head_t* next;
    };

    db_byte* m_bitmap;
    db_byte* m_data;

    page_head_t m_free_entry[ENTRY_SIZE];
};


#endif //__MEM_POOL_H__
