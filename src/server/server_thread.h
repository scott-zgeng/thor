// server_server.h by scott.zgeng@gmail.com 2014.09.15

#ifndef  __SERVER_THREAD_H__
#define  __SERVER_THREAD_H__


#include "tinythread.h"
#include "channel.h"

class thread_base_t
{
public:
    thread_base_t() {
        m_thread = NULL;
    }

    virtual ~thread_base_t() {
        if (m_thread == NULL)
            return;

        delete m_thread;
    }

public:
    result_t start() {
        tthread::thread* m_thread = new tthread::thread(thread_entry, this);
        IF_RETURN_FAILED(m_thread == NULL);

        return RT_SUCCEEDED;
    }

    virtual void run() = 0;

private:
    static void thread_entry(void* arg) {
        thread_base_t* t = (thread_base_t*)arg;
        t->run();
    }

    tthread::thread* m_thread;
};


class server_main_thread_t
{
public:
    server_main_thread_t();
    ~server_main_thread_t();

public:
    result_t init();
    void run();

private:
    channel_loop_t m_loop;
    db_bool m_stop;
};

#endif //__SERVER_THREAD_H__


