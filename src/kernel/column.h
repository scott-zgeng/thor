// column.h by scott.zgeng@gmail.com 2014.08.20


#ifndef  __COLUMN_H__
#define  __COLUMN_H__


#include <assert.h>

#include "define.h"
#include "variant.h"
#include "column_table.h"
#include "mem_pool.h"


struct row_item_t
{
    static const db_uint32 ROW_HIGH_BITS = 10;
    static const db_uint32 ROW_MIDDLE_BITS = 10;
    static const db_uint32 ROW_LOW_BITS = 10;

    static const db_uint32 ROW_MIDDLE_MASK = (1 << (ROW_MIDDLE_BITS + ROW_LOW_BITS)) - (1 << ROW_LOW_BITS);
    static const db_uint32 ROW_LOW_MASK = (1 << ROW_LOW_BITS) - 1;

    static const db_uint32 MAX_ROW_COUNT = 1 << (ROW_HIGH_BITS + ROW_MIDDLE_BITS + ROW_LOW_BITS);
    static const rowid_t MAX_ROWID = MAX_ROW_COUNT - 1;

    row_item_t(rowid_t rowid) {
        COMPILE_ASSERT(SEGMENT_SIZE == (1 << ROW_LOW_BITS));

        high = rowid >> (ROW_MIDDLE_BITS + ROW_LOW_BITS);
        middle = (rowid & ROW_MIDDLE_MASK) >> ROW_LOW_BITS;
        low = rowid & ROW_LOW_MASK;    
    }

    db_uint16 high;
    db_uint16 middle;
    db_uint16 low;    
};

struct segment_item_t
{
    static const db_uint32 SEGMENT_HIGH_BITS = row_item_t::ROW_HIGH_BITS;
    static const db_uint32 SEGMENT_LOW_BITS = row_item_t::ROW_MIDDLE_BITS;
    static const db_uint32 SEGMENT_LOW_MASK = (1 << SEGMENT_LOW_BITS) - 1;

    static const db_uint32 MAX_SEGMENT_COUNT = 1 << (SEGMENT_HIGH_BITS + SEGMENT_LOW_BITS);
    static const db_uint32 MAX_SEGMENT_ID = MAX_SEGMENT_COUNT - 1;

    segment_item_t(db_uint32 segment_id) {
        high = segment_id >> SEGMENT_LOW_BITS;
        low = segment_id & SEGMENT_LOW_MASK;        
    }

    db_uint16 high;
    db_uint16 low;
};


class column_base_t
{
public:
    static column_base_t* create_column(data_type_t type);

public:
    virtual ~column_base_t() {}

    virtual result_t init(column_table_t* table, db_int32 data_len) = 0;
    virtual void uninit() = 0;
    virtual result_t insert(void* values, db_int32 num) = 0;
    virtual data_type_t data_type() const = 0;
    virtual result_t get_segment(db_int32 segment_id, void** ptr, db_int32* count) = 0;

};



template<typename T>
class column_t : public column_base_t
{
public:
    column_t() {
        m_table = NULL;
        m_mem_pool = NULL;
        m_data_len = 0;
        m_count = 0;
        m_capacity = 0;
        memset(m_address, 0, sizeof(m_address));      
    }

    virtual ~column_t() {
        uninit();
    }

public:
    virtual result_t init(column_table_t* table, db_int32 data_len) {
        assert(table != NULL);

        m_table = table;
        m_mem_pool = m_table->get_mem_pool();
        m_data_len = data_len;
        m_count = 0;
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

        if (!ensure_capacity(m_count + num))
            return RT_FAILED;

        db_int32 offset = m_count & row_item_t::ROW_LOW_MASK;        
        db_uint32 last_segment_id = m_segment_count - 1;

        if (offset + num > SEGMENT_SIZE) {
            db_int32 part_num = SEGMENT_SIZE - offset;

            T* segment = get_segment(last_segment_id - 1);
            memcpy(segment + offset, values, sizeof(T)* part_num);

            T* new_segment = get_segment(last_segment_id);
            memcpy(new_segment, (T*)values + part_num, sizeof(T)* (num - part_num));

        } else {
            T* segment = get_segment(last_segment_id);
            memcpy(segment, values, sizeof(T)* num);
        }

        m_count += num;
        return RT_SUCCEEDED;
    }

    virtual data_type_t data_type() const {
        variant_type<T> type;
        return type();
    }

    virtual T* get_segment(db_int32 segment_id) {        
        segment_item_t s(segment_id);
        assert(m_address[s.high] && m_address[s.high][s.low]);
        return m_address[s.high][s.low];
    }

    virtual db_int32 get_segment_row_count(db_int32 segment_id) {
        if (segment_id + 1 < m_segment_count)
            return SEGMENT_SIZE;
        else
            return (m_count & row_item_t::ROW_LOW_MASK);
    }

private:

    bool ensure_capacity(db_uint32 size) {
        assert(size <= m_capacity + SEGMENT_SIZE);

        if (size <= m_capacity)
            return true;
        
        segment_item_t s(m_segment_count);

        if (m_address[s.high] == NULL) {
            m_address[s.high] = (T**)m_mem_pool->alloc_page(sizeof(void*)* SEGMENT_SIZE);
            if (m_address[s.high] == NULL) {
                return false;
            }
        }
        
        assert(m_address[s.high][s.low] == NULL);
        T* new_page = (T*)m_mem_pool->alloc_page(sizeof(T)* SEGMENT_SIZE);
        if (new_page == NULL)
            return false;

        m_address[s.high][s.low] = new_page;        
        m_capacity += SEGMENT_SIZE;
        m_segment_count++;
        return true;
    }


private: 
    static const db_uint32 ENTRY_SIZE = (1 << row_item_t::ROW_HIGH_BITS);

    column_table_t* m_table;
    mem_pool* m_mem_pool;

    db_uint32 m_data_len;
    db_uint32 m_count;
    db_uint32 m_capacity;
    db_uint32 m_segment_count;
    

    T** m_address[ENTRY_SIZE];
};


#endif //__COLUMN_H__


