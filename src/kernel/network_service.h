// network_service.h by scott.zgeng@gmail.com 2014.09.15

#ifndef  __NETWORK_SERVICE_H__
#define  __NETWORK_SERVICE_H__

#include <netinet/in.h>

#include "tinythread.h"
#include "fast_mutex.h"
#include "channel.h"


/*
1. start-up
    To begin a session, a frontend opens a connection to the server and sends a startup message. 
    This message includes the names of the user and of the database the user wants to connect to; 
    it also identifies the particular protocol version to be used. (Optionally, the startup message 
    can include additional settings for run-time parameters.) The server then uses this information 
    and the contents of its configuration files (such as pg_hba.conf) to determine whether the connection 
    is provisionally acceptable, and what additional authentication is required (if any).

    The server then sends an appropriate authentication request message, to which the frontend must 
    reply with an appropriate authentication response message (such as a password).

*/

class packet_ostream_t;
class opacket_t;
class server_session_t : private channel_action_t
{
public:
    static const db_uint32 MAX_SEND_BUF_SIZE = 1024;
    static const db_uint32 MAX_RECV_BUF_SIZE = 1024;    

    static const db_uint32 HEAD_SIZE = 5;

public:
    server_session_t();
    virtual ~server_session_t();

public:
    void init(channel_loop_t* loop, db_int32 fd, const sockaddr_in& addr);

    void recv_startup();
    void recv_packet();
    void send_packet(opacket_t& packet);

private:
    virtual void on_send();
    virtual void on_recv();
    virtual void on_close();
    
    result_t on_recv_packet();
    result_t on_recv_head();

private:
    sockaddr_in m_client_addr;
    channel_base_t m_channel;

    db_int8 m_is_startup;
    db_int8 m_is_header;

    db_int8 m_pack_type;
    db_uint32 m_pack_len;
    db_char m_send_buff[MAX_SEND_BUF_SIZE];
    db_char m_recv_buff[MAX_RECV_BUF_SIZE];

    
};




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
        tthread::thread* m_thread = new tthread::thread(thread_entry, executor);
        IF_RETURN_FAILED(m_thread == NULL);

        return RT_SUCCEEDED;
    }

private:
    static void thread_entry(void* arg) {
        executor_t* executor = (executor_t*)arg;        
        executor->run();
    }

    tthread::thread* m_thread;    
};


class worker_thread_t : private executor_t
{
public:
    worker_thread_t();
    virtual ~worker_thread_t();

public:
    result_t start(db_int32 fd, const sockaddr_in& addr);
    

private:
    virtual void run();

private:
    db_bool m_stop;
    channel_loop_t m_loop;
    thread_t m_thread;
    server_session_t m_session;

};



class network_service : public listen_action_t
{
public:
    static const db_uint32 MAX_THREAD_NUM = 1024;

public:
    network_service();
    ~network_service();

public:
    result_t init();
    void run();

    virtual void on_accept(db_int32 fd, const sockaddr_in& addr);

private:
    worker_thread_t* find_unused_worker();

private:
    channel_loop_t m_loop;
    db_bool m_stop;
    listen_channel_t m_listener;

    worker_thread_t* m_workers[MAX_THREAD_NUM];
    mutex_t m_lock;
};



#endif //__NETWORK_SERVICE_H__


