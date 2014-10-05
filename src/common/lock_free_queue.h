// lock_free_queue.h by scott.zgeng@gmail.com 2014.10.5
// the class of lfds611_queue wrapper


#ifndef  __LOCK_FREE_QUEUE_H__
#define  __LOCK_FREE_QUEUE_H__

extern "C" {
#include "liblfds611.h"
}


class lock_free_queue_t
{
public:
    lock_free_queue_t() {
        m_handle = NULL;
    }

    ~lock_free_queue_t() {
        uninit();
    }

public:
    bool init(db_uint32 element_count) {
        assert(m_handle == NULL);
        lfds611_atom_t ec = element_count;

        // Returns 1 on success and 0 on failure, with *qs being set to NULL on failure. 
        return lfds611_queue_new(&m_handle, ec);
    }


    // NOTES: any thread wishing to use that queue must first call this function, except the instantiating thread
    void start() {
        assert(m_handle != NULL);
        lfds611_queue_use(m_handle);
    }

    void uninit() {
        if (m_handle != NULL) {
            lfds611_queue_delete(m_handle, queue_delete_entry, NULL);
            m_handle = NULL;
        }
    }

    // enqueue function only fails when there are no elements available in the queue
    bool enqueue(object_t* obj) {
        // Returns 1 on a successful enqueue. Returns 0 if enqueing failed. 
        // Enqueuing only fails if the queue has exhausted its supply of freelist elements
        return lfds611_queue_enqueue(m_handle, obj);

    }

    // guaranteed_enqueue function only fails when malloc fails
    bool guaranteed_enqueue(object_t* obj) {
        return lfds611_queue_guaranteed_enqueue(m_handle, obj);
    }

    bool dequeue(object_t** obj) {
        return lfds611_queue_dequeue(m_handle, (void**)obj);
    }

    // the total count of elements belonging to the queue's freelist (not the free count, the total count).
    db_uint32 query_free_count() {
        lfds611_atom_t count;
        lfds611_queue_query(m_handle, LFDS611_QUEUE_QUERY_ELEMENT_COUNT, NULL, &count);
        return count;
    }

private:
    static void queue_delete_entry(void *user_data, void *user_state) {
        object_t* obj = (object_t*)user_data;
        delete obj;
    }

private:
    struct lfds611_queue_state* m_handle;
};


#endif //__LOCK_FREE_QUEUE_H__