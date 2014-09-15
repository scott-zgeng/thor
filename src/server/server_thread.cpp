// server_thread.cpp by scott.zgeng@gmail.com 2014.09.15


#include "server_thread.h"


server_main_thread_t::server_main_thread_t()
{
    m_stop = false;
}

server_main_thread_t::~server_main_thread_t()
{
    m_stop = true;
}

result_t server_main_thread_t::init()
{
    return RT_SUCCEEDED;
}

void server_main_thread_t::run()
{
    while (!m_stop) {
        m_loop.run_once();
    }
}
