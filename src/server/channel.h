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


class channel_base_t
{
public:
    static const db_int32 BACKLOG = 8;

public:
    channel_base_t(channel_loop_t* loop, channel_handle_t* handle);
    virtual ~channel_base_t();

    void send(void* buff, db_uint32 len);
    void recv(void* buff, db_uint32 len);
    void close();


protected:    
    static void on_ev_write(struct ev_loop* loop, ev_io* ev, int events);
    static void on_ev_read(struct ev_loop* loop, ev_io* ev, int events);

    void attach(socket_handle fd, const sockaddr_in& addr);

    void loop_write();
    void loop_read();

    void set_mode(int events); 
    void set_none();

private:
    channel_loop_t* m_loop;
    channel_handle_t* m_handle;
    sockaddr_in m_addr;
    socket_handle m_fd;
    db_byte* m_write_buff;
    db_uint32 m_write_len;
    db_byte* m_read_buff;
    db_uint32 m_read_len;
    ev_io m_ev;
};



class listen_handle_t
{
    void on_accept();
};


class listen_channel_t : public channel_base_t, channel_handle_t
{
public:
    listen_channel_t(channel_loop_t* loop, listen_handle_t* handle);
    
public:
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
    channel_loop_t();
    virtual ~channel_loop_t();
    
public:
    result_t init();
    struct ev_loop* ptr() { return m_loop; }
private:
    struct ev_loop* m_loop;
};







#endif //__CHANNEL_H__

