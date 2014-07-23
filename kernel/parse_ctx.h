// parse_ctx.h by scott.zgeng@gmail.com 2014.07.11

 
#ifndef  __PARSE_CTX__
#define  __PARSE_CTX__

extern "C" {
#include "../sql/sqliteInt.h"

}


class node_base
{
public:
    virtual int init(Parse* parse, Select* select, int param) = 0;
    virtual int next() = 0;
    virtual void uninit() = 0;
};


#endif //__PARSE_CTX__

