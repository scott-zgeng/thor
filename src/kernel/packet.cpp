// packet.cpp by scott.zgeng@gmail.com 2014.09.21

#include <assert.h>
#include <arpa/inet.h>

#include "packet.h"
#include "network_service.h"
#include "database.h"
#include "parse_ctx.h"
#include "project_node.h"


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
    db_uint32 len = strlen(val) + 1;
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
    *(db_int8*)m_pos = val;
    m_pos += sizeof(db_int8);
    m_left_len -= sizeof(db_int8);
    assert(m_left_len >= 0);
    return RT_SUCCEEDED;
}

result_t packet_ostream_t::write_int16(db_int16 val)
{
    *(db_int16*)m_pos = htons(val);
    m_pos += sizeof(db_int16);
    m_left_len -= sizeof(db_int16);
    assert(m_left_len >= 0);
    return RT_SUCCEEDED;
}

result_t packet_ostream_t::write_int32(db_int32 val)
{
    *(db_int32*)m_pos = htonl(val);
    m_pos += sizeof(db_int32);
    m_left_len -= sizeof(db_int32);
    assert(m_left_len >= 0);
    return RT_SUCCEEDED;
}


result_t packet_ostream_t::write_string(const db_char* val)
{
    strcpy(m_pos, val);
    db_uint32 len = strlen(val) + 1;
    m_pos += len;
    m_left_len -= len;
    assert(m_left_len >= 0);
    return RT_SUCCEEDED;
}


result_t packet_ostream_t::write_bytes(const db_byte* val, db_uint32 len)
{
    memcpy(m_pos, val, len);
    m_pos += len;
    m_left_len -= len;
    assert(m_left_len >= 0);
    return RT_SUCCEEDED;
}


ipacket_t* ipacket_t::create_packet(db_int8 type)
{
    DB_TRACE("create_packet = %d(%c)", type, type);
    switch (type)
    {
    case 0:
        return new startup_ipacket_t();
    case 'p':
        return new password_ipacket_t();
    case 'Q':
        return new query_ipacket_t();
    default:
        return NULL;
    }
}




result_t startup_ipacket_t::decode(packet_istream_t& stream)
{
    protocol_version = stream.read_int32();

    while (!stream.is_eof()) {
        db_char* param_name = stream.read_string();

        if (strcmp(param_name, "") == 0)
            break;

        if (strcmp(param_name, "user") == 0)
            user = stream.read_string();
        else if (strcmp(param_name, "database") == 0)
            database = stream.read_string();
        else {                        
            // application_name
            // client_encoding
            // ...
            (void)stream.read_string();
        }            
    }

    return RT_SUCCEEDED;
}


result_t startup_ipacket_t::process(server_session_t* session)
{
    IF_RETURN_FAILED(protocol_version != PROTOCOL_VERSION);
    
    auth_md5_opacket_t packet;
    session->send_packet(packet);
    
    return RT_SUCCEEDED;
}


result_t password_ipacket_t::decode(packet_istream_t& stream)
{
    password = stream.read_string();
    return RT_SUCCEEDED;
}
 

result_t password_ipacket_t::process(server_session_t* session)
{
    // TODO(scott.zgeng): 需要增加对密码的校验，目前先简化这个操作，直接返回成功
    
    auth_opacket_t<AuthenticationOk> packet;    
    return session->send_packet_with_end(packet);
}
  

query_ipacket_t::query_ipacket_t()
{
    sql = NULL;
    stmt = NULL;
    segment_row_count = 0;
    total_count = 0;
}


result_t query_ipacket_t::decode(packet_istream_t& stream)
{
    sql = stream.read_string();
    return RT_SUCCEEDED;
}

db_bool query_ipacket_t::is_copy_command(const db_char* sql) const
{
    return strncmp(sql, "copy", 4) == 0;

}

result_t query_ipacket_t::process_copy(server_session_t* session, const char* sql)
{

}


result_t query_ipacket_t::process(server_session_t* session)
{
    const char* tail;
    db_int32 len = strlen(sql);

    if (is_copy_command(sql)) {
        return process_copy(session, sql);
    }

    db_int32 sqlite_ret = sqlite3_vector_prepare(database_t::instance.native_handle(), sql, len, &stmt, &tail);
    if (sqlite_ret != SQLITE_OK) { 
        error_opacket_t error_packet("sqlite3_vector_prepare failed");
        session->send_packet_with_end(error_packet);
        return RT_SUCCEEDED;
    }

    sqlite_ret = sqlite3_vector_step(stmt);
    if (sqlite_ret == SQLITE_ERROR) {
        error_opacket_t error_packet("sqlite3_vector_step failed");
        session->send_packet_with_end(error_packet);
        return RT_SUCCEEDED;
    }

    if (sqlite_ret == SQLITE_DONE) {
        complete_opacket_t comp_packet(1);
        session->send_packet_with_end(comp_packet);
        return RT_SUCCEEDED;
    }

    // sqlite_ret = SQLITE_ROW
    segment_row_count = sqlite3_vector_row_count(stmt);
    total_count += segment_row_count;
    row_idx = 0;

    row_desc_opacket_t row_head_packet;
    project_node_t* node = get_statement_root_node(stmt);
    for (db_uint32 i = 0; i < node->column_count(); i++) {        
        row_head_packet.add_row_desc(node->column_name(i), node->column_type(i), node->column_size(i));
    }           

    session->send_packet(row_head_packet, this);
    return RT_SUCCEEDED;
}


void query_ipacket_t::on_send_complete(server_session_t* session)
{
    if (row_idx == segment_row_count) {
        db_int32 sqlite_ret = sqlite3_vector_step(stmt);
        if (sqlite_ret == SQLITE_ERROR) {
            error_opacket_t packet("sqlite3_vector_step failed");
            session->send_packet_with_end(packet);
            return;
        }

        if (sqlite_ret == SQLITE_DONE) {
            complete_opacket_t packet(total_count);
            session->send_packet_with_end(packet);
            return;
        }

        // case SQLITE_ROW
        segment_row_count = sqlite3_vector_row_count(stmt);
        total_count += segment_row_count;
        row_idx = 0;
        assert(segment_row_count > 0);
    }

    
    data_row_opacket_t data_packet;
    db_int32 col_num = sqlite3_vector_column_count(stmt);
    for (db_int32 i = 0; i < col_num; i++) {
        const variant_t& val = sqlite3_vector_column_variant(stmt, i, row_idx);
        data_packet.add_row_data(val);
    }
    row_idx++;
    session->send_packet(data_packet, this);
}

