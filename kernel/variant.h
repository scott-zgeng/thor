// variant.h by scott.zgeng@gmail.com 2014.08.07


#ifndef  __VARIANT_H__
#define  __VARIANT_H__

#include "define.h"

// NOTE(scott.zgeng): 根据数据类型定义元的数据类型
template<data_type_t TYPE>
struct variant {};


template<> struct variant<DB_INT8>     { int8 v; };
template<> struct variant<DB_INT16>     { int16 v; };
template<> struct variant<DB_INT32>     { int32 v; };
template<> struct variant<DB_INT64>     { int64 v; };
template<> struct variant<DB_FLOAT>     { float v; };
template<> struct variant<DB_DOUBLE>    { double v; };
template<> struct variant<DB_STRING>    { char* v; };


template<data_type_t TYPE, data_type_t RT_TYPE>
struct variant_convertor {};

#define VARIANT_CONV_ENABLE(TYPE, RT_TYPE, type, rt_type)  \
    template<> struct variant_convertor<TYPE, RT_TYPE> { rt_type operator () (type v) { return (rt_type)v; }}

VARIANT_CONV_ENABLE(DB_INT8, DB_INT16, int8, int16);
VARIANT_CONV_ENABLE(DB_INT8, DB_INT32, int8, int32);
VARIANT_CONV_ENABLE(DB_INT8, DB_INT64, int8, int64);
VARIANT_CONV_ENABLE(DB_INT8, DB_FLOAT, int8, float);
VARIANT_CONV_ENABLE(DB_INT8, DB_DOUBLE, int8, double);

VARIANT_CONV_ENABLE(DB_INT16, DB_INT32, int16, int32);
VARIANT_CONV_ENABLE(DB_INT16, DB_INT64, int16, int64);
VARIANT_CONV_ENABLE(DB_INT16, DB_FLOAT, int16, float);
VARIANT_CONV_ENABLE(DB_INT16, DB_DOUBLE, int16, double);

VARIANT_CONV_ENABLE(DB_INT32, DB_INT64, int32, int64);
VARIANT_CONV_ENABLE(DB_INT32, DB_DOUBLE, int32, double);

VARIANT_CONV_ENABLE(DB_INT64, DB_DOUBLE, int64, double);

VARIANT_CONV_ENABLE(DB_FLOAT, DB_DOUBLE, float, double);


template<data_type_t TYPE, data_type_t RT_TYPE>
struct variant_cast {};

#define VARIANT_CAST(TYPE, RT_TYPE, type, rt_type)  \
    template<> struct variant_cast<TYPE, RT_TYPE> { rt_type operator () (type v) { return (rt_type)v; } }


VARIANT_CAST(DB_INT8, DB_INT64, int8, int64);
VARIANT_CAST(DB_INT16, DB_INT64, int16, int64);
VARIANT_CAST(DB_INT32, DB_INT64, int32, int64);
VARIANT_CAST(DB_INT64, DB_INT64, int64, int64);

#endif //__VARIANT_H__

