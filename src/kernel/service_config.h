// service_config.h by scott.zgeng@gmail.com 2014.10.04

#ifndef  __SERVICE_CONFIG_H__
#define  __SERVICE_CONFIG_H__



#include "define.h"


struct service_config_t
{
    result_t init(const char* path_name);

    db_int32 service_port;
    db_int64 mem_pool_size;
};



#endif //__SERVICE_CONFIG_H__

