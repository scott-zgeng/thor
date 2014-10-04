// variant.h by scott.zgeng@gmail.com 2014.08.07


#ifndef  __VARIANT_H__
#define  __VARIANT_H__

#include <assert.h>
#include <stdlib.h>
#include "define.h"



// NOTE(scott.zgeng): 根据数据类型定义元的数据类型
template<typename T> 
struct variant_type { 
    data_type_t operator() (); 
};

#define VARIANT_TYPE(T, TYPE)  \
    template<> struct variant_type<T> { \
        data_type_t operator() () { return TYPE; } \
    }

VARIANT_TYPE(db_int8, DB_INT8);
VARIANT_TYPE(db_int16, DB_INT16);
VARIANT_TYPE(db_int32, DB_INT32);
VARIANT_TYPE(db_int64, DB_INT64);
VARIANT_TYPE(db_float, DB_FLOAT);
VARIANT_TYPE(db_double, DB_DOUBLE);
VARIANT_TYPE(db_string, DB_STRING);


template<typename T, typename RT, bool MUTE=false> 
struct variant_cast {};

#define VARIANT_CAST(T, RT)  \
    template<> struct variant_cast<T, RT, false> { \
        RT operator () (T v) { return v; } \
    }

#define VARIANT_CAST_NO_WARNING(T, RT)  \
    template<> struct variant_cast<T, RT, false> { \
        RT operator () (T v) { return (RT)v; } \
    }

#define VARIANT_CAST_DISABLE(T, RT)  \
    template<> struct variant_cast<T, RT, false> { \
        RT operator () (T v) { assert(false);  return 0; } \
    }

#define VARIANT_CAST_FORCE(T, RT)  \
    template<> struct variant_cast<T, RT, true> { \
        RT operator () (T v) { return (RT)v; } \
    }


VARIANT_CAST(db_int8, db_int16);
VARIANT_CAST(db_int8, db_int32);
VARIANT_CAST(db_int8, db_int64);
VARIANT_CAST(db_int8, db_float);
VARIANT_CAST(db_int8, db_double);

VARIANT_CAST(db_int16, db_int16);
VARIANT_CAST(db_int16, db_int32);
VARIANT_CAST(db_int16, db_int64);
VARIANT_CAST(db_int16, db_float);
VARIANT_CAST(db_int16, db_double);

VARIANT_CAST_DISABLE(db_int32, db_int16);
VARIANT_CAST(db_int32, db_int32);
VARIANT_CAST(db_int32, db_int64);
VARIANT_CAST_DISABLE(db_int32, db_float);
VARIANT_CAST(db_int32, db_double);

VARIANT_CAST_NO_WARNING(db_int64, db_int16);
VARIANT_CAST_NO_WARNING(db_int64, db_int32);
VARIANT_CAST(db_int64, db_int64);
VARIANT_CAST_NO_WARNING(db_int64, db_float);
VARIANT_CAST_NO_WARNING(db_int64, db_double);

VARIANT_CAST_DISABLE(db_float, db_int16);
VARIANT_CAST_DISABLE(db_float, db_int32);
VARIANT_CAST_DISABLE(db_float, db_int64);
VARIANT_CAST_DISABLE(db_float, db_float);
VARIANT_CAST(db_float, db_double);

VARIANT_CAST_DISABLE(db_double, db_int8);
VARIANT_CAST_DISABLE(db_double, db_int16);
VARIANT_CAST_DISABLE(db_double, db_int32);
VARIANT_CAST_DISABLE(db_double, db_int64);
VARIANT_CAST_DISABLE(db_double, db_float);
VARIANT_CAST_DISABLE(db_double, db_double);



VARIANT_CAST_FORCE(db_int64, db_int8);
VARIANT_CAST_FORCE(db_int64, db_int16);
VARIANT_CAST_FORCE(db_int64, db_int32);
VARIANT_CAST_FORCE(db_int64, db_int64);



struct variant_t
{
    variant_t() {
        type = DB_UNKNOWN;
    }

    explicit variant_t(db_int8 val) {
        type = DB_INT8;
        int8_val = val;
    }

    explicit variant_t(db_int16 val) {
        type = DB_INT16;
        int16_val = val;
    }

    explicit variant_t(db_int32 val) {
        type = DB_INT32;
        int32_val = val;
    }

    explicit variant_t(db_int64 val) {
        type = DB_INT64;
        int64_val = val;
    }

    explicit variant_t(db_float val) {
        type = DB_FLOAT;
        float_val = val;
    }

    explicit variant_t(db_double val) {
        type = DB_DOUBLE;
        double_val = val;
    }

    explicit variant_t(db_char* val) {
        type = DB_STRING;
        str_val = val;
    }

    data_type_t type;
    union {
        db_int8 int8_val;
        db_int16 int16_val;
        db_int32 int32_val;
        db_int64 int64_val;
        db_float float_val;
        db_double double_val;
        db_char* str_val;
    };

    void to_string(db_char* val) {
        switch (type)
        {
        case DB_INT8:
            sprintf(val, "%d", int8_val);
            break;
        case DB_INT16:
            sprintf(val, "%d", int16_val);
            break;
        case DB_INT32:
            sprintf(val, "%d", int32_val);
            break;
        case DB_INT64:
            sprintf(val, "%lld", int64_val);
            break;
        case DB_FLOAT:
            sprintf(val, "%f", float_val);
            break;
        case DB_DOUBLE:
            sprintf(val, "%f", double_val);
            break;
        case DB_STRING:
            strcpy(val, str_val);
            break;
        default:
            assert(false);
            break;
        }
    }
};

template<typename T>
struct str2variant {
    T operator() (const db_char* str);
};

template<>
struct str2variant<db_int8> {
    db_int8 operator() (const db_char* str) {
        return (db_int8)atoi(str);
    }
};

template<>
struct str2variant<db_int16> {
    db_int16 operator() (const db_char* str) {
        return (db_int16)atoi(str);
    }
};


template<>
struct str2variant<db_int32> {
    db_int32 operator() (const db_char* str) {
        return atoi(str);
    }
};


template<>
struct str2variant<db_int64> {
    db_int64 operator() (const db_char* str) {
        return atoll(str);
    }
};

template<>
struct str2variant<db_float> {
    db_float operator() (const db_char* str) {
        return (db_float)atof(str);
    }
};


template<>
struct str2variant<db_double> {
    db_double operator() (const db_char* str) {
        return atof(str);
    }
};


template<>
struct str2variant<db_string> {
    db_string operator() (const db_char* str) {
        return NULL;
    }
};


#endif //__VARIANT_H__

