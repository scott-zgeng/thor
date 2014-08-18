// mem_pool.h by scott.zgeng@gmail.com 2014.08.15


#ifndef  __MEM_POOL_H__
#define  __MEM_POOL_H__



#include "define.h"
#include "bitmap.h"


class mem_pool
{
public:     
    static const db_uint32 MAX_LEVEL = 4;    

    static const db_uint32 MIN_PAGE_BITS = 10;
    static const db_uint32 MIN_PAGE_SIZE = (1 << MIN_PAGE_BITS);
    static const db_uint32 MAX_PAGE_SIZE = (MIN_PAGE_SIZE << MAX_LEVEL);
    
    static const db_uint32 PAGE_MASK = (1 << MAX_LEVEL) - 1;

    // 每个MIN_PAGE占用1bit来表示这个页面的状态，用MIN_PAGE组成的完全二叉树需要 (2 * n - 1) 个状态信息
    // 所以相当于每个MIN_PAGE占用2个BIT

    static const db_uint32 BITMAP_BIT = 1;
    static const db_uint32 BITMAP_UNIT_SIZE = (1 << MAX_LEVEL) * 2 * BITMAP_BIT / 8;

    static const db_uint32 ENTRY_SIZE = (MAX_LEVEL + 1);

    enum state_t {
        MEM_FREE = 0,
        MEM_ALLOC = 1,        
    };

public:
    mem_pool();
    ~mem_pool();

public:
    result_t init(db_size pool_size);
    void uninit();

    void* alloc(db_size size);
    void free(void* ptr);

private:
    struct page_head_t {
        page_head_t* perv;
        page_head_t* next;
    };

    page_head_t* alloc_inner(db_uint32 level);

    void free_inner(db_byte* ptr, db_uint32 level);


    db_byte* get_buddy(db_byte* ptr, db_uint32 level) {
        db_uint32 page_no = get_page_no(ptr);

        db_uint32 page_idx = page_no & PAGE_MASK;

        db_uint32 tree_idx = (page_idx >> level) + (1 << (MAX_LEVEL - level));

        if (tree_idx & 0x01) {
            return ptr - (1 << level) * MIN_PAGE_SIZE;
        } else {
            return ptr + (1 << level) * MIN_PAGE_SIZE;
        }

    }

    
    void mem_pool::set_state(void* ptr, db_uint32 level, state_t state) {

        // 计算最小PAGE所对应的下标
        db_uint32 page_no = get_page_no(ptr);

        // 找到PAGE对应BITMAP的入口地址
        db_byte* base = get_bitmap_base(page_no);

        db_uint32 page_idx = page_no & PAGE_MASK;

        // 计算在二叉树的实际位置
        db_uint32 tree_idx = (page_idx >> level) + (1 << (MAX_LEVEL - level));

        bitmap<BITMAP_BIT>::set(base, tree_idx, state);
    }


    state_t mem_pool::get_state(void* ptr, db_uint32 level) {

        // 计算最小PAGE所对应的下标
        db_uint32 page_no = get_page_no(ptr);

        // 找到PAGE对应BITMAP的入口地址
        db_byte* base = get_bitmap_base(page_no);

        db_uint32 page_idx = page_no & PAGE_MASK;

        // 计算在二叉树的实际位置
        db_uint32 tree_idx = (page_idx >> level) + (1 << (MAX_LEVEL - level));

        return (state_t)bitmap<BITMAP_BIT>::get(base, tree_idx);
    }



    db_uint32 get_page_no(void* ptr) {
        return ((db_byte*)ptr - m_data) >> MIN_PAGE_BITS;
    }

    db_byte* get_bitmap_base(db_uint32 page_no) {        
        return m_bitmap + (page_no >> MAX_LEVEL) * BITMAP_UNIT_SIZE;
    }



    void unlink_from_pool(void* ptr) {
        page_head_t* temp = (page_head_t*)ptr;

        temp->perv->next = temp->next;
        if (temp->next != NULL) {
            temp->next->perv = temp->perv;
        }
    }

    void link_to_pool(void* ptr, db_uint32 level) {
        page_head_t* entry = m_free_entry + level;

        page_head_t* temp = (page_head_t*)ptr;

        temp->next = entry->next;
        temp->perv = entry;
        if (entry->next != NULL) {
            entry->next->perv = temp;
        }
        entry->next = temp;     
    }


private:
    db_byte* m_bitmap;
    db_byte* m_data;
    page_head_t m_free_entry[ENTRY_SIZE];
};


#endif //__MEM_POOL_H__
