// service_config.cpp by scott.zgeng@gmail.com 2014.10.04


#include "service_config.h"
#include "ini_config.h"

result_t service_config_t::init(const char* path_name)
{
    service_port = ini_getl("Database", "ServicePort", 19992, path_name);
    mem_pool_size = ini_getl("Database", "MemSpaceSize", 100, path_name);

    return RT_SUCCEEDED;
}


