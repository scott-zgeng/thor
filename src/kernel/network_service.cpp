// network_service.cpp by scott.zgeng@gmail.com 2014.09.15

#include <string.h>
#include "network_service.h"



worker_thread_t::worker_thread_t()
{
    m_stop = false;
}

worker_thread_t::~worker_thread_t()
{
    m_stop = true;
}


result_t worker_thread_t::start()
{
    result_t ret;
    ret = m_loop.init();
    IF_RETURN_FAILED(ret != RT_SUCCEEDED);

    ret = m_thread.start(this);
    IF_RETURN_FAILED(ret != RT_SUCCEEDED);

    return RT_SUCCEEDED;
}

void worker_thread_t::run()
{
    while (!m_stop) {
        m_loop.run_once();
    }
}



network_service::network_service() : 
    m_listener(this)
{
    m_stop = false;
    memset(m_workers, 0, sizeof(m_workers));

}

network_service::~network_service()
{
    m_stop = true;
}


#include <sys/stat.h>
#include <sys/types.h>

result_t network_service::init()
{
    result_t ret;
    db_uint16 server_port = 19992;  // TODO(scott.zgeng): 需要从配置中获取

    ret = m_loop.init();
    IF_RETURN_FAILED(ret != RT_SUCCEEDED);

    ret = m_listener.listen(&m_loop, server_port);
    IF_RETURN_FAILED(ret != RT_SUCCEEDED);

    
    return RT_SUCCEEDED;
}


void network_service::on_accept(db_int32 fd, const sockaddr_in& addr)
{
    worker_thread_t* worker = find_unused_worker();
    if (worker == NULL) {
        close(fd);
        return;
    }

    worker->start();
}

worker_thread_t* network_service::find_unused_worker()
{
    for (db_uint32 i = 0; i < MAX_THREAD_NUM; i++) {
        if (m_workers[i] == NULL)  {
            m_workers[i] = new worker_thread_t();
            return m_workers[i];
        }
    }

    return NULL;
}


void network_service::run()
{
    while (!m_stop) {
        m_loop.run_once();
    }
}
