// define.h by scott.zgeng@gmail.com 2014.08.5


#ifndef  __DEFINE_H__
#define  __DEFINE_H__

#include <stdio.h>



enum data_type_t {
    DB_UNKNOWN = 0,
    DB_INT8,
    DB_INT16,
    DB_INT32,
    DB_INT64,
    DB_FLOAT,
    DB_DOUBLE,
    DB_STRING,
    DB_MAX_TYPE  // 必须放在最后
};


// 和上面的data_type是对应的
typedef char db_int8;
typedef unsigned db_uint8;
typedef short db_int16;
typedef unsigned short db_uint16;
typedef int db_int32;
typedef unsigned int db_uint32;
typedef long long db_int64;
typedef unsigned long long db_uint64;
typedef float db_float;
typedef double db_double;
typedef char* db_string;


typedef unsigned long long rowid_t;

static const rowid_t ROWID_INVALID = (-1);
static const rowid_t ROWID_SEGMENT = (-2);


// 为了防止和内置类型混用
struct result_t
{
    db_uint32 code;
    bool operator == (result_t other) { return code == other.code; }
    bool operator != (result_t other) { return code != other.code; }
};


const static result_t RT_SUCCEEDED = { 0 };
const static result_t RT_FAILED = { -1 };



#ifndef DB_OMIT_TRACE
#define DB_TRACE(...)  do { printf(__VA_ARGS__); printf(" in %s() [%s:%d]\n", __FUNCTION__, __FILE__, __LINE__); } while (0)
#else
#define DB_TRACE(...)
#endif



#define IF_RETURN(code, condition) \
do { if (condition) { DB_TRACE("if_return"); return (code); } } while (0)

#define IF_RETURN_FAILED(condition) IF_RETURN(RT_FAILED, condition)


#define INLINE  inline


#undef COMPILE_ASSERT
#define COMPILE_ASSERT(condition)  do { int __dummy[(condition) ? 1 : -1]; __dummy[0] = __dummy[0]; } while (0)
    

static const db_int32 SEGMENT_SIZE = 1024;



#endif //__DEFINE_H__

