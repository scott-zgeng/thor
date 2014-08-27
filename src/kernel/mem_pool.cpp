// mem_pool.cpp by scott.zgeng@gmail.com 2014.08.15



#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "mem_pool.h"


//db_uint32 next_power_of_2(db_uint32 x)
//{
//    --x;
//    x |= x>>1;
//    x |= x>>2;
//    x |= x>>4;
//    x |= x>>8;
//    x |= x>>16;
//    ++x;
//    return x;
//}



static inline db_uint32 calc_power_of_2(db_uint32 x)
{
    x--;
    db_uint32 count = 0;
    while (x != 0) {
        x = x >> 1;
        count++;
    }
    return count;
}


mem_pool_t::mem_pool_t()
{
    m_bitmap = NULL;    
    m_data = NULL;
    m_bitmap_num = 0;    
}

mem_pool_t::~mem_pool_t()
{
    uninit();
}


result_t mem_pool_t::init(db_size pool_size)
{
    IF_RETURN_FAILED(pool_size % MAX_PAGE_SIZE != 0);
    
    // 计算存放 MAX_PAGE 能存放多少个
    db_size page_num = pool_size / MAX_PAGE_SIZE;
    IF_RETURN_FAILED(page_num < 1);
    
    // 一个 MAX_PAGE 对应一个BITMAP
    db_size bitmap_size = page_num * BITMAP_BYTES;
    m_bitmap = (db_byte*)::malloc(bitmap_size);
    IF_RETURN_FAILED(m_bitmap == NULL);

    m_data = (db_byte*)::malloc(pool_size);
    // TODO(scott.zgeng@gmail.com): 先简单写，后面改为自动指针   
    if (m_data == NULL) {
        ::free(m_bitmap);
        m_bitmap = NULL;
        return RT_FAILED;
    }

    m_bitmap_num = page_num;
    
    // 清除BITMAP状态
    memset(m_bitmap, 0, bitmap_size);

    // 链接所有PAGE到空闲链表
    mem_dlist* list = m_free_lists + MAX_LEVEL;
    for (db_byte* ptr = m_data; ptr < m_data + pool_size; ptr += MAX_PAGE_SIZE) {
        list->link(ptr);
    }

    return RT_SUCCEEDED;
}

void mem_pool_t::uninit()
{
    if (m_data != NULL) {
        ::free(m_data);
        m_data = NULL;
    }

    if (m_bitmap != NULL) {
        ::free(m_bitmap);
        m_bitmap = NULL;
    }

    m_bitmap_num = 0;

    for (db_int32 i = 0; i < ENTRY_SIZE; i++) {
        m_free_lists[i].detach();
    }
    
}



void* mem_pool_t::alloc_page(db_size size)
{
    assert(MIN_PAGE_SIZE <= size && size <= MAX_PAGE_SIZE);    
    db_int32 level = calc_power_of_2(size) - MIN_PAGE_BITS;
    return alloc_inner(level);
}



void* mem_pool_t::alloc_inner(db_uint32 level)
{
    db_byte* page = NULL;    
    db_uint32 max_level = level;
    while (max_level <= MAX_LEVEL) {    
        page = (db_byte*)m_free_lists[max_level].pop_head();
        if (page != NULL) break;
        max_level++;
    }

    if (page == NULL) return NULL;
    
    db_uint32 page_size = MIN_PAGE_SIZE << max_level;
    for (db_uint32 n = max_level; n > level; n--) {
        set_bitmap(page, n);
        m_free_lists[n-1].link(page);
        page_size = page_size >> 1;
        page += page_size;
    }
    
    set_bitmap(page, level);    
    return page;
}


void mem_pool_t::free_page(void* ptr)
{    
    db_uint32 level = get_page_level(ptr);    
    free_inner((db_byte*)ptr, level);
    return;
}


void mem_pool_t::free_inner(db_byte* ptr, db_uint32 level)
{
    clean_bitmap(ptr, level);

    // 如果是已经最大页面了，直接放到链表即可
    if (level == MAX_LEVEL) {
        m_free_lists[level].link(ptr);
        return;
    }

    db_byte* buddy_ptr = get_buddy(ptr, level);   
    
    // 如果buddy使用中，则直接将释放页回归到空闲链表
    if (get_bitmap(buddy_ptr, level)) {
        m_free_lists[level].link(ptr);        
        return;
    }

    // when buddy is free
    m_free_lists[level].unlink(buddy_ptr);
    db_byte* parent_ptr = ptr < buddy_ptr ? ptr : buddy_ptr;
    free_inner(parent_ptr, level + 1);
    return;
}

