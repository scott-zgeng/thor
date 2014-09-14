// channel.cpp by scott.zgeng@gmail.com 2014.09.14

#include <assert.h>
#include "channel.h"




void channel_base_t::on_ev_write(struct ev_loop* loop, ev_io* ev, int events)
{
    DB_TRACE("channel_base_t::on_ev_write");

    channel_base_t* channel = (channel_base_t*)ev->data;
    channel->m_handle->on_send();
}

void channel_base_t::on_ev_read(struct ev_loop* loop, ev_io* ev, int events)
{
    DB_TRACE("channel_base_t::on_ev_read");

    channel_base_t* channel = (channel_base_t*)ev->data;
    channel->m_handle->on_recv();
}


channel_base_t::channel_base_t(channel_loop_t* loop, channel_handle_t* handle)
{
    m_loop = loop;
    m_handle = handle;
    memset(&m_addr, 0, sizeof(m_addr));
    m_fd = INVALID_SOCKET;

    m_write_buff = NULL;
    m_write_len = 0;
    m_read_buff = NULL;
    m_read_len = 0;

    ev_io_init(&m_ev, NULL, m_fd, EV_NONE);
    m_ev.data = this;
}

channel_base_t::~channel_base_t() 
{
    close();
}


void channel_base_t::send(void* buff, db_uint32 len)
{
    m_write_buff = (db_byte*)buff;
    m_write_len = len;

    loop_write();
}

void channel_base_t::recv(void* buff, db_uint32 len)
{
    m_read_buff = (db_byte*)buff;
    m_read_len = len;

    loop_read();
}

void channel_base_t::close()
{   
    if (m_fd != INVALID_SOCKET) {
        set_none();
        close_socket(m_fd);
        m_fd = INVALID_SOCKET;
        m_handle->on_close();
    }    
}

void channel_base_t::attach(socket_handle fd, const sockaddr_in& addr)
{
    m_fd = fd;
    memcpy(&m_addr, &addr, sizeof(m_addr));
}


void channel_base_t::loop_write()
{
    assert(m_fd != INVALID_SOCKET);




}

void channel_base_t::loop_read()
{

}

void channel_base_t::set_mode(int events)
{
    assert(m_fd != INVALID_SOCKET);

    if (events != m_ev.events) {
        ev_io_stop(m_loop->ptr(), &m_ev);
    }

    ev_io_set(&m_ev, m_fd, events);
    ev_io_start(m_loop->ptr(), &m_ev);
}



void channel_base_t::set_none()
{
    assert(m_fd != INVALID_SOCKET);

    if (m_ev.events == EV_NONE)
        return;

    ev_io_stop(m_loop->ptr(), &m_ev);
    ev_io_set(&m_ev, m_fd, EV_NONE);
}



listen_channel_t::listen_channel_t(channel_loop_t* loop) : channel_base_t(loop, this)
{
    

}

void fasda()
{
    ///m_fd = socket(PF_INET, SOCK_STREAM, 0);
    //IF_RETURN_FAILED(m_fd <= 0);

    //m_addr.sin_family = AF_INET;
    //m_addr.sin_port = htons(port);
    //m_addr.sin_addr.s_addr = INADDR_ANY;

    //ret = bind(m_fd, (sockaddr*)&m_addr, sizeof(m_addr));
    //IF_RETURN_FAILED(ret != 0);

    //ret = listen(m_fd, BACKLOG);
    //IF_RETURN_FAILED(ret != 0);

    //return RT_SUCCEEDED;
}

channel_loop_t::channel_loop_t()
{
    m_loop = NULL;
}

channel_loop_t::~channel_loop_t()
{
    if (m_loop != NULL) {
        ev_loop_destroy(m_loop);
    }
    
}

result_t channel_loop_t::init()
{
    m_loop = ev_loop_new();
    IF_RETURN_FAILED(m_loop == NULL);

    return RT_SUCCEEDED;
}



