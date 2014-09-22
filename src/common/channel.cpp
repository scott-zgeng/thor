// channel.cpp by scott.zgeng@gmail.com 2014.09.14

#include <assert.h>
#include <errno.h>
#include <sys/types.h>  
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>


#include "channel.h"


db_int32 set_noblock_socket(db_int32 fd)
{
#ifndef _WIN32
    return fcntl(fd, F_SETFL, O_NONBLOCK | O_RDWR);       
#else
    unsigned long flag = 1;
    return ioctlsocket(fd, FIONBIO, &flag);    
#endif
}

db_int32 set_nodelay_socket(db_int32 fd)
{
    int flag = 1;
    return setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag));
}

db_int32 set_reuseaddr_socket(db_int32 fd)
{
    int flag = 1;
    return setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*)&flag, sizeof(flag));
}

db_bool is_noblock_error(int code)
{
    switch (code)
    {
    case ECONNRESET:
    case ECONNABORTED:
    case EAGAIN:
        return true;
    default:
        return false;
    }
}


void channel_base_t::on_ev_send(struct ev_loop* loop, ev_io* ev, int events)
{
    channel_base_t* channel = (channel_base_t*)ev->data;
    channel->loop_send();
}


void channel_base_t::on_ev_recv(struct ev_loop* loop, ev_io* ev, int events)
{
    channel_base_t* channel = (channel_base_t*)ev->data;
    channel->loop_recv();
}


channel_base_t::channel_base_t(channel_action_t* action)
{
    m_loop = NULL;
    m_action = action;

    m_fd = INVALID_SOCKET;

    m_send_ptr = NULL;
    m_send_len = 0;
    m_recv_ptr = NULL;
    m_recv_len = 0;

    ev_io_init(&m_ev, NULL, m_fd, EV_NONE);
    m_ev.data = this;
}

channel_base_t::~channel_base_t() 
{
    close();
}


result_t channel_base_t::send(void* ptr, db_int32 len)
{
    if (m_fd == INVALID_SOCKET) 
        return RT_FAILED;

    m_send_ptr = (db_char*)ptr;
    m_send_len = len;

    post_send();
    return RT_SUCCEEDED;
}

result_t channel_base_t::recv(void* ptr, db_int32 len)
{
    if (m_fd == INVALID_SOCKET)
        return RT_FAILED;

    m_recv_ptr = (db_char*)ptr;
    m_recv_len = len;

    post_recv();
    return RT_SUCCEEDED;
}


void channel_base_t::close()
{   
    if (m_fd != INVALID_SOCKET) {
        post_pause();
        ::close(m_fd);
        m_fd = INVALID_SOCKET;
        m_action->on_close();
    }    
}

void channel_base_t::attach(channel_loop_t* loop, db_int32 fd)
{
    m_loop = loop;
    m_fd = fd;    
}


void channel_base_t::loop_send()
{
    assert(m_fd != INVALID_SOCKET);

    while (m_send_len > 0) {
        db_int32 ret = ::send(m_fd, m_send_ptr, m_send_len, 0);
        if (ret > 0) {
            m_send_ptr += ret;
            m_send_len -= ret;

        } else if (ret < 0 && is_noblock_error(errno)) {
            post_send();
            return;

        } else {
            close();
            return;
        }
    }

    post_pause();
    m_action->on_send();
}


void channel_base_t::loop_recv()
{
    assert(m_fd != INVALID_SOCKET);

    while (m_recv_len > 0) {
        db_int32 ret = ::recv(m_fd, m_recv_ptr, m_recv_len, 0);
        if (ret > 0) {
            m_recv_ptr += ret;
            m_recv_len -= ret;

        }
        else if (ret < 0 && is_noblock_error(errno)) {
            post_recv();
            return;

        }
        else {
            close();
            return;
        }
    }

    post_pause();
    m_action->on_recv();
}

void channel_base_t::post_pause()
{
    if (m_ev.events == EV_NONE)
        return;

    assert(m_fd != INVALID_SOCKET);
    struct ev_loop* loop = m_loop->native_handle();
    ev_io_stop(loop, &m_ev);
    ev_io_set(&m_ev, m_fd, EV_NONE);
}


void channel_base_t::post_event(int events)
{
    assert(m_fd != INVALID_SOCKET);
    struct ev_loop* loop = m_loop->native_handle();

    if (m_ev.events == EV_NONE) {
        ev_io_set(&m_ev, m_fd, events);
        ev_io_start(loop, &m_ev);

    } else if (m_ev.events != events) {
        ev_io_stop(loop, &m_ev);
        ev_io_set(&m_ev, m_fd, events);
        ev_io_start(loop, &m_ev);

    } else {}
        
}




listen_channel_t::listen_channel_t(listen_action_t* action) : 
    channel_base_t(this)
{
    m_action = action;
}

listen_channel_t::~listen_channel_t()
{

}

result_t listen_channel_t::listen(channel_loop_t* loop, db_uint16 port)
{
    db_int32 fd = ::socket(PF_INET, SOCK_STREAM, 0);
    IF_RETURN_FAILED(fd <= 0);

    attach(loop, fd);
    
    set_noblock_socket(fd);
    set_nodelay_socket(fd);
    set_reuseaddr_socket(fd);
     
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    db_int32 ret;
    ret = ::bind(fd, (sockaddr*)&addr, sizeof(addr));
    IF_RETURN_FAILED(ret != 0);
    
    ret = ::listen(fd, BACKLOG);
    IF_RETURN_FAILED(ret != 0);
        
    post_recv();
    return RT_SUCCEEDED;
}

void listen_channel_t::on_send()
{    
}


void listen_channel_t::on_recv()
{
    DB_TRACE("listen_channel_t::on_recv");

    sockaddr_in addr;
    socklen_t addr_len = 0;
    db_int32 fd = ::accept(socket(), (sockaddr*)&addr, &addr_len);
    if (fd == INVALID_SOCKET)
        return;
        
    m_action->on_accept(fd, addr);
    post_recv();
}


void listen_channel_t::on_close()
{
    DB_TRACE("listen_channel_t::on_close");
}



