// server_thread.cpp by scott.zgeng@gmail.com 2014.09.15


#include "server_thread.h"



server_worker_thread_t::server_worker_thread_t()
{
    m_stop = false;
}

server_worker_thread_t::~server_worker_thread_t()
{
    m_stop = true;
}


result_t server_worker_thread_t::start()
{
    result_t ret;
    ret = m_loop.init();
    IF_RETURN_FAILED(ret != RT_SUCCEEDED);

    ret = m_thread.start(this);
    IF_RETURN_FAILED(ret != RT_SUCCEEDED);

    return RT_SUCCEEDED;
}

void server_worker_thread_t::run()
{
    while (!m_stop) {
        m_loop.run_once();
    }
}



server_main_thread_t::server_main_thread_t() : 
    m_listener(&m_loop, this)
{
    m_stop = false;
    memset(m_workers, 0, sizeof(m_workers));
}

server_main_thread_t::~server_main_thread_t()
{
    m_stop = true;
}

result_t server_main_thread_t::init()
{
    result_t ret;
    db_uint16 server_port = 19992;  // TODO(scott.zgeng): 需要从配置中获取

    ret = m_listener.listen(server_port);
    IF_RETURN_FAILED(ret != RT_SUCCEEDED);

    return RT_SUCCEEDED;
}


void server_main_thread_t::on_accept(socket_handle fd, const sockaddr_in& addr)
{
    server_worker_thread_t* worker = find_unused_worker();
    if (worker == NULL) {
        close_socket(fd);
        return;
    }

    worker->start();
}

server_worker_thread_t* server_main_thread_t::find_unused_worker()
{
    for (db_uint32 i = 0; i < MAX_THREAD_NUM; i++) {
        if (m_workers[i] == NULL)  {
            m_workers[i] = new server_worker_thread_t();
            return m_workers[i];
        }
    }

    return NULL;
}


void server_main_thread_t::run()
{
    while (!m_stop) {
        m_loop.run_once();
    }
}
