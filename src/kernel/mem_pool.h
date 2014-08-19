// mem_pool.h by scott.zgeng@gmail.com 2014.08.15


#ifndef  __MEM_POOL_H__
#define  __MEM_POOL_H__



#include "define.h"
#include "bitmap.h"


// TODO(scott.zgeng): 
//  目前还需要地址对齐
//  如果有性能问题，可参考：消除嵌套循环，传入参数改为LEVEL，BITMAP操作优化，延时COMBINE

class mem_pool
{
public:     
    static const db_uint32 MAX_LEVEL = 4;    

    static const db_uint32 MIN_PAGE_BITS = 10;
    static const db_uint32 MIN_PAGE_SIZE = (1 << MIN_PAGE_BITS);
    static const db_uint32 MAX_PAGE_BITS = (MIN_PAGE_BITS + MAX_LEVEL);
    static const db_uint32 MAX_PAGE_SIZE = (1 << MAX_PAGE_BITS);
    
    static const db_uint32 PAGE_MASK = (1 << MAX_LEVEL) - 1;

    // 每一个最小页面占用 1bit 来表示这个页面的状态
    static const db_uint32 BITMAP_BIT = 1;
    // 用最小页面组成和合并的最大页面的状态，可以用一个完全二叉树表示，所以需要(2 * n - 1) 个状态信息
    static const db_uint32 BITMAP_SIZE = (1 << MAX_LEVEL) * 2 * BITMAP_BIT / 8;

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

    void* alloc_page(db_size size);
    void free_page(void* ptr);

private:
    struct page_head_t {
        page_head_t* perv;
        page_head_t* next;
    };

    page_head_t* alloc_inner(db_uint32 level);

    void free_inner(db_byte* ptr, db_uint32 level);

    // 找到PAGE对应BITMAP的入口地址
    db_byte* get_bitmap_base(void* ptr) {
        db_uint32 offset = (db_byte*)ptr - m_data;
        return m_bitmap + (offset >> MAX_PAGE_BITS) * BITMAP_SIZE;
    }

    // 计算PAGE在二叉树的实际位置
    db_uint32 get_bitmap_index(void* ptr, db_uint32 level) {
        db_uint32 offset = (db_byte*)ptr - m_data;
        db_uint32 idx = (offset >> MIN_PAGE_BITS) & PAGE_MASK;  // MIN PAGE IDX
        return  (idx >> level) + (1 << (MAX_LEVEL - level));
    }

    db_uint32 page_size(db_uint32 level) {
        return (MIN_PAGE_SIZE << level);
    }

    db_byte* get_buddy(db_byte* ptr, db_uint32 level) {
        db_uint32 index = get_bitmap_index(ptr, level);
        return (index & 0x01) ? ptr - page_size(level) : ptr + page_size(level);        
    }

    void mem_pool::set_state(void* ptr, db_uint32 level, state_t state) {
        db_byte* base = get_bitmap_base(ptr);
        db_uint32 index = get_bitmap_index(ptr, level);
        bitmap<BITMAP_BIT>::set(base, index, state);
    }


    state_t mem_pool::get_state(void* ptr, db_uint32 level) {
        db_byte* base = get_bitmap_base(ptr);
        db_uint32 index = get_bitmap_index(ptr, level);
        return (state_t)bitmap<BITMAP_BIT>::get(base, index);
    }


    // 找出指针申请的长度
    db_uint32 get_page_level(void* ptr) {
        db_uint32 level = 0;
        db_byte* base = get_bitmap_base(ptr);
        db_uint32 index = get_bitmap_index(ptr, 0);

        while (true) {
            state_t state = (state_t)bitmap<BITMAP_BIT>::get(base, index);
            if (state == MEM_ALLOC)
                break;
            index = index >> 1;
            level++;
        }
        assert(level <= MAX_LEVEL);
        return level;
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
    db_uint32 m_bitmap_num;
    db_byte* m_bitmap;
    db_byte* m_data;
    page_head_t m_free_entry[ENTRY_SIZE];
};


#endif //__MEM_POOL_H__
