// packet.h by scott.zgeng@gmail.com 2014.09.21


#ifndef  __PACKET_H__
#define  __PACKET_H__


#include "define.h"


class packet_istream_t
{
public:    
    packet_istream_t(db_char* ptr, db_uint32 capacity);
    ~packet_istream_t();

    DISALLOW_COPY_AND_ASSIGN(packet_istream_t);

public:
    db_int8 read_int8();
    db_int16 read_int16();
    db_int32 read_int32();
    db_char* read_string(); // warning: no copy

    bool is_eof() { return m_left_len == 0; }
private:
    db_char* m_pos;
    db_uint32 m_left_len; 
};




class packet_ostream_t
{    
public:
    packet_ostream_t(db_char* ptr, db_uint32 capacity);
    ~packet_ostream_t();

    DISALLOW_COPY_AND_ASSIGN(packet_ostream_t);
    
public:
    result_t write_int8(db_int8 val);
    result_t write_int16(db_int16 val);
    result_t write_int32(db_int32 val);
    result_t write_string(db_char* val);

    bool is_eof() { return m_left_len == 0; }

private:
    db_char* m_ptr;
    db_char* m_pos;
    db_uint32 m_left_len;
};


class server_session_t;
class in_packet_t
{
public:
    static in_packet_t* create_packet(db_int8 type);

public:
    virtual ~in_packet_t() {}

public:
    virtual result_t decode(packet_istream_t& stream) = 0;
    virtual result_t process(server_session_t* session, packet_ostream_t& stream) = 0;
};


class out_packet_t
{
public:
    virtual ~out_packet_t() {}
public:
    virtual result_t encode(packet_ostream_t& stream) = 0;
};


class auth_ok_opacket_t : public out_packet_t
{
public:
    virtual ~auth_ok_opacket_t();
    virtual result_t encode(packet_ostream_t& stream);
};


class startup_ipacket_t : public in_packet_t
{
public:
    static const db_int32 PROTOCOL_VERSION = 196608;
    
public:
    virtual result_t decode(packet_istream_t& stream);
    virtual result_t process(server_session_t* session, packet_ostream_t& stream);

public:
    db_int32 protocol_version;
    db_char* user;
    db_char* database;    
    db_char* options;
};




#endif //__PACKET_H__

