// network_service.cpp by scott.zgeng@gmail.com 2014.09.15

#include <string.h>
#include "network_service.h"
#include "packet.h"






server_session_t::server_session_t() : m_channel(this)
{
    memset(&m_client_addr, 0, sizeof(m_client_addr));

    m_is_startup = true;
    m_is_header = true;
    m_send_buff[0] = 0;
    m_recv_buff[0] = 0;
}

server_session_t::~server_session_t()
{

}


void server_session_t::init(channel_loop_t* loop, db_int32 fd, const sockaddr_in& addr)
{
    m_client_addr = addr;
    m_channel.attach(loop, fd);
}


void server_session_t::recv_startup()
{
    m_is_header = true;
    m_channel.recv(m_recv_buff, sizeof(db_int32));
}


void server_session_t::recv_packet()
{
    m_is_header = true;
    m_channel.recv(m_recv_buff, HEAD_SIZE);
}



void server_session_t::on_send()
{
    recv_packet();
}


result_t server_session_t::on_recv_head()
{
    packet_istream_t stream(m_recv_buff, HEAD_SIZE);

    if LIKELY(!m_is_startup) {
        m_pack_type = stream.read_int8();
    } else {
        m_is_startup = false;
    }

    m_pack_len = stream.read_int32();
    IF_RETURN_FAILED(m_pack_len > MAX_RECV_BUF_SIZE);
    m_pack_len -= sizeof(db_int32);
    m_is_header = false;
    m_channel.recv(m_recv_buff, m_pack_len);
    return RT_SUCCEEDED;
}


result_t server_session_t::on_recv_packet()
{    
    packet_istream_t istream(m_recv_buff, m_pack_len);

    in_packet_t* packet = in_packet_t::create_packet(m_pack_type);
    IF_RETURN_FAILED(packet == NULL);

    result_t ret = packet->decode(istream);
    IF_RETURN_FAILED(ret != RT_SUCCEEDED);

    packet_ostream_t osteam(m_send_buff, MAX_SEND_BUF_SIZE);
    ret = packet->process(this, osteam);
    IF_RETURN_FAILED(ret != RT_SUCCEEDED);

    return RT_SUCCEEDED;  
}


void server_session_t::on_recv()
{
    result_t ret = m_is_header ? on_recv_head() : on_recv_packet();
    if (ret != RT_SUCCEEDED) {
        m_channel.close();
        return;
    }
}



void server_session_t::on_close()
{

}



worker_thread_t::worker_thread_t()
{
    m_stop = false;
}

worker_thread_t::~worker_thread_t()
{
    m_stop = true;
}


result_t worker_thread_t::start(db_int32 fd, const sockaddr_in& addr)
{
    result_t ret;

    m_session.init(&m_loop, fd, addr);

    ret = m_loop.init();
    IF_RETURN_FAILED(ret != RT_SUCCEEDED);

    ret = m_thread.start(this);
    IF_RETURN_FAILED(ret != RT_SUCCEEDED);

    return RT_SUCCEEDED;
}

void worker_thread_t::run()
{       
    m_session.recv_startup();

    while (!m_stop) {
        m_loop.run_once();
    }
}


network_service::network_service() : 
    m_listener(this)
{
    m_stop = false;
    memset(m_workers, 0, sizeof(m_workers));

}

network_service::~network_service()
{
    m_stop = true;
}


#include <sys/stat.h>
#include <sys/types.h>

result_t network_service::init()
{
    result_t ret;
    db_uint16 server_port = 19992;  // TODO(scott.zgeng): ��Ҫ�������л�ȡ

    ret = m_loop.init();
    IF_RETURN_FAILED(ret != RT_SUCCEEDED);

    ret = m_listener.listen(&m_loop, server_port);
    IF_RETURN_FAILED(ret != RT_SUCCEEDED);

    
    return RT_SUCCEEDED;
}


void network_service::on_accept(db_int32 fd, const sockaddr_in& addr)
{
    worker_thread_t* worker = find_unused_worker();
    if (worker == NULL) {
        close(fd);
        return;
    }
    
    if (worker->start(fd, addr) != RT_SUCCEEDED) {
        close(fd);
        return;
    }
}


worker_thread_t* network_service::find_unused_worker()
{
    for (db_uint32 i = 0; i < MAX_THREAD_NUM; i++) {
        if (m_workers[i] == NULL)  {
            m_workers[i] = new worker_thread_t();
            return m_workers[i];
        }
    }

    return NULL;
}


void network_service::run()
{
    while (!m_stop) {
        m_loop.run_once();
    }
}
