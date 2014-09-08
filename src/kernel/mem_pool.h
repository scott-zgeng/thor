// mem_pool.h by scott.zgeng@gmail.com 2014.08.15


#ifndef  __MEM_POOL_H__
#define  __MEM_POOL_H__



#include "define.h"
#include "bitmap.h"



class mem_dlist
{
private:
    struct page_head_t {
        page_head_t* prev;
        page_head_t* next;
    };
public:
    static const db_uint32 HEAD_SIZE = sizeof(page_head_t);

    class iterator
    {
    public:
        iterator() {
            m_page = NULL;
        }

        iterator(mem_dlist* list) {
            init(list);
        }

        void init(mem_dlist* list) {
            m_page = &list->m_entry;
        }

        void* next() {
            m_page = m_page->next;
            if (m_page == NULL)
                return NULL;

            return (db_byte*)m_page + HEAD_SIZE;
        }

    private:
        page_head_t* m_page;
    };


public:
    mem_dlist() {
        detach();        
    }

    ~mem_dlist() {        
    }

    void link(void* ptr) {
        assert(ptr != NULL);
        page_head_t* temp = (page_head_t*)ptr;

        temp->next = m_entry.next;
        temp->prev = &m_entry;
        if (m_entry.next != NULL) {
            m_entry.next->prev = temp;
        }
        m_entry.next = temp;
        m_count++;
    }

    void unlink(void* ptr) {
        assert(ptr != NULL);
        page_head_t* temp = (page_head_t*)ptr;

        temp->prev->next = temp->next;
        if (temp->next != NULL) {
            temp->next->prev = temp->prev;
        }
        m_count--;
    }

    void detach() {
        m_count = 0;
        m_entry.next = NULL;
        m_entry.prev = NULL;
    }

    void* pop_head() {        
        void* p = m_entry.next;
        if (p != NULL) {
            unlink(p);
        }
        return p;
    }

    void push_head(void* ptr) {
        return link(ptr);
    }

    db_uint32 size() const {
        return m_count;
    }

    void* page_data(void* ptr) const {
        return (db_byte*)ptr + sizeof(page_head_t);
    }

protected:
    page_head_t m_entry;
    db_uint32 m_count;
    
};




// TODO(scott.zgeng): 
//  目前还需要地址对齐
//  如果有性能问题，可参考：消除嵌套循环，传入参数改为LEVEL，BITMAP操作优化，延时COMBINE

class mem_pool_t
{
public:     
    static const db_uint32 MAX_LEVEL = 4;    

    static const db_uint32 MIN_PAGE_BITS = 10;
    static const db_uint32 MIN_PAGE_SIZE = (1 << MIN_PAGE_BITS);
    static const db_uint32 MAX_PAGE_BITS = (MIN_PAGE_BITS + MAX_LEVEL);
    static const db_uint32 MAX_PAGE_SIZE = (1 << MAX_PAGE_BITS);
    
    static const db_uint32 PAGE_MASK = (1 << MAX_LEVEL) - 1;

    // 每一个最小页面占用 1bit 来表示这个页面的状态
    // 用最小页面组成和合并的最大页面的状态，可以用一个完全二叉树表示，所以需要(2 * n - 1) 个状态信息
    static const db_uint32 BITMAP_BYTES = (1 << MAX_LEVEL) * 2 / 8;

    static const db_uint32 ENTRY_SIZE = (MAX_LEVEL + 1);

public:
    mem_pool_t();
    ~mem_pool_t();

public:
    result_t init(db_size pool_size);
    void uninit();

    void* alloc_page(db_size size);
    void* alloc_max_page();
    void free_page(void* ptr);
    

private:
    void* alloc_inner(db_uint32 level);
    void free_inner(db_byte* ptr, db_uint32 level);

    // 找到PAGE对应BITMAP的入口地址
    db_byte* get_bitmap_base(void* ptr) {
        db_uint32 offset = (db_byte*)ptr - m_data;
        return m_bitmap + (offset >> MAX_PAGE_BITS) * BITMAP_BYTES;
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

    void set_bitmap(void* ptr, db_uint32 level) {
        db_byte* base = get_bitmap_base(ptr);
        db_uint32 index = get_bitmap_index(ptr, level);
        bitmap_t::set(base, index);        
    }


    void clean_bitmap(void* ptr, db_uint32 level) {
        db_byte* base = get_bitmap_base(ptr);
        db_uint32 index = get_bitmap_index(ptr, level);
        bitmap_t::clean(base, index);                
    }

    db_bool get_bitmap(void* ptr, db_uint32 level) {
        db_byte* base = get_bitmap_base(ptr);
        db_uint32 index = get_bitmap_index(ptr, level);
        return bitmap_t::get(base, index);        
    }


    // 找出指针申请的长度
    db_uint32 get_page_level(void* ptr) {
        db_uint32 level = 0;
        db_byte* base = get_bitmap_base(ptr);
        db_uint32 index = get_bitmap_index(ptr, 0);

        while (true) {
            db_bool is_used = bitmap_t::get(base, index);            
            if (is_used) break;
            index = index >> 1;
            level++;
        }
        assert(level <= MAX_LEVEL);
        return level;
    }

private:
    db_uint32 m_bitmap_num;
    db_byte* m_bitmap;
    db_byte* m_data;
    mem_dlist m_free_lists[ENTRY_SIZE];
};




class mem_stack_t
{
public:
    friend class mem_handle_t;

    mem_stack_t() {
        m_begin = m_buffer;
        m_end = m_begin + sizeof(m_buffer);
        m_position = m_begin;
    }

