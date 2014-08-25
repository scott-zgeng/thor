// bitmap.h by scott.zgeng@gmail.com 2014.08.17


#ifndef  __BITMAP_H__
#define  __BITMAP_H__


#include <assert.h>
#include "define.h"



class bitmap_t
{
public:
    bitmap_t(db_byte* ptr) {
        m_bitmap = ptr;
    }
    
    void set(db_uint32 idx) {
        static const db_byte MASK[] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };        
        m_bitmap[idx >> 3] |= MASK[idx & 0xFF];
    }

    void clean(db_uint32 idx) {
        static const db_byte MASK[] = { 0x7F, 0xBF, 0xDF, 0xEF, 0xF7, 0xFB, 0xFD, 0xFE };
        m_bitmap[idx >> 3] &= MASK[idx & 0xFF];
    }

    db_byte get(db_uint32 idx) {
        static const db_byte MASK[] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };
        return (m_bitmap[idx >> 3] &= MASK[idx & 0xFF]);
    }

private:
    db_byte* m_bitmap;
};



template<size_t SIZE>
class packed_integer
{
public:
    static void set(db_byte* base, db_int32 idx, db_int32 val);
    static db_int32 get(db_byte* base, db_int32 idx);
};


template<>
class packed_integer<1>
{
private:
    struct item {
        unsigned char f0 : 1;
        unsigned char f1 : 1;
        unsigned char f2 : 1;
        unsigned char f3 : 1;
        unsigned char f4 : 1;
        unsigned char f5 : 1;
        unsigned char f6 : 1;
        unsigned char f7 : 1;
    };

public:
    static void set(db_byte* base, db_int32 idx, db_int32 val) {
        assert(0 <= val && val < 2);

        item* ptr = (item*)(base + (idx >> 3));
        switch (idx & 0x07)
        {
        case 0: ptr->f0 = val; return;
        case 1: ptr->f1 = val; return;
        case 2: ptr->f2 = val; return;
        case 3: ptr->f3 = val; return;
        case 4: ptr->f4 = val; return;
        case 5: ptr->f5 = val; return;
        case 6: ptr->f6 = val; return;
        case 7: ptr->f7 = val; return;
        default: return;
        }
    }


    static db_int32 get(db_byte* base, db_int32 idx) {
        item* ptr = (item*)(base + (idx >> 3));
        switch (idx & 0x07)
        {
        case 0: return ptr->f0;
        case 1: return ptr->f1;
        case 2: return ptr->f2;
        case 3: return ptr->f3;
        case 4: return ptr->f4;
        case 5: return ptr->f5;
        case 6: return ptr->f6;
        case 7: return ptr->f7;
        default: return 0;
        }
    }
};


template<>
class packed_integer<2>
{
private:
    struct item {
        unsigned char f0 : 2;
        unsigned char f1 : 2;
        unsigned char f2 : 2;
        unsigned char f3 : 2;
    };
public:
    static void set(db_byte* base, db_int32 idx, db_int32 val) {
        assert(0 <= val && val < 4);
        item* ptr = (item*)(base + (idx >> 2));
        switch (idx & 0x03)
        {
        case 0: ptr->f0 = val; return;
        case 1: ptr->f1 = val; return;
        case 2: ptr->f2 = val; return;
        case 3: ptr->f3 = val; return;
        default: return;
        }
    }

    static db_int32 get(db_byte* base, db_int32 idx) {
        item* ptr = (item*)(base + (idx >> 2));
        switch (idx & 0x03)
        {
        case 0: return ptr->f0;
        case 1: return ptr->f1;
        case 2: return ptr->f2;
        case 3: return ptr->f3;
        default: return 0;
        }
    }
};



template<>
class packed_integer<4>
{
private:
    struct item {
        unsigned char f0 : 4;
        unsigned char f1 : 4;        
    };
public:
    static void set(db_byte* base, db_int32 idx, db_int32 val) {
        assert(0 <= val && val < 16);

        item* ptr = (item*)(base + (idx >> 1));
        switch (idx & 0x01)
        {
        case 0: ptr->f0 = val; return;
        case 1: ptr->f1 = val; return;        
        default: return;
        }
    }

    static db_int32 get(db_byte* base, db_int32 idx) {
        item* ptr = (item*)(base + (idx >> 1));
        switch (idx & 0x01)
        {
        case 0: return ptr->f0;
        case 1: return ptr->f1;        
        default: return 0;
        }
    }
};


typedef packed_integer<1> db_int1;
typedef packed_integer<2> db_int2;
typedef packed_integer<4> db_int4;


#endif //__BITMAP_H__

