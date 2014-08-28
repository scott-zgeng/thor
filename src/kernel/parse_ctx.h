// parse_ctx.h by scott.zgeng@gmail.com 2014.07.11

 
#ifndef  __PARSE_CTX_H__
#define  __PARSE_CTX_H__

extern "C" {
#include "../sql/sqliteInt.h"
#include "../sql/vdbeInt.h"
}



#include "define.h"


inline void strncpy_ex(char* dst, const char* src, size_t n)
{
    // TODO(scott.zgeng): 这个函数需要测试一下实际效果
    // strncpy_s已经考虑了BUFFER的长度，包括结束符
    strncpy_s(dst, n, src, n);
}




#endif //__PARSE_CTX_H__

