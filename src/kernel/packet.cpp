// packet.cpp by scott.zgeng@gmail.com 2014.09.21

#include <assert.h>
#include <arpa/inet.h>

#include "packet.h"
#include "network_service.h"
#include "database.h"
#include "parse_ctx.h"
#include "project_node.h"
//#include "column.h"

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



void copy_in_opacket_t::init_columns(column_table_t* table)
{
    db_uint32 count = table->get_column_count();
    for (db_uint32 i = 0; i < count; i++) {
        column_base_t* column = table->get_column(i);
        db_int16 col_type = column->data_type();
        m_column_types.push_back(col_type);
    }    
}


result_t startup_ipacket_t::decode(packet_istream_t& stream)
{
    m_protocol_version = stream.read_int32();

    while (!stream.is_eof()) {
        db_char* param_name = stream.read_string();

        if (strcmp(param_name, "") == 0)
            break;

        if (strcmp(param_name, "user") == 0)
            m_user = stream.read_string();
        else if (strcmp(param_name, "database") == 0)
            m_database = stream.read_string();
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
    IF_RETURN_FAILED(m_protocol_version != PROTOCOL_VERSION);
    
    auth_md5_opacket_t packet;
    session->send_packet(packet);
    
    return RT_SUCCEEDED;
}


result_t password_ipacket_t::decode(packet_istream_t& stream)
{
    m_password = stream.read_string();
    return RT_SUCCEEDED;
}
 

result_t password_ipacket_t::process(server_session_t* session)
{
    // TODO(scott.zgeng): ��Ҫ���Ӷ������У�飬Ŀǰ�ȼ����������ֱ�ӷ��سɹ�
    
    auth_opacket_t<AuthenticationOk> packet;    
    return session->send_packet_with_end(packet);
}


command_action_t* command_action_t::create_command(const char* sql)
{
    if (strncmp(sql, "copy", 4) == 0)
        return new copy_in_action_t();
    else 
        return new simple_query_action_t();
}


copy_in_action_t::copy_in_action_t()
{

}

copy_in_action_t::~copy_in_action_t()
{

}


result_t copy_in_action_t::execute(server_session_t* session, const char* sql)
{
    char temp[1024];
    strcpy(temp, sql);
    char delim[] = " ";
    char* saveptr = NULL;    
    char* token = strtok_r(temp, delim, &saveptr);
    assert(token != NULL);
    token = strtok_r(NULL, delim, &saveptr);
    IF_RETURN_FAILED(token == NULL);

    column_table_t* table = database_t::instance.find_table(token);
    if (table == NULL) {
        error_opacket_t error_packet("can not find the table");
        session->send_packet_with_end(error_packet);
        return RT_SUCCEEDED;
    }

    copy_in_opacket_t copy_in_response;
    copy_in_response.init_columns(table);
    return session->send_packet(copy_in_response, this);
}


result_t copy_in_action_t::on_send_complete(server_session_t* session)
{
    session->recv_packet(this);
    return RT_SUCCEEDED;
}


result_t copy_in_action_t::on_recv_complete(server_session_t* session, packet_istream_t& stream)
{
    copy_data_ipacket_t copy_data;
    copy_data.decode(stream);
    
    return RT_SUCCEEDED;
}


simple_query_action_t::simple_query_action_t()
{
    m_stmt = NULL;
    m_segment_row_count = 0;
    m_total_count = 0;
}

simple_query_action_t::~simple_query_action_t()
{
    // TODO(scott.zgeng): 
}


  
result_t simple_query_action_t::execute(server_session_t* session, const char* sql)
{
    const char* tail;
    db_int32 len = strlen(sql);

    db_int32 sqlite_ret = sqlite3_vector_prepare(database_t::instance.native_handle(), sql, len, &m_stmt, &tail);
    if (sqlite_ret != SQLITE_OK) {
        error_opacket_t error_packet("sqlite3_vector_prepare failed");
        session->send_packet_with_end(error_packet);
        return RT_SUCCEEDED;
    }

    sqlite_ret = sqlite3_vector_step(m_stmt);
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
    m_segment_row_count = sqlite3_vector_row_count(m_stmt);
    m_total_count += m_segment_row_count;
    m_row_idx = 0;

    row_desc_opacket_t row_head_packet;
    project_node_t* node = get_statement_root_node(m_stmt);
    for (db_uint32 i = 0; i < node->column_count(); i++) {
        row_head_packet.add_row_desc(node->column_name(i), node->column_type(i), node->column_size(i));
    }

    return session->send_packet(row_head_packet, this);    
}


result_t simple_query_action_t::on_send_complete(server_session_t* session)
{
    if (m_row_idx == m_segment_row_count) {
        db_int32 sqlite_ret = sqlite3_vector_step(m_stmt);
        if (sqlite_ret == SQLITE_ERROR) {
            error_opacket_t packet("sqlite3_vector_step failed");
            session->send_packet_with_end(packet);
            return RT_SUCCEEDED;
        }

        if (sqlite_ret == SQLITE_DONE) {
            complete_opacket_t packet(m_total_count);
            session->send_packet_with_end(packet);
            return RT_SUCCEEDED;
        }

        // case SQLITE_ROW
        m_segment_row_count = sqlite3_vector_row_count(m_stmt);
        m_total_count += m_segment_row_count;
        m_row_idx = 0;
        assert(m_segment_row_count > 0);
    }


    data_row_opacket_t data_packet;
    db_int32 col_num = sqlite3_vector_column_count(m_stmt);
    for (db_int32 i = 0; i < col_num; i++) {
        const variant_t& val = sqlite3_vector_column_variant(m_stmt, i, m_row_idx);
        data_packet.add_row_data(val);
    }
    m_row_idx++;
    session->send_packet(data_packet, this);
    return RT_SUCCEEDED;
}




query_ipacket_t::query_ipacket_t()
{
    m_sql = NULL;

}


result_t query_ipacket_t::decode(packet_istream_t& stream)
{
    m_sql = stream.read_string();
    return RT_SUCCEEDED;
}



result_t query_ipacket_t::process(server_session_t* session)
{
    command_action_t* command = command_action_t::create_command(m_sql);
    IF_RETURN_FAILED(command == NULL);

    return command->execute(session, m_sql);
}

