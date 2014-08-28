// column.cpp by scott.zgeng@gmail.com 2014.08.20


#include "column.h"


column_base_t* column_base_t::create_column(data_type_t type)
{
    switch (type)
    {
    case DB_INT8:
        return new column_t<db_int8>();
    case DB_INT16:
        return new column_t<db_int16>();
    case DB_INT32:
        return new column_t<db_int32>();
    case DB_INT64:
        return new column_t<db_int64>();
    case DB_FLOAT:
        return new column_t<db_float>();
    case DB_DOUBLE:
        return new column_t<db_double>();
    case DB_STRING:
        return new varchar_column_t();
    default:
        return NULL;
    }
}

