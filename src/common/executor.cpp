// executor.cpp by scott.zgeng@gmail.com 2014.10.5


#include "executor.h"


class executor_worker_t: public runnable_t
{
public:
    executor_worker_t() {
        m_queue = NULL;
        m_start = false;
    }

    virtual ~executor_worker_t() {
    }


    result_t start(lock_free_queue_t* queue) {
        m_queue = queue;
        m_start = true;
        return m_thread.start(this);
    }

    virtual void run() {
        bool has_object;
        db_uint32 loop_count = 0;
        runnable_t* task;
        
        m_queue->start();

        while (m_start) {
            has_object = m_queue->dequeue((object_t**)&task);
            if (!has_object) {                
                if (loop_count >= 10000)
                    usleep(4000);                    
                else loop_count++;

                continue;
            }

            loop_count = 0;
            task->run();
        }        
    }

private:
    lock_free_queue_t* m_queue;
    thread_t m_thread;
    db_bool m_start;
};


executor_t::executor_t()
{
}

executor_t::~executor_t()
{
}


result_t executor_t::init(db_uint32 thread_count, db_uint32 max_task_queue)
{
    bool is_succ = m_task_queue.init(max_task_queue);
    IF_RETURN_FAILED(!is_succ);

    for (db_uint32 i = 0; i < thread_count; i++) {
        executor_worker_t* worker = new executor_worker_t();
        IF_RETURN_FAILED(worker == NULL);

        bool is_succ = m_workers.push_back(worker);
        IF_RETURN_FAILED(!is_succ);
    }

    for (db_uint32 i = 0; i < m_workers.size(); i++) {
        result_t ret= m_workers[i]->start(&m_task_queue);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);
    }

    return RT_SUCCEEDED;
}


void executor_t::uninit()
{
    // TODO(scott.zgeng): 增加清理操作
}

