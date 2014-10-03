// network_service.cpp by scott.zgeng@gmail.com 2014.09.15

#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "network_service.h"
#include "packet.h"
#include "database.h"

// TODO(scott.zgeng): 
// 线程模型需要修改一下，在startup阶段，最好都是在主线程完成，
// 不需要立即创建新线程，只有当鉴权通过后，才创建线程

server_session_t::server_session_t() : m_channel(this)
{
    memset(&m_client_addr, 0, sizeof(m_client_addr));

    m_pack_type = 0;
    m_is_startup = true;
    m_is_header = true;
    m_send_buff[0] = 0;
    m_recv_buff[0] = 0;
    m_send_action = this; // 自动恢复到缺省动作
    m_recv_action = this;
}

server_session_t::~server_session_t()
{

}


void server_session_t::init(channel_loop_t* loop, db_int32 fd, const sockaddr_in& addr)
{
    m_pack_type = 0;
    m_is_startup = true;
    m_is_header = true;

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
    recv_packet(this);    
}


void server_session_t::recv_packet(session_recv_action_t* action)
{
    m_is_header = true;
    m_recv_action = action;
    m_channel.recv(m_recv_buff, HEAD_SIZE);
}


result_t server_session_t::send_packet(opacket_t& packet, session_send_action_t* action)
{
    packet_ostream_t osteam1(m_send_buff, HEAD_SIZE);

    osteam1.write_int8(packet.type());

    packet_ostream_t osteam2(m_send_buff + HEAD_SIZE, MAX_SEND_BUF_SIZE - HEAD_SIZE);
    result_t ret = packet.encode(osteam2);
    IF_RETURN_FAILED(ret != RT_SUCCEEDED);

    db_uint32 packet_len = osteam2.length();
    osteam1.write_int32(packet_len + sizeof(db_int32));

    m_send_action = action;
    m_channel.send(m_send_buff, packet_len + HEAD_SIZE);

    return RT_SUCCEEDED;
}

result_t server_session_t::send_packet(opacket_t& packet)
{
    return send_packet(packet, this);
}


result_t server_session_t::send_packets(packet_vector_t& packets)
{
    m_send_action = this;

    db_char* pos = m_send_buff;
    db_uint32 capacity = MAX_SEND_BUF_SIZE;

    for (db_uint32 i = 0; i < packets.size(); i++) {
        packet_ostream_t osteam1(pos, HEAD_SIZE);
        osteam1.write_int8(packets[i]->type());

        packet_ostream_t osteam2(pos + HEAD_SIZE, capacity - HEAD_SIZE);
        result_t ret = packets[i]->encode(osteam2);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);
        

        db_uint32 packet_len = osteam2.length() + sizeof(db_int32);
        osteam1.write_int32(packet_len);

        packet_len += sizeof(db_int8); // 加上头的一个字节

        pos += packet_len;
        capacity -= packet_len;

        assert(capacity >= 0); // 先简单这么做，后面再完善
    }

    m_channel.send(m_send_buff, pos - m_send_buff);
    return RT_SUCCEEDED;
}



result_t server_session_t::send_packet_with_end(opacket_t& packet)
{
    packet_vector_t packets;
    packets.push_back(&packet);

    read_for_query_opacket_t<true> end_packet;
    packets.push_back(&end_packet);

    return send_packets(packets);
}


void server_session_t::on_send()
{
    DB_TRACE("server_session_t::on_send");    
    m_send_action->on_send_complete(this);
}


result_t server_session_t::on_send_complete(server_session_t* session)
{
    DB_TRACE("server_session_t::on_send_complete");
    recv_packet();
    return RT_SUCCEEDED;
}


result_t server_session_t::on_recv_complete(server_session_t* session, db_int8 type, packet_istream_t& stream)
{
    DB_TRACE("server_session_t::on_recv_complete");
    ipacket_t* packet = ipacket_t::create_packet(type);
    IF_RETURN_FAILED(packet == NULL);

    result_t ret = packet->decode(stream);
    IF_RETURN_FAILED(ret != RT_SUCCEEDED);


    ret = packet->process(this);
    IF_RETURN_FAILED(ret != RT_SUCCEEDED);

    return RT_SUCCEEDED;
}


result_t server_session_t::on_recv_head()
{
    packet_istream_t stream(m_recv_buff, HEAD_SIZE);

    if LIKELY(!m_is_startup) {
        m_pack_type = stream.read_int8();
    } else {
        m_pack_type = 0;
        m_is_startup = false;
    }

    m_pack_len = (db_uint32)stream.read_int32();
    IF_RETURN_FAILED(m_pack_len > MAX_RECV_BUF_SIZE);
    m_pack_len -= sizeof(db_int32);
    m_is_header = false;

    if (m_pack_len > 0) {
        m_channel.recv(m_recv_buff, m_pack_len);
        return RT_SUCCEEDED;

    } else { // 有些只有头没有后面内容
        packet_istream_t stream(m_recv_buff, m_pack_len);
        return m_recv_action->on_recv_complete(this, m_pack_type, stream);
    }    
}




void server_session_t::on_recv()
{
    DB_TRACE("server_session_t::on_recv");

    result_t ret;
    if (m_is_header) {
        ret = on_recv_head();        
    } else {
        packet_istream_t stream(m_recv_buff, m_pack_len);
        ret = m_recv_action->on_recv_complete(this, m_pack_type, stream);
    }
    
    if (ret != RT_SUCCEEDED) {
        m_channel.close();
        return;
    }    
}


void server_session_t::on_close()
{
    DB_TRACE("server_session_t::on_close");
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




result_t network_service::init()
{
    result_t ret;
    db_uint16 server_port = 19992;  // TODO(scott.zgeng): 需要从配置中获取
    
    ret = database_t::instance.init();
    IF_RETURN_FAILED(ret != RT_SUCCEEDED);

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
