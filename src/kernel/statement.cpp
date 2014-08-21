// statement.cpp by scott.zgeng@gmail.com 2014.08.21


#include "statement.h"


insert_stmt_t::insert_stmt_t()
{
}


insert_stmt_t::~insert_stmt_t()
{
}


result_t insert_stmt_t::init()
{
    return RT_FAILED;
}

result_t insert_stmt_t::next()
{
    return RT_FAILED;
}

void insert_stmt_t::uninit()
{

}



select_stmt_t::select_stmt_t()
{
}


select_stmt_t::~select_stmt_t()
{
}


result_t select_stmt_t::init()
{
    return RT_FAILED;
}

result_t select_stmt_t::next()
{
    return RT_FAILED;
}

void select_stmt_t::uninit()
{
}


