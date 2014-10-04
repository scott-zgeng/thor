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
    case 'd':
        return new copy_data_ipacket_t();
    case 'c':
        return new copy_done_ipacket_t();
    
    default:
        return NULL;
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
            db_char* option_val = stream.read_string();
            DB_TRACE("%s = %s", param_name, option_val);
        }            
    }

    return RT_SUCCEEDED;
}


result_t startup_ipacket_t::process(server_session_t* session)
{
    // 80877103 means SSL startup
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
    // TODO(scott.zgeng): 需要增加对密码的校验，目前先简化这个操作，直接返回成功
    

    auth_opacket_t<AuthenticationOk> packet1;    
    param_status_opacket_t packet2("client_encoding", "LATIN1");
    param_status_opacket_t packet3("DateStyle", "ISO");
    read_for_query_opacket_t<true> end_packet;

    packet_vector_t packets;
    packets.push_back(&packet1);
    packets.push_back(&packet2);
    packets.push_back(&packet3);
    packets.push_back(&end_packet);
    
    return session->send_packets(packets);    
}



command_action_t* command_action_t::create_command(const char* sql)
{
    if (strncasecmp(sql, "copy", 4) == 0)
        return new copy_in_action_t();
    else 
        return new simple_query_action_t();
}


copy_in_action_t::copy_in_action_t()
{
    m_table = NULL;
}


copy_in_action_t::~copy_in_action_t()
{

}


result_t copy_in_action_t::execute(server_session_t* session, const char* sql)
{
    char temp[1024];
    strcpy(temp, sql); // TODO(scott.zgeng): 
    char delim[] = " ";
    char* saveptr = NULL;    
    char* token = strtok_r(temp, delim, &saveptr);
    assert(token != NULL);
    token = strtok_r(NULL, delim, &saveptr);
    IF_RETURN_FAILED(token == NULL);

    m_table = database_t::instance.find_table(token);
    if (m_table == NULL) {
        error_opacket_t error_packet("can not find the table");
        session->send_packet_with_end(error_packet);
        return RT_SUCCEEDED;
    }

    copy_in_opacket_t copy_in_response(false, m_table->get_column_count());        
    return session->send_packet(copy_in_response, this);
}


result_t copy_in_action_t::on_send_complete(server_session_t* session)
{
    DB_TRACE("copy_in_action_t::on_send_complete");
    session->recv_packet(this);
    return RT_SUCCEEDED;
}


result_t copy_in_action_t::on_recv_complete(server_session_t* session, db_int8 type, packet_istream_t& stream)
{    
    assert(type == 'd' || type == 'c');
    
    if (type == 'c') {  // it is copy done
        complete_opacket_t comp_packet(1);
        return session->send_packet_with_end(comp_packet);    
    }

    // type == 'd', it is copy data packet
    copy_data_ipacket_t copy_data;
    copy_data.decode(stream);
    row_data_t row_data;
    copy_data.gen_row_data(row_data, m_table->get_column_count());

    result_t ret = m_table->insert_row(row_data);
    IF_RETURN_FAILED(ret != RT_SUCCEEDED);

    session->recv_packet(this);
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

copy_data_ipacket_t::copy_data_ipacket_t()
{
    m_row = NULL;
    m_save_pos = NULL;
}


result_t copy_data_ipacket_t::decode(packet_istream_t& stream)
{
    m_row = stream.read_string();
    return RT_SUCCEEDED;
}
    

result_t copy_data_ipacket_t::process(server_session_t* session)
{
    return RT_SUCCEEDED;
}

void copy_data_ipacket_t::gen_row_data(row_data_t& row_data, db_uint32 column_count)
{   
    assert(column_count > 0);

    db_char* val = strtok_r(m_row, ",\n", &m_save_pos);
    row_data.push_back(val);
    
    for (db_uint32 i = 1; i < column_count; i++) {
        val = strtok_r(NULL, ",\n", &m_save_pos);
        row_data.push_back(val);
    }

}
