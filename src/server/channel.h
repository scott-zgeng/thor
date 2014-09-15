// channel.h by scott.zgeng@gmail.com 2014.09.14


#ifndef  __CHANNEL_H__
#define  __CHANNEL_H__


#include "ev.h"
#include "../kernel/define.h"


#ifdef _WIN32
#include <WinSock2.h>
typedef SOCKET socket_handle;
inline void close_socket(socket_handle fd)  { closesocket(fd); }
#else 
typedef int socket_handle;
inline void close_socket(socket_handle fd)  { close(fd); }
#endif



class channel_handle_t
{
public:
    virtual ~channel_handle_t() {}

    virtual void on_send() = 0;
    virtual void on_recv() = 0;
    virtual void on_close() = 0;
};



// 回调可以改为模板

class channel_loop_t;
class channel_base_t
{
public:
    static const db_int32 BACKLOG = 8;

public:
    channel_base_t(channel_loop_t* loop, channel_handle_t* handle);
    virtual ~channel_base_t();

    result_t send(void* ptr, db_int32 len);
    result_t recv(void* ptr, db_int32 len);
    void close();

protected:    
    void attach_socket(socket_handle fd);

    void loop_send();
    void loop_recv();

    void post_send() { 
        m_ev.cb = on_ev_send;
        post_event(EV_WRITE); 
    }
    void post_recv() { 
        m_ev.cb = on_ev_recv;
        post_event(EV_READ); 
    }
    void post_pause();
    void post_event(int events);

    socket_handle socket() { return m_fd; }

private:
    static void on_ev_send(struct ev_loop* loop, ev_io* ev, int events);
    static void on_ev_recv(struct ev_loop* loop, ev_io* ev, int events);

private:
    channel_loop_t* m_loop;
    channel_handle_t* m_handle;
    
    socket_handle m_fd;    
    
    db_char* m_send_ptr;
    db_int32 m_send_len;

    db_char* m_recv_ptr;
    db_int32 m_recv_len;

    ev_io m_ev;
};



class listen_handle_t
{
public:
    virtual void on_accept(socket_handle fd, const sockaddr_in& addr) = 0;    
};


class listen_channel_t : public channel_base_t, private channel_handle_t
{
public:
    listen_channel_t(channel_loop_t* loop, listen_handle_t* handle);
    virtual ~listen_channel_t();

public:
    result_t listen(db_uint16 port);

private:
    virtual void on_send();
    virtual void on_recv();
    virtual void on_close();

private:
    listen_handle_t* m_handle;
};



class up_channel_t : public channel_base_t
{
public:
    void connect();
};


class down_channel_t : public channel_base_t
{
public:
    void accept();
};



class channel_loop_t
{
public:
    channel_loop_t() {
        m_loop = NULL;
    }

    ~channel_loop_t() {
        if (m_loop != NULL) {
            stop();
            ev_loop_destroy(m_loop);
            return;        
        }        
    }
    
public:
    result_t init() {
        m_loop = ev_loop_new();
        IF_RETURN_FAILED(m_loop == NULL);

        return RT_SUCCEEDED;
    }

    void run() {
        ev_run(m_loop, 0);
    } 

    void run_once() {
        ev_run(m_loop, EVRUN_ONCE);
    }

    void stop() {
        ev_break(m_loop, EVBREAK_ALL);
    }

    struct ev_loop* native_handle() { 
        return m_loop; 
    }

private:
    struct ev_loop* m_loop;
};







#endif //__CHANNEL_H__

