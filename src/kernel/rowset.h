// rowset.h by scott.zgeng@gmail.com 2014.08.26

#ifndef  __ROWSET_H__
#define  __ROWSET_H__


#include <stdlib.h>
#include <assert.h>

#include "define.h"




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



struct row_item_t : public segment_item_t
{
    static const db_uint32 ROW_OFFSET_BITS = 10;
    static const db_uint32 ROW_OFFSET_MASK = (1 << ROW_OFFSET_BITS) - 1;

    static const db_uint32 MAX_ROW_COUNT = 1 << (SEGMENT_HIGH_BITS + SEGMENT_LOW_BITS + ROW_OFFSET_BITS);
    static const rowid_t MAX_ROWID = MAX_ROW_COUNT - 1;

    row_item_t(rowid_t rowid) : segment_item_t(rowid >> ROW_OFFSET_BITS) {
        COMPILE_ASSERT(SEGMENT_SIZE == (1 << ROW_OFFSET_BITS));
        offset = rowid & ROW_OFFSET_MASK;
    }

    db_uint16 offset;   // offset in the segment  
};


// row_set提供的功能： 
//   有多少行数据
//   如果获取行的数据
class rowset_t
{
public:
    enum mode_t {
        SCAN_MODE = 0,
        RANDOM_MODE,
    };

    rowset_t() {
        m_count = 0;        
        m_mode = SCAN_MODE;
        m_data = NULL;
        m_data_alloc = NULL;
    }

    rowset_t(rowid_t* ptr) {
        m_count = 0;        
        m_mode = SCAN_MODE;
        m_data = ptr;
        m_data_alloc = NULL;
    }

    ~rowset_t() {
        if (m_data_alloc != NULL) {
            free(m_data_alloc);
        }
    }

    result_t init(db_int32 column_num) {
        assert(m_data_alloc == NULL);

        m_data_alloc = (rowid_t*)malloc(column_num * SEGMENT_SIZE * sizeof(rowid_t));
        if (NULL == m_data_alloc)
            return RT_FAILED;

        m_data = m_data_alloc;
        return RT_SUCCEEDED;
    }

    void init(rowid_t* ptr) {
        m_data = ptr;
    }

    db_int32 count() const {
        return m_count;
    }

    void set_count(db_int32 n) {
        m_count = n;
    }

    void set_mode(mode_t mode) {
        m_mode = mode;
    }

    rowid_t* data() const {
        return m_data;
    }

    mode_t mode() const {
        return m_mode;
    }

    db_int32 segment() const {
        return m_data[0] >> row_item_t::ROW_OFFSET_BITS;
    }

private:
    db_int32 m_count;    
    mode_t m_mode;
    rowid_t* m_data;
    rowid_t* m_data_alloc;
};


#endif //__ROWSET_H__

