// packet.h by scott.zgeng@gmail.com 2014.09.21


#ifndef  __PACKET_H__
#define  __PACKET_H__

#include <stdlib.h>
#include "define.h"
#include "variant.h"
#include "pod_vector.h"

#define INIT_COLUMN_SIZE (1024)
typedef pod_vector<db_char*, INIT_COLUMN_SIZE> row_data_t;



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
    result_t write_string(const db_char* val);
    result_t write_bytes(const db_byte* val, db_uint32 len);

    bool is_eof() { return m_left_len == 0; }
    db_uint32 length() { return m_pos - m_ptr; }

private:
    db_char* m_ptr;
    db_char* m_pos;
    db_uint32 m_left_len;
};


class server_session_t;


// TODO(scott.zgeng): ipacket， opacket名字不太好，后面考虑换个名字
// ipacket_t => recv_msg_handle_t ?
// opacket_t => send_msg_handle_t ?



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



class error_opacket_t : public opacket_t
{
public:
    error_opacket_t(const db_char* msg) {
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
    const db_char* error_message;
};



class param_status_opacket_t :public opacket_t
{
public:
    param_status_opacket_t(const char* name, const char* value) {
        // TODO(scott.zgeng): 字符串考虑后面统一改一下
        strcpy(m_name, name);
        strcpy(m_value, value);
    }

    virtual db_int8 type() {
        return 'S';
    }

    virtual result_t encode(packet_ostream_t& stream) {
        stream.write_string(m_name);
        stream.write_string(m_value);
        return RT_SUCCEEDED;
    }

    
    char m_name[256];
    char m_value[256];
};

class complete_opacket_t : public opacket_t
{
public:
    complete_opacket_t(db_uint32 rows) {
        sprintf(tag, "%d", rows);        
    }

    virtual db_int8 type() {
        return 'C';
    }

    virtual result_t encode(packet_ostream_t& stream) {        
        stream.write_string(tag);

        return RT_SUCCEEDED;
    }

    db_char tag[1024];
};



#include "fstring.h"
class row_desc_opacket_t : public opacket_t
{
public:
    virtual db_int8 type() {
        return 'T';
    }

    virtual result_t encode(packet_ostream_t& stream) {
        db_int16 num = (db_int16)m_rows_desc.size();        
        stream.write_int16(num);

        for (db_int16 i = 0; i < num; i++) {
            row_desc_item_t& item = m_rows_desc[i];
            stream.write_string(item.name.c_str());
            stream.write_int32(0); // table_id
            stream.write_int16(0); // column_id
            stream.write_int32(item.type); // data_type
            stream.write_int16(item.size); // data_size
            stream.write_int32(0); // modifier
            stream.write_int16(0); // format code
        }

        return RT_SUCCEEDED;
    }


    struct row_desc_item_t {        
        fstring<64> name;
        db_int32 type;
        db_int16 size;
    };

    db_bool add_row_desc(const char* name, db_int32 type, db_uint32 size) {
        row_desc_item_t item;
        item.name = name;
        item.type = type;
        item.size = (db_int16)size;
        return m_rows_desc.push_back(item);
    }

private:
    pod_vector<row_desc_item_t, 36> m_rows_desc;
};


class data_row_opacket_t : public opacket_t
{
public:
    virtual db_int8 type() {
        return 'D';
    }

    virtual result_t encode(packet_ostream_t& stream) {
        db_int16 count = (db_int16)variants.size();
        stream.write_int16(count);

        db_char buff[128];

        for (db_int16 i = 0; i < count; i++) {
            variants[i].to_string(buff);
            db_int32 len = strlen(buff) + 1;
            stream.write_int32(len);
            stream.write_string(buff);
        }
               
        return RT_SUCCEEDED;            
    }

    bool add_row_data(const variant_t& val) {
        return variants.push_back(val);
    }

    typedef pod_vector<variant_t, 64> variant_vector_t;
    variant_vector_t variants;
};


class column_table_t;
class copy_in_opacket_t : public opacket_t
{
public:
    
