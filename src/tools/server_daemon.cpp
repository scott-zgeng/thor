// server_daemon.cpp by scott.zgeng@gmail.com 2014.09.15

#include "server_thread.h"
#include "server_daemon.h"
#include "pod_vector.h"


result_t server_daemon_t::main_entry(int argc, char* argv[])
{
    result_t ret;
    main_thread_t main_thread;

    ret = main_thread.init();
    IF_RETURN_FAILED(ret != RT_SUCCEEDED);

    main_thread.run();    

    return RT_SUCCEEDED;
}
