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



/*

struct rowset_t
{
    db_uint32 count;
    db_char* buffer;
}


节点取出数据，表达式用数据

单表（ROWID） 有两种取法： RANDOM or SCAN
多表（ROWID）  有两种模型： 列式 or 行式
临时表(POINTER)  



表达式初始化时，就设置好模式（单表，多表，临时表）

result_set_t
rs_context_t
{
    count;
    mdoe;
    buffer;
}

template<table_num>
rowset(row_ctx_t)


aggr_rowset(row_ctx_t)


*/




// rowset分成两部分，一个提供空间，一个提供操作，函数之间只传递空间

// row_set提供的功能： 
//   有多少行数据
//   如果获取行的数据



enum rowset_mode_t
{
    UNKNOWN_MODE = 0,
    SINGLE_TABLE_MODE = 1,
    MULTI_TABLE_MODE = 2,
    AGGR_TABLE_MODE = 3,
};


struct rowset_t
{
    db_uint32 count;
    rowset_mode_t mode;    
};


struct single_rowset_t : public rowset_t
{
    static const db_uint32 INVALID_SEGMENT = (-1);
    db_uint32 segment_id;
    rowid_t rows[SEGMENT_SIZE];

    single_rowset_t() {
        count = 0;
        mode = SINGLE_TABLE_MODE;
        segment_id = INVALID_SEGMENT;
    }

    db_bool is_scan() { 
        return (segment_id != INVALID_SEGMENT);
    }
};


template<db_uint32 TABLE_NUM>
struct multi_rowset_t : public rowset_t
{
    rowid_t rows[TABLE_NUM][SEGMENT_SIZE];

    multi_rowset_t() {
        count = 0;
        mode = MULTI_TABLE_MODE;
    }
};


struct aggr_rowset_t : public rowset_t
{
    void* rows[SEGMENT_SIZE];

    aggr_rowset_t() {
        count = 0;
        mode = MULTI_TABLE_MODE;
    }
};

inline rowset_t* create_rowset(rowset_mode_t mode, db_uint32 table_count)
{
    switch (mode)
    {

    case SINGLE_TABLE_MODE:
        return new single_rowset_t(); 
    case MULTI_TABLE_MODE:
        switch (table_count)
        {
        case 2: return new multi_rowset_t<2>();
        case 3: return new multi_rowset_t<3>();
        case 4: return new multi_rowset_t<4>();
        case 5: return new multi_rowset_t<5>();
        case 6: return new multi_rowset_t<6>();
        case 7: return new multi_rowset_t<7>();
        case 8: return new multi_rowset_t<8>();
        default: return NULL;            
        }
        
    case AGGR_TABLE_MODE:
        return new aggr_rowset_t();
    default:
        return NULL;
    }


}


//struct st_rowset_t : public new_rowset_t
//{
//    rowid_t rows[SEGMENT_SIZE];
//};


//class rowset_t
//{
//public:
//    enum mode_t {
//        SCAN_MODE = 0,
//        RANDOM_MODE,
//    };
//
//    rowset_t() {
//        m_count = 0;        
//        m_mode = SCAN_MODE;
//        m_data = NULL;
//        m_data_alloc = NULL;
//    }
//
//    rowset_t(rowid_t* ptr) {
//        m_count = 0;        
//        m_mode = SCAN_MODE;
//        m_data = ptr;
//        m_data_alloc = NULL;
//    }
//
//    ~rowset_t() {
//        if (m_data_alloc != NULL) {
//            free(m_data_alloc);
//        }
//    }
//
//    result_t init(db_int32 column_num) {
//        assert(m_data_alloc == NULL);
//
//        m_data_alloc = (rowid_t*)malloc(column_num * SEGMENT_SIZE * sizeof(rowid_t));
//        if (NULL == m_data_alloc)
//            return RT_FAILED;
//
//        m_data = m_data_alloc;
//        return RT_SUCCEEDED;
//    }
//
//    void init(rowid_t* ptr) {
//        m_data = ptr;
//    }
//
//    db_uint32 count() const {
//        return m_count;
//    }
//
//    void set_count(db_uint32 n) {
//        m_count = n;
//    }
//
//    void set_mode(mode_t mode) {
//        m_mode = mode;
//    }
//
//    rowid_t* data() const {
//        return m_data;
//    }
//
//    mode_t mode() const {
//        return m_mode;
//    }
//
//    db_int32 segment() const {
//        return m_data[0] >> row_item_t::ROW_OFFSET_BITS;
//    }
//
//private:
//    db_uint32 m_count;    
//    mode_t m_mode;
//    rowid_t* m_data;
//    rowid_t* m_data_alloc;
//};


#endif //__ROWSET_H__

