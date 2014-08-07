// variant.h by scott.zgeng@gmail.com 2014.08.07


#ifndef  __VARIANT_H__
#define  __VARIANT_H__

#include "define.h"



// NOTE(scott.zgeng): 根据数据类型定义元的数据类型
template<typename T> struct variant_type { data_type_t operator() (); };
#define VARIANT_TYPE(T, TYPE)  \
    template<> struct variant_type<T> { data_type_t operator() () { return TYPE; } }

VARIANT_TYPE(db_int8, DB_INT8);
VARIANT_TYPE(db_int16, DB_INT16);
VARIANT_TYPE(db_int32, DB_INT32);
VARIANT_TYPE(db_int64, DB_INT64);
VARIANT_TYPE(db_float, DB_FLOAT);
VARIANT_TYPE(db_double, DB_DOUBLE);
VARIANT_TYPE(db_string, DB_STRING);


template<typename T, typename RT, bool MUTE = false> struct variant_cast {};

#define VARIANT_CAST(T, RT)  \
    template<> struct variant_cast<T, RT, false> { RT operator () (T v) { return v; } }

#define VARIANT_CAST_NO_WARNING(T, RT)  \
    template<> struct variant_cast<T, RT, false> { RT operator () (T v) { return (RT)v; } }

#define VARIANT_CAST_DISABLE(T, RT)  \
    template<> struct variant_cast<T, RT, false> { RT operator () (T v) { assert(false);  return 0; } }

#define VARIANT_CAST_FORCE(T, RT)  \
    template<> struct variant_cast<T, RT, true> { RT operator () (T v) { return (RT)v; } }


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




#endif //__VARIANT_H__