    copy_in_opacket_t(db_bool is_binary, db_int32 column_count) {
        m_is_binary = is_binary ? 1 : 0;
        m_column_count = (db_int16)column_count;
    }
    virtual db_int8 type() {
        return 'G';
    }

    virtual result_t encode(packet_ostream_t& stream) {
        stream.write_int8(m_is_binary);        
        stream.write_int16(m_column_count);

        for (db_int16 i = 0; i < m_column_count; i++) {
            stream.write_int16((db_int16)m_is_binary);
        }

        return RT_SUCCEEDED;
    }

    
private:
    // 0 indicates the overall COPY format is textual(rows separated by newlines, columns separated by separator characters, etc). 
    // 1 indicates the overall copy format is binary(similar to DataRow format).
    db_int8 m_is_binary; 
    db_int16 m_column_count;
    
};





class startup_ipacket_t : public ipacket_t
{
public:
    static const db_int32 PROTOCOL_VERSION = 196608;
    
public:
    virtual result_t decode(packet_istream_t& stream);
    virtual result_t process(server_session_t* session);

private:
    db_int32 m_protocol_version;
    db_char* m_user;
    db_char* m_database;    
    db_char* m_options;
};

class password_ipacket_t : public ipacket_t
{
public:
    virtual result_t decode(packet_istream_t& stream);
    virtual result_t process(server_session_t* session);

public:
    char* m_password;
};



struct sqlite3_stmt;

class session_send_action_t
{
public:    
    virtual ~session_send_action_t() {}
    virtual result_t on_send_complete(server_session_t* session) = 0;
    
};


class session_recv_action_t
{
public:
    virtual ~session_recv_action_t() {}
    virtual result_t on_recv_complete(server_session_t* session, db_int8 type, packet_istream_t& stream) = 0;
};




class command_action_t: public session_send_action_t
{
public:
    static command_action_t* create_command(const char* sql);
public:
    virtual ~command_action_t() {}
    virtual result_t execute(server_session_t* session, const char* sql) = 0;
    
};


class copy_in_action_t : public command_action_t, session_recv_action_t
{
public:
    copy_in_action_t();
    virtual ~copy_in_action_t();
public:
    virtual result_t execute(server_session_t* session, const char* sql);
    virtual result_t on_send_complete(server_session_t* session);
    virtual result_t on_recv_complete(server_session_t* session, db_int8 type, packet_istream_t& stream);

private:
    column_table_t* m_table;
};


class simple_query_action_t : public command_action_t
{
public:
    simple_query_action_t();
    virtual ~simple_query_action_t();

public:
    virtual result_t execute(server_session_t* session, const char* sql);
    virtual result_t on_send_complete(server_session_t* session);

private:
    sqlite3_stmt* m_stmt;
    db_int32 m_segment_row_count;
    db_int32 m_row_idx;
    db_int32 m_total_count;
};


class query_ipacket_t : public ipacket_t
{
public:
    query_ipacket_t();
    virtual ~query_ipacket_t() {}

public:
    virtual result_t decode(packet_istream_t& stream);
    virtual result_t process(server_session_t* session);

private:
    char* m_sql;
};




class copy_data_ipacket_t : public ipacket_t
{
public:
    copy_data_ipacket_t();
    virtual ~copy_data_ipacket_t() {}

public:
    virtual result_t decode(packet_istream_t& stream);
    virtual result_t process(server_session_t* session); 

    void gen_row_data(row_data_t& row_data, db_uint32 column_count);    

private:
    db_char* m_row;
    db_char* m_save_pos;
};



class copy_done_ipacket_t : public ipacket_t
{
public:
    virtual result_t decode(packet_istream_t& stream) { return RT_SUCCEEDED; }
    virtual result_t process(server_session_t* session) { return RT_SUCCEEDED; }
};

 

#endif //__PACKET_H__



