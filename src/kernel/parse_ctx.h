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
    // TODO(scott.zgeng): ���������Ҫ����һ��ʵ��Ч��
    // strncpy_s�Ѿ�������BUFFER�ĳ��ȣ�����������
    strncpy_s(dst, n, src, n);
}




#endif //__PARSE_CTX_H__

