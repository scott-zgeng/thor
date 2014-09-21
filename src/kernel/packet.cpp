// packet.cpp by scott.zgeng@gmail.com 2014.09.21

#include <assert.h>
#include <arpa/inet.h>
#include "packet.h"
#include "network_service.h"



packet_istream_t::packet_istream_t(db_char* ptr, db_uint32 capacity)
{
    m_pos = ptr;
    m_left_len = capacity;    
}


packet_istream_t::~packet_istream_t()
{
}


db_int8 packet_istream_t::read_int8()
{
    db_int8 val = *(db_int8*)m_pos;
    m_pos += sizeof(db_int8);
    m_left_len -= sizeof(db_int8);
    assert(m_left_len >= 0);
    return val;
}

db_int16 packet_istream_t::read_int16()
{
    db_int16 val = *(db_int16*)m_pos;
    m_pos += sizeof(db_int16);
    m_left_len -= sizeof(db_int16);
    assert(m_left_len >= 0);
    return ntohs(val);
}

db_int32 packet_istream_t::read_int32()
{
    db_int32 val = *(db_int32*)m_pos;
    m_pos += sizeof(db_int32);
    m_left_len -= sizeof(db_int32);
    assert(m_left_len >= 0);
    return ntohl(val);
}

db_char* packet_istream_t::read_string()
{
    db_char* val = m_pos;
    db_uint32 len = strnlen(val, m_left_len);
    m_pos += len;
    m_left_len -= len;
    assert(m_left_len >= 0);
    return val;
}




packet_ostream_t::packet_ostream_t(db_char* ptr, db_uint32 capacity)
{
    m_ptr = ptr;
    m_pos = ptr;    
    m_left_len = capacity;
}


packet_ostream_t::~packet_ostream_t()
{
}


result_t packet_ostream_t::write_int8(db_int8 val)
{

}
result_t packet_ostream_t::write_int16(db_int16 val)
{

}
result_t packet_ostream_t::write_int32(db_int32 val)
{

}
result_t packet_ostream_t::write_string(db_char* val)
{

}





in_packet_t* in_packet_t::create_packet(db_int8 type)
{
    switch (type)
    {
    case 0:
        return new startup_ipacket_t();
    default:
        return NULL;
    }
}


auth_ok_opacket_t::~auth_ok_opacket_t()
{

}

result_t auth_ok_opacket_t::encode(packet_ostream_t& stream)
{
    return RT_SUCCEEDED;
}

result_t startup_ipacket_t::decode(packet_istream_t& stream)
{
    protocol_version = stream.read_int32();

    while (!stream.is_eof()) {
        db_char* param_name = stream.read_string();
        if (strcmp(param_name, "user") == 0)
            user = stream.read_string();
        else if (strcmp(param_name, "database") == 0)
            database = stream.read_string();
        else {            
            options = stream.read_string();
        }            
    }

    return RT_SUCCEEDED;
}


result_t startup_ipacket_t::process(server_session_t* session, packet_ostream_t& stream)
{
    IF_RETURN_FAILED(protocol_version != PROTOCOL_VERSION);

    channel_base_t* channel = session->channel();
    
    auth_ok_opacket_t packet;
    result_t ret = packet.encode(stream);
    

    return RT_SUCCEEDED;
}



