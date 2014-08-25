// column.h by scott.zgeng@gmail.com 2014.08.20


#ifndef  __COLUMN_H__
#define  __COLUMN_H__


#include <assert.h>
#include <string.h>

#include "define.h"
#include "variant.h"
#include "mem_pool.h"



struct segment_item_t
{
    static const db_uint32 SEGMENT_HIGH_BITS = 10;
    static const db_uint32 SEGMENT_LOW_BITS = 10;
    static const db_uint32 SEGMENT_LOW_SIZE = 1 << SEGMENT_LOW_BITS;
    static const db_uint32 SEGMENT_BITS = SEGMENT_LOW_BITS + SEGMENT_HIGH_BITS;
    static const db_uint32 SEGMENT_LOW_MASK = (1 << SEGMENT_LOW_BITS) - 1;

    static const db_uint32 MAX_SEGMENT_COUNT = 1 << SEGMENT_BITS;
    static const db_uint32 MAX_SEGMENT_ID = MAX_SEGMENT_COUNT - 1;

    segment_item_t(db_uint32 segment_id) {
        s_high = segment_id >> SEGMENT_LOW_BITS;
        s_low = segment_id & SEGMENT_LOW_MASK;
    }

    db_uint16 s_high;   // segment high bits
    db_uint16 s_low;    // segment low bits
};



struct row_item_t
{
    static const db_uint32 SEGMENT_HIGH_BITS = segment_item_t::SEGMENT_HIGH_BITS;
    static const db_uint32 SEGMENT_LOW_BITS = segment_item_t::SEGMENT_LOW_BITS;
    static const db_uint32 ROW_OFFSET_BITS = 10;

    static const db_uint32 SEGMENT_LOW_MASK = (1 << (SEGMENT_LOW_BITS + ROW_OFFSET_BITS)) - (1 << ROW_OFFSET_BITS);
    static const db_uint32 ROW_OFFSET_MASK = (1 << ROW_OFFSET_BITS) - 1;

    static const db_uint32 MAX_ROW_COUNT = 1 << (SEGMENT_HIGH_BITS + SEGMENT_LOW_BITS + ROW_OFFSET_BITS);
    static const rowid_t MAX_ROWID = MAX_ROW_COUNT - 1;

    row_item_t(rowid_t rowid) {
        COMPILE_ASSERT(SEGMENT_SIZE == (1 << ROW_OFFSET_BITS));

        s_high = rowid >> (SEGMENT_LOW_BITS + ROW_OFFSET_BITS);
        s_low = (rowid & SEGMENT_LOW_MASK) >> ROW_OFFSET_BITS;
        offset = rowid & ROW_OFFSET_MASK;    
    }

    db_uint16 s_high;   // segment high bits
    db_uint16 s_low;    // segment low bits
    db_uint16 offset;   // offset in the segment  
};


class column_base_t
{
public:
    static column_base_t* create_column(data_type_t type);

public:
    virtual ~column_base_t() {}

    virtual result_t init(mem_pool* pool, db_int32 data_len) = 0;
    virtual void uninit() = 0;
    virtual result_t insert(void* values, db_int32 num) = 0;
    virtual data_type_t data_type() const = 0;
    virtual void* get_segment(db_uint32 segment_id) = 0;
    virtual db_int32 get_segment_row_count(db_uint32 segment_id) = 0;
    virtual void batch_get_rows_value(rowid_t* rows, db_int32 count, void* ptr) = 0;    
    virtual db_uint32 get_segment_count() = 0;
    virtual db_uint32 get_row_count() = 0;
};


// TODO(scott.zgeng): 部分函数可以优化成非虚函数
template<typename T>
class column_t : public column_base_t
{
public:
    column_t() {        
        m_mem_pool = NULL;
        m_data_len = 0;
        m_row_count = 0;
        m_capacity = 0;
        memset(m_address, 0, sizeof(m_address));      
    }

