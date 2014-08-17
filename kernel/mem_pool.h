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
    
    static const db_uint32 MAX_LEVEL = 7;    

    static const db_uint32 MIN_PAGE_BITS = 7;
    static const db_uint32 MIN_PAGE_SIZE = (1 << MIN_PAGE_BITS);
    static const db_uint32 MAX_PAGE_SIZE = (MIN_PAGE_SIZE << MAX_LEVEL);
    
    static const db_uint32 PAGE_MASK = (1 << MAX_LEVEL) - 1;

    // 每个MIN_PAGE占用2bit来表示这个页面的状态，用MIN_PAGE组成的完全二叉树需要 (2 * n - 1) 个状态信息
    // 所以相当于每个MIN_PAGE占用4个BIT
    static const db_uint32 BITMAP_UNIT_SIZE = (1 << MAX_LEVEL) / 8 * 2 * 2;

    static const db_uint32 ENTRY_SIZE = (MAX_LEVEL + 1);

    enum state_t {
        MEM_FREE = 0,
        MEM_ALLOC = 1,
        MEM_SPLIT = 2,
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
    struct page_head_t;

    void* alloc_inner(db_uint32 pow);
    void free_inner(void* ptr, db_uint32 pow);

    void set_state(void* ptr, db_uint32 pow, state_t state);

    db_uint32 get_page_no(void* ptr) {
        return ((db_byte*)ptr - m_data) >> MIN_BIT;
    }

    db_byte* get_bitmap_base(db_uint32 page_idx) {        
        return m_bitmap + (page_idx >> PAGE_BIT) * BITMAP_UNIT_SIZE;        
    }

    void combine(void* ptr, db_uint32 pow);


    void unlink_from_pool(void* ptr) {
        page_head_t* temp = (page_head_t*)ptr;

        temp->perv->next = temp->next;
        if (temp->next != NULL) {
            temp->next->perv = temp->perv;
        }
    }

    void link_to_pool(void* ptr, page_head_t* first) {        
        page_head_t* temp = (page_head_t*)ptr;

        temp->next = first->next;
        temp->perv = first;
        if (first->next != NULL) {
            first->next->perv = temp;
        }
        first->next = temp;     
    }


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
