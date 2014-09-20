// service_entry.cpp by scott.zgeng@gmail.com 2014.09.20


#include "service_entry.h"
#include "network_service.h"


int service_entry(int argc, char* argv[])
{
    result_t ret;
    network_service service;

    ret = service.init();
    if (ret != RT_SUCCEEDED)
        return 1;

    service.run();
    return 0;
}

