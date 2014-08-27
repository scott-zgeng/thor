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

    void* begin() {
        return m_entry.next;
    }

    db_uint32 size() {
        return m_count;
    }

protected:
    page_head_t m_entry;
    db_uint32 m_count;
};




// TODO(scott.zgeng): 
//  Ŀǰ����Ҫ��ַ����
//  ������������⣬�ɲο�������Ƕ��ѭ�������������ΪLEVEL��BITMAP�����Ż�����ʱCOMBINE

class mem_pool_t
{
public:     
    static const db_uint32 MAX_LEVEL = 4;    

    static const db_uint32 MIN_PAGE_BITS = 10;
    static const db_uint32 MIN_PAGE_SIZE = (1 << MIN_PAGE_BITS);
    static const db_uint32 MAX_PAGE_BITS = (MIN_PAGE_BITS + MAX_LEVEL);
    static const db_uint32 MAX_PAGE_SIZE = (1 << MAX_PAGE_BITS);
    
    static const db_uint32 PAGE_MASK = (1 << MAX_LEVEL) - 1;

    // ÿһ����Сҳ��ռ�� 1bit ����ʾ���ҳ���״̬
    // ����Сҳ����ɺͺϲ������ҳ���״̬��������һ����ȫ��������ʾ��������Ҫ(2 * n - 1) ��״̬��Ϣ
    static const db_uint32 BITMAP_BYTES = (1 << MAX_LEVEL) * 2 / 8;

    static const db_uint32 ENTRY_SIZE = (MAX_LEVEL + 1);

public:
    mem_pool_t();
    ~mem_pool_t();

public:
    result_t init(db_size pool_size);
    void uninit();

    void* alloc_page(db_size size);
    void free_page(void* ptr);

private:
    void* alloc_inner(db_uint32 level);
    void free_inner(db_byte* ptr, db_uint32 level);

    // �ҵ�PAGE��ӦBITMAP����ڵ�ַ
    db_byte* get_bitmap_base(void* ptr) {
        db_uint32 offset = (db_byte*)ptr - m_data;
        return m_bitmap + (offset >> MAX_PAGE_BITS) * BITMAP_BYTES;
    }

    // ����PAGE�ڶ�������ʵ��λ��
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

    void mem_pool_t::set_bitmap(void* ptr, db_uint32 level) {
        db_byte* base = get_bitmap_base(ptr);
        db_uint32 index = get_bitmap_index(ptr, level);
        bitmap_t::set(base, index);        
    }


    void mem_pool_t::clean_bitmap(void* ptr, db_uint32 level) {
        db_byte* base = get_bitmap_base(ptr);
        db_uint32 index = get_bitmap_index(ptr, level);
        bitmap_t::clean(base, index);                
    }

    db_bool mem_pool_t::get_bitmap(void* ptr, db_uint32 level) {
        db_byte* base = get_bitmap_base(ptr);
        db_uint32 index = get_bitmap_index(ptr, level);
        return bitmap_t::get(base, index);        
    }


    // �ҳ�ָ������ĳ���
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
    void alloc_memory(db_int32 size, mem_handle_t& handle);

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
        //  ֻ�ͷ�����ջ�Ŀռ䣬������ǣ��п����ǿգ�Ҳ������Ӧ���Լ��Ŀռ䣬���ں����÷�        
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


class mem_region_t
{
public:
    mem_region_t(mem_pool_t* pool);
    ~mem_region_t();

public:
    void* alloc(db_size size);
    void release();

private:
    //struct page_head_t {
    //    page_head_t
    //}

};



#endif //__MEM_POOL_H__

