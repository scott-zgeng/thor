// thread.h by scott.zgeng@gmail.com 2014.10.05


#ifndef  __THREAD_H__
#define  __THREAD_H__


#include "define.h"
#include "tinythread.h"
#include "fast_mutex.h"


typedef tthread::fast_mutex mutex_t;

class runnable_t: public object_t
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
    result_t start(runnable_t* command) {
        tthread::thread* m_thread = new tthread::thread(thread_t::entry, command);
        IF_RETURN_FAILED(m_thread == NULL);

        return RT_SUCCEEDED;
    }

private:
    static void entry(void* arg) {
        runnable_t* command = (runnable_t*)arg;
        command->run();
    }

    tthread::thread* m_thread;
};




#endif //__THREAD_H__
