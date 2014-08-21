// statement.h by scott.zgeng@gmail.com 2014.08.21


#ifndef  __STATEMENT_H__
#define  __STATEMENT_H__



#include "define.h"



class statement_t
{
public:
    virtual ~statement_t() {}
    virtual result_t init() = 0;
    virtual result_t next() = 0;
    virtual void uninit() = 0;
};


class insert_stmt_t : public statement_t
{
public:
    insert_stmt_t();
    virtual ~insert_stmt_t();
public:
    virtual result_t init();
    virtual result_t next();
    virtual void uninit();
};


class select_stmt_t : public statement_t
{
public:
    select_stmt_t();
    virtual ~select_stmt_t();
public:
    virtual result_t init();
    virtual result_t next();
    virtual void uninit();
};





#endif //__STATEMENT_H__


