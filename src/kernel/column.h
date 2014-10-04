// column.h by scott.zgeng@gmail.com 2014.08.20


#ifndef  __COLUMN_H__
#define  __COLUMN_H__


#include <assert.h>
#include <string.h>

#include "define.h"
#include "variant.h"
#include "mem_pool.h"

#include "rowset.h"
#include "parse_ctx.h"


class column_base_t
{
public:
    static column_base_t* create_column(data_type_t type);

public:
    
    virtual ~column_base_t() {}

public:
    virtual result_t init(mem_pool_t* pool, db_int32 data_len) = 0;
    virtual void uninit() = 0;
    virtual result_t insert(void* values, db_int32 num) = 0;
    virtual data_type_t data_type() const = 0;
    virtual void* get_segment(db_uint32 segment_id) = 0;
    virtual db_int32 get_segment_row_count(db_uint32 segment_id) = 0;
    virtual void batch_get_rows_value(rowid_t* rows, db_int32 count, void* ptr) = 0;    
    virtual db_uint32 get_segment_count() = 0;
    virtual db_uint32 get_row_count() = 0;


    virtual result_t insert_row(const db_char* val) = 0;
};




// TODO(scott.zgeng): 部分函数可以优化成非虚函数
template<typename T>
class column_t: public column_base_t
{
public:
    column_t() {
        m_mem_pool = NULL;
        m_data_len = 0;
        m_row_count = 0;
        m_capacity = 0;
        m_segment_count = 0;
        memset(m_address, 0, sizeof(m_address));                      
    }

    virtual ~column_t() {
        uninit();
    }


public:
    virtual result_t init(mem_pool_t* pool, db_int32 data_len) {
        assert(pool != NULL);
        m_mem_pool = pool;
        m_data_len = data_len;
        return RT_SUCCEEDED;
    }

    // TODO(scott.zgeng): 增加内存清除的分支
    virtual void uninit() {
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


    virtual db_uint32 get_segment_count() {
        return m_segment_count;
    }


    virtual db_uint32 get_row_count() {
        return m_row_count;
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

        }
        else {
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

    virtual void batch_get_rows_value(rowid_t* rows, db_int32 count, void* ptr) {
        assert(count <= SEGMENT_SIZE);

        T* val = (T*)ptr;
        for (db_int32 i = 0; i < count; i++) {
            val[i] = get_row_value(rows[i]);
        }
    }
    
    virtual result_t insert_row(const db_char* val) {
        str2variant<T> str2var;
        T v = str2var(val);
        return insert(&v, 1);        
    }


protected:
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

protected:
    static const db_uint32 ENTRY_SIZE = (1 << row_item_t::SEGMENT_HIGH_BITS);
    
    mem_pool_t* m_mem_pool;
    db_uint32 m_data_len;
    db_uint32 m_row_count;
    db_uint32 m_capacity;
    db_uint32 m_segment_count;
    
    T** m_address[ENTRY_SIZE];    
};



// NOTE(scott.zgeng): 也可以用模板实现，后面如果有需要可以优化
class varchar_column_t : public column_t<db_string>
{
public:
    varchar_column_t() {
    }

    virtual ~varchar_column_t() {
        uninit();
    }

    virtual result_t init(mem_pool_t* pool, db_int32 data_len) {
        if (column_t<db_string>::init(pool, data_len) != RT_SUCCEEDED)
            return RT_FAILED;

        m_mem_region.init(pool);
        return RT_SUCCEEDED;
    }

    virtual void uninit() {

    }

    // NOTE(scott.zgeng): 只支持最大一次增加SEGMENT_SIZE的数据，如果超过需要循环调用该接口
    virtual result_t insert(void* values, db_int32 num) {
        assert(num <= SEGMENT_SIZE);

        result_t ret;
        if (!ensure_capacity(m_row_count + num))
            return RT_FAILED;

        db_int32 offset = m_row_count & row_item_t::ROW_OFFSET_MASK;
        db_uint32 last_segment_id = m_segment_count - 1;
        db_string* input = (db_string*)values;
        if (offset + num > SEGMENT_SIZE) {
            db_int32 part_num = SEGMENT_SIZE - offset;

            db_string* segment = (db_string*)get_segment(last_segment_id - 1);
            ret = insert_with_copy(segment + offset, input, part_num);
            IF_RETURN_FAILED(ret != RT_SUCCEEDED);

            db_string* new_segment = (db_string*)get_segment(last_segment_id);
            ret = insert_with_copy(new_segment, input + part_num, num - part_num);
            IF_RETURN_FAILED(ret != RT_SUCCEEDED);
        }
        else {
            db_string* segment = (db_string*)get_segment(last_segment_id);
            ret = insert_with_copy(segment + offset, input, num);
        }

        m_row_count += num;
        return RT_SUCCEEDED;
    }

    virtual result_t insert_row(const db_char* val) {                
        return insert(&val, 1);
    }



protected:
    result_t insert_with_copy(db_string* dst, db_string* src, db_uint32 num) {
        db_uint32 len;
        db_string ptr;
        for (db_uint32 i = 0; i < num; i++) {
            len = strlen(src[i]) + 1;
            ptr = (db_string)m_mem_region.alloc(len);
            IF_RETURN_FAILED(ptr == NULL);
            strncpy_ex(ptr, src[i], len);            
            dst[i] = ptr;            
        }
        return RT_SUCCEEDED;
    }

protected:
    mem_region_t m_mem_region;
};




#endif //__COLUMN_H__


