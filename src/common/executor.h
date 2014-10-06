// executor.h by scott.zgeng@gmail.com 2014.10.5

#ifndef  __EXECUTOR_H__
#define  __EXECUTOR_H__


#include <assert.h>

#include "define.h"
#include "thread.h"
#include "pod_vector.h"
#include "lock_free_queue.h"




class executor_worker_t;
class executor_t
{
public:
    executor_t();
    ~executor_t();

public:
    result_t init(db_uint32 thread_count, db_uint32 max_task_queue);
    void uninit();

    bool execute(runnable_t* task) {
        return m_task_queue.enqueue(task);
    }

public:
    pod_vector<executor_worker_t*> m_workers;
    lock_free_queue_t m_task_queue;
};


#endif //__EXECUTOR_H__

