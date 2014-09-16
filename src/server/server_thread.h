// server_server.h by scott.zgeng@gmail.com 2014.09.15

#ifndef  __SERVER_THREAD_H__
#define  __SERVER_THREAD_H__


#include "tinythread.h"
#include "fast_mutex.h"
#include "channel.h"


typedef tthread::fast_mutex mutex_t;

class executor_t
{
public:
    virtual void run() = 0;
};


class thread_t
{
public:
    thread_t() {
        m_thread = NULL;
    }

    virtual ~thread_t() {
        if (m_thread == NULL)
            return;

        delete m_thread;
    }

public:
    result_t start(executor_t* executor) {
        tthread::thread* m_thread = new tthread::thread(thread_entry, this);
        IF_RETURN_FAILED(m_thread == NULL);

        return RT_SUCCEEDED;
    }

private:
    static void thread_entry(void* arg) {
        thread_t* t = (thread_t*)arg;
        t->m_executor->run();
    }

    tthread::thread* m_thread;
    executor_t* m_executor;
};




class server_worker_thread_t : private executor_t
{
public:
    server_worker_thread_t();
    virtual ~server_worker_thread_t();

public:
    result_t start();
    result_t append_server_session(socket_handle fd, const sockaddr_in& addr);

private:
    virtual void run();

private:
    db_bool m_stop;
    channel_loop_t m_loop;
    thread_t m_thread;
};



class server_main_thread_t : public listen_handle_t
{
public:
    static const db_uint32 MAX_THREAD_NUM = 1024;

public:
    server_main_thread_t();
    ~server_main_thread_t();

public:
    result_t init();
    void run();

    virtual void on_accept(socket_handle fd, const sockaddr_in& addr);

private:
    server_worker_thread_t* find_unused_worker();

private:
    channel_loop_t m_loop;
    db_bool m_stop;
    listen_channel_t m_listener;

    server_worker_thread_t* m_workers[MAX_THREAD_NUM];
    mutex_t m_lock;
};

#endif //__SERVER_THREAD_H__


