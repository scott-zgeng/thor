// define.h by scott.zgeng@gmail.com 2014.08.5


#ifndef  __DEFINE_H__
#define  __DEFINE_H__

#include <stdio.h>

typedef char int8;
typedef unsigned uint8;
typedef short int16;
typedef unsigned short uint16;
typedef int int32;
typedef unsigned int uint32;
typedef long long int64;
typedef unsigned long long uint64;


typedef unsigned long long rowid_t;

// 为了防止和内置类型混用
struct result_t
{
    uint32 code;
    bool operator == (result_t other) { return code == other.code; }
    bool operator != (result_t other) { return code != other.code; }
};


enum data_type_t {
    DB_UNKNOWN = 0,
    DB_INT8,
    DB_INT16,
    DB_INT32,
    DB_INT64,
    DB_FLOAT,
    DB_DOUBLE,
    DB_STRING,
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

#endif //__DEFINE_H__