    virtual ~column_t() {
        uninit();
    }

public:
    virtual result_t init(mem_pool* pool, db_int32 data_len) {
        assert(pool != NULL);
        
        m_mem_pool = pool;
        m_data_len = data_len;
        m_row_count = 0;
        m_capacity = 0;
        m_segment_count = 0;
        return RT_SUCCEEDED;
    }


    virtual void uninit() {
        // TODO(scott.zgeng): 增加内存清除的分支
    }


    // NOTE(scott.zgeng): 只支持最大一次增加SEGMENT_SIZE的数据，如果超过需要循环调用该接口
    virtual result_t insert(void* values, db_int32 num) {
        assert(num <= SEGMENT_SIZE);

        if (!ensure_capacity(m_row_count + num))
            return RT_FAILED;

        db_int32 offset = m_row_count & row_item_t::ROW_OFFSET_MASK;        
        db_uint32 last_segment_id = m_segment_count - 1;

        if (offset + num > SEGMENT_SIZE) {
            db_int32 part_num = SEGMENT_SIZE - offset;

            T* segment = (T*)get_segment(last_segment_id - 1);
            memcpy(segment + offset, values, sizeof(T)* part_num);

            T* new_segment = (T*)get_segment(last_segment_id);
            memcpy(new_segment, (T*)values + part_num, sizeof(T)* (num - part_num));

        } else {
            T* segment = (T*)get_segment(last_segment_id);
            memcpy(segment + offset, values, sizeof(T)* num);
        }

        m_row_count += num;
        return RT_SUCCEEDED;
    }

    virtual data_type_t data_type() const {
        variant_type<T> type;
        return type();
    }

    virtual void* get_segment(db_uint32 segment_id) {
        segment_item_t s(segment_id);
        assert(m_address[s.s_high] && m_address[s.s_high][s.s_low]);
        return m_address[s.s_high][s.s_low];
    }

    virtual db_int32 get_segment_row_count(db_uint32 segment_id) {
        if (segment_id + 1 < m_segment_count)
            return SEGMENT_SIZE;
        else
            return (m_row_count & row_item_t::ROW_OFFSET_MASK);
    }

    virtual void batch_get_rows_value(rowid_t* rows, db_int32 count, void* ptr) {
        assert(count <= SEGMENT_SIZE);

        T* val = (T*)ptr;
        for (db_int32 i = 0; i < count; i++) {
            val[i] = get_row_value(rows[i]);
        }        
    }

    virtual db_uint32 get_segment_count() {
        return m_segment_count;
    }

    virtual db_uint32 get_row_count() {
        return m_row_count;
    }
    

private:
    const T& get_row_value(rowid_t rowid) {
        row_item_t r(rowid);
        return m_address[r.s_high][r.s_low][r.offset];
    }

    bool ensure_capacity(db_uint32 size) {
        assert(size <= m_capacity + SEGMENT_SIZE);

        if (size <= m_capacity)
            return true;
        
        segment_item_t s(m_segment_count);

        if (m_address[s.s_high] == NULL) {
            T** new_low_segment = (T**)m_mem_pool->alloc_page(sizeof(void*)* SEGMENT_SIZE);
            if (new_low_segment == NULL) 
                return false;            

            memset(new_low_segment, 0, sizeof(void*)* segment_item_t::SEGMENT_LOW_SIZE);
            m_address[s.s_high] = new_low_segment;
        }
        
        assert(m_address[s.s_high][s.s_low] == NULL);
        T* new_page = (T*)m_mem_pool->alloc_page(sizeof(T)* SEGMENT_SIZE);
        if (new_page == NULL)
            return false;

        m_address[s.s_high][s.s_low] = new_page;        
        m_capacity += SEGMENT_SIZE;
        m_segment_count++;
        return true;
    }


private: 
    static const db_uint32 ENTRY_SIZE = (1 << row_item_t::SEGMENT_HIGH_BITS);
    
    mem_pool* m_mem_pool;

    db_uint32 m_data_len;
    db_uint32 m_row_count;
    db_uint32 m_capacity;
    db_uint32 m_segment_count;
    
    T** m_address[ENTRY_SIZE];
};


#endif //__COLUMN_H__


