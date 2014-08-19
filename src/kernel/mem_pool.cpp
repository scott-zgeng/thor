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


mem_pool::mem_pool()
{
    m_bitmap = NULL;    
    m_data = NULL;
    m_bitmap_num = 0;
    memset(m_free_entry, 0, sizeof(m_free_entry));
}

mem_pool::~mem_pool()
{
    uninit();
}


result_t mem_pool::init(db_size pool_size)
{
    IF_RETURN_FAILED(pool_size % MAX_PAGE_SIZE != 0);
    
    // ������ MAX_PAGE �ܴ�Ŷ��ٸ�
    db_size page_num = pool_size / MAX_PAGE_SIZE;
    IF_RETURN_FAILED(page_num < 1);
    
    // һ�� MAX_PAGE ��Ӧһ��BITMAP
    db_size bitmap_size = page_num * BITMAP_SIZE;
    m_bitmap = (db_byte*)::malloc(bitmap_size);
    IF_RETURN_FAILED(m_bitmap == NULL);

    m_data = (db_byte*)::malloc(pool_size);
    // TODO(scott.zgeng@gmail.com): �ȼ�д�������Ϊ�Զ�ָ��   
    if (m_data == NULL) {
        ::free(m_bitmap);
        m_bitmap = NULL;
        return RT_FAILED;
    }

    m_bitmap_num = page_num;
    
    // ���BITMAP״̬
    memset(m_bitmap, 0, bitmap_size);

    // ��������PAGE����������
    page_head_t* entry = m_free_entry + MAX_LEVEL;
    page_head_t* curr = (page_head_t*)m_data;
    
    curr->perv = entry;
    entry->next = curr;    
    
    page_head_t* prev;

    for (db_byte* ptr = m_data + MAX_PAGE_SIZE; ptr < m_data + pool_size; ptr += MAX_PAGE_SIZE) {
        curr = (page_head_t*)ptr;
        prev = (page_head_t*)(ptr - MAX_PAGE_SIZE);

        curr->perv = prev;
        prev->next = curr;
    }

    curr = (page_head_t*)(m_data + pool_size - MAX_PAGE_SIZE);
    curr->next = NULL;

    return RT_SUCCEEDED;
}

void mem_pool::uninit()
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

    memset(m_free_entry, 0, sizeof(m_free_entry));
}



void* mem_pool::alloc_page(db_size size)
{
    assert(MIN_PAGE_SIZE <= size && size <= MAX_PAGE_SIZE);    
    db_int32 level = calc_power_of_2(size) - MIN_PAGE_BITS;
    return alloc_inner(level);
}


mem_pool::page_head_t* mem_pool::alloc_inner(db_uint32 level)
{
    if (level > MAX_LEVEL) return NULL;

    // ��������������п���PAGE����ֱ�ӽ����еķ����ȥ    
    page_head_t* temp = m_free_entry[level].next;
    if (temp != NULL) {
        unlink_from_pool(temp); 
        set_state(temp, level, MEM_ALLOC);
        return temp;
    } 

    // �����Ӧ�Ŀ��������ǿյģ�����Ҫ����һ������һ����PAGE
    page_head_t* parent = alloc_inner(level + 1);
    if (parent == NULL) return NULL;

    // ��ǰ��һ��PAGE�ҵ���������
    link_to_pool(parent, level);
    
    // ������һ��page����
    temp = (page_head_t*)((db_byte*)parent + page_size(level));
    set_state(temp, level, MEM_ALLOC);
    return temp;
}


void mem_pool::free_page(void* ptr)
{    
    db_uint32 level = get_page_level(ptr);    
    free_inner((db_byte*)ptr, level);
    return;
}


void mem_pool::free_inner(db_byte* ptr, db_uint32 level)
{
    set_state(ptr, level, MEM_FREE);

    // ������Ѿ����ҳ���ˣ�ֱ�ӷŵ�������
    if (level == MAX_LEVEL) {
        link_to_pool(ptr, level);        
        return;
    }

    db_byte* buddy_ptr = get_buddy(ptr, level);
    state_t state = get_state(buddy_ptr, level);

    if (state != MEM_FREE) {
        link_to_pool(ptr, level);        
        return;
    }

    // buddy is free
    unlink_from_pool(buddy_ptr);
    db_byte* parent_ptr = ptr < buddy_ptr ? ptr : buddy_ptr;
    free_inner(parent_ptr, level + 1);
    return;
}