    mem_stack_t::~mem_stack_t() {
    }

public:
    // note(scott.zgeng): mem_handle_t放在参数中主要是为了方便传递参数
    void alloc_memory(db_int32 size, mem_handle_t& handle);
    void spin_memory(mem_handle_t& handle);

    void reset() {
        m_position = m_begin;
    }

private:
    db_int8* m_begin;
    db_int8* m_end;
    db_int8* m_position;

    db_int8 m_buffer[SEGMENT_SIZE * 1024 * 2];
};


class mem_handle_t
{
public:
    mem_handle_t() {
        init(NULL, NULL);
    }

    mem_handle_t(mem_stack_t* ctx, void* data) {
        init(ctx, data);
    }

    ~mem_handle_t() {
        uninit();
    }

    void init(mem_stack_t* ctx, void* data) {
        //DB_TRACE("mem_handle init, %p", data);
        m_ctx = ctx;
        m_data = (db_int8*)data;
    }

    void uninit() {
        // NOTE(scott.zgeng): 
        //  只释放属于栈的空间，如果不是，有可能是空，也可能是应用自己的空间，属于合理用法        
        if (m_data >= m_ctx->m_buffer && m_data < m_ctx->m_end) {
            //DB_TRACE("mem_handle uninit, %p", m_data);
            assert(m_data > m_ctx->m_position);
            m_ctx->m_position = m_data;            
        }        
    }

    void* transfer() {
        db_int8* data = m_data;
        m_data = NULL;
        return data;
    }

    void* ptr() const {
        return m_data;
    }

private:
    mem_stack_t* m_ctx;
    db_int8* m_data;

};


// NOTE(scott.zgeng): 因为使用了mem_handle_t实现，所以放在此处
inline void mem_stack_t::alloc_memory(db_int32 size, mem_handle_t& handle)
{
    // NOTE(scott.zgeng): BUFFER的大小完全可以根据已知的表达式，得到最大需要空间大小
    assert(m_position + size <= m_end);

    handle.init(this, m_position);
    m_position += size;
}

// NOTE(scott.zgeng): 因为使用了mem_handle_t实现，所以放在此处
inline void mem_stack_t::spin_memory(mem_handle_t& handle)
{
    handle.init(this, m_position);    
}





class mem_region_t
{
public:
    static const db_uint32 MAX_ALLOC_SIZE = mem_pool_t::MAX_PAGE_SIZE - mem_dlist::HEAD_SIZE;

public:
    class iterator {
    public:
        iterator() {
        }

        iterator(mem_region_t* region) {
            m_iterator.init(&region->m_list);
        }

        void init(mem_region_t* region) {
            m_iterator.init(&region->m_list);
        }

        void* next() {
            return m_iterator.next();            
        }
    private:
        mem_dlist::iterator m_iterator;
    };


public:
    mem_region_t() {
        m_capacity = 0;
        m_pool = NULL;
        m_data = NULL;
    }

    ~mem_region_t() {
        release();
    }

public:
    void init(mem_pool_t* pool) {
        m_pool = pool;
    }

    void* alloc(db_size n) {
        if (!ensure_capacity(n))
            return NULL;

        void* ptr = m_data;
        m_data += n;
        m_capacity -= n;
        return ptr;
    }


    void release() {

        while (true) {
            void * ptr = m_list.pop_head();
            if (ptr == NULL) break;        
            m_pool->free_page(ptr);
        }

        m_capacity = 0;        
        m_data = NULL;
    }

 

private:
    bool ensure_capacity(db_uint32 n) {
        assert(n <= MAX_ALLOC_SIZE);

        if (n <= m_capacity)
            return true;

        void* page = m_pool->alloc_max_page();
        if UNLIKELY(page == NULL)
            return false;

        m_list.push_head(page);
        m_data = (db_byte*)m_list.page_data(page);
        m_capacity = MAX_ALLOC_SIZE;       
        return true;
    }

private:
    mem_dlist m_list;    
    db_uint32 m_capacity;
    mem_pool_t* m_pool;
    db_byte* m_data;
};


class mem_row_table_t
{
public:
    class iterator
    {
    public:
        iterator() {
            m_len = 0;
            m_left_count = 0;
            m_row_count_per_page = 0;
            m_row_idx = 0;
            m_pos = NULL;
        }

        iterator(mem_row_table_t* table) {            
            init(table);
        }

        void init(mem_row_table_t* table) {
            m_iterator.init(&table->m_region);
            m_len = table->m_len;
            m_left_count = table->m_count;
            m_row_count_per_page = mem_region_t::MAX_ALLOC_SIZE / table->m_len;
            m_row_idx = m_row_count_per_page;
            m_pos = NULL;
        }

        void* next() {
            if (m_left_count == 0)
                return NULL;

            m_left_count--;
            if (m_row_idx < m_row_count_per_page) {
                m_pos += m_len;
                m_row_idx++;                

            } else {
                m_pos = (db_byte*)m_iterator.next();
                assert(m_pos != NULL);
                m_row_idx = 0;
            }

            return m_pos;
        }

    private:        
        mem_region_t::iterator m_iterator;
        db_uint32 m_len;
        db_uint32 m_left_count;
        db_uint32 m_row_count_per_page;
        db_uint32 m_row_idx;
        db_byte* m_pos;
    };

    friend class iterator;

    mem_row_table_t() {
        m_len = 0;
        m_count = 0;
    }

    void init(mem_pool_t* pool, db_uint32 len) {
        m_region.init(pool);
        m_len = len;
    }

    void* alloc() {
        void* p = m_region.alloc(m_len);
        if (p == NULL) return NULL;
        m_count++;
        return p;
    }

private:
    mem_region_t m_region;    
    db_uint32 m_len;
    db_uint32 m_count;
    
};


#endif //__MEM_POOL_H__


