// packet.h by scott.zgeng@gmail.com 2014.09.21


#ifndef  __PACKET_H__
#define  __PACKET_H__

#include <stdlib.h>
#include "define.h"
#include "pod_vector.h"




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
    result_t write_bytes(db_byte* val, db_uint32 len);

    bool is_eof() { return m_left_len == 0; }
    db_uint32 length() { return m_pos - m_ptr; }

private:
    db_char* m_ptr;
    db_char* m_pos;
    db_uint32 m_left_len;
};


class server_session_t;
class ipacket_t
{
public:
    static ipacket_t* create_packet(db_int8 type);

public:
    virtual ~ipacket_t() {}

public:    
    virtual result_t decode(packet_istream_t& stream) = 0;
    virtual result_t process(server_session_t* session) = 0;
};


class opacket_t
{
public:
    virtual ~opacket_t() {}
public:
    virtual db_int8 type() = 0;
    virtual result_t encode(packet_ostream_t& stream) = 0;
};


typedef pod_vector<opacket_t*, 128> packet_vector_t;

#define AuthenticationOk 0
#define AuthenticationKerberosV5 2
#define AuthenticationCleartextPassword 3
#define AuthenticationMD5Password 5
#define AuthenticationSCMCredential  6
#define AuthenticationGSS 7
#define AuthenticationGSSContinue 8
#define AuthenticationSSPI 9


template<db_int32 AUTH_TYPE>
class auth_opacket_t : public opacket_t
{
public:
    static const db_int32 auth_type = AUTH_TYPE;

public:
    auth_opacket_t() {}
    virtual ~auth_opacket_t() {}
public:
    virtual db_int8 type() { 
        return 'R'; 
    }

    virtual result_t encode(packet_ostream_t& stream) {
        stream.write_int32(auth_type);
        return RT_SUCCEEDED;
    }
};


class auth_md5_opacket_t : public auth_opacket_t<AuthenticationMD5Password>
{
public:
    auth_md5_opacket_t() {
        db_int32 temp = rand();
        memcpy(salt, &temp, sizeof(salt));
    }

    virtual result_t encode(packet_ostream_t& stream) {
        stream.write_int32(auth_type);
        stream.write_bytes(salt, sizeof(salt));
        return RT_SUCCEEDED;
    }

    db_byte salt[4];
};


template<bool IS_IDLE>
class read_for_query_opacket_t : public opacket_t
{
public:
    read_for_query_opacket_t() {
        status = IS_IDLE ? 'I' : 'E';
    }
    virtual ~read_for_query_opacket_t() {}
public:

    virtual db_int8 type() {
        return 'Z';
    }

    virtual result_t encode(packet_ostream_t& stream) {
        stream.write_int8(status);
        return RT_SUCCEEDED;
    }

    db_byte status;
};



class error_ipacket_t : public opacket_t
{
public:
    error_ipacket_t(db_char* msg) {
        error_type = 'D';
        error_message = msg;
    }

    virtual db_int8 type() {
        return 'E';
    }

    virtual result_t encode(packet_ostream_t& stream) {
        stream.write_int8(error_type);
        stream.write_string(error_message);

        return RT_SUCCEEDED;
    }

    db_byte error_type;
    db_char* error_message;
};


class startup_ipacket_t : public ipacket_t
{
public:
    static const db_int32 PROTOCOL_VERSION = 196608;
    
public:
    virtual result_t decode(packet_istream_t& stream);
    virtual result_t process(server_session_t* session);

public:
    db_int32 protocol_version;
    db_char* user;
    db_char* database;    
    db_char* options;
};

class password_ipacket_t : public ipacket_t
{
public:
    virtual result_t decode(packet_istream_t& stream);
    virtual result_t process(server_session_t* session);

public:
    char* password;
};



struct sqlite3_stmt;
class query_ipacket_t :public ipacket_t
{
public:
    virtual result_t decode(packet_istream_t& stream);
    virtual result_t process(server_session_t* session);
public:
    char* sql;
    sqlite3_stmt* stmt;
};



#endif //__PACKET_H__

