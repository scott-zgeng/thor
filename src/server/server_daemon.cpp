// server_daemon.cpp by scott.zgeng@gmail.com 2014.09.15

#include "server_thread.h"
#include "server_daemon.h"
#include "../kernel/pod_vector.h"


struct server_context_t
{
    server_main_thread_t main_thread;
    

    result_t init() {
        return main_thread.init();
    }

    result_t start() {
        main_thread.run();
        return RT_SUCCEEDED;
    }
};



result_t server_daemon_t::main_entry(int argc, char* argv[])
{
    server_context_t ctx;
    result_t ret;

    ret = ctx.init();
    IF_RETURN_FAILED(ret != RT_SUCCEEDED);

    ret = ctx.start();
    IF_RETURN_FAILED(ret != RT_SUCCEEDED);

    return RT_SUCCEEDED;
}
