// scan_node.cpp by scott.zgeng@gmail.com 2014.08.27



#include "scan_node.h"


//-----------------------------------------------------------------------------
// scan_node_t
//-----------------------------------------------------------------------------
scan_node_t::scan_node_t(database_t* db, int index)
{
    m_database = db;
    m_index = index;
    m_where = NULL;
    m_eof = false;

    m_odd_rows = NULL;
    m_odd_count = 0;
}


scan_node_t::~scan_node_t()
{

}


result_t scan_node_t::init(Parse* parse, Select* select)
{
    result_t ret;

    expr_factory_t factory(m_database);

    ret = factory.build(select->pWhere, &m_where);
    IF_RETURN_FAILED(ret != RT_SUCCEEDED);

    SrcList::SrcList_item* src = &select->pSrc->a[m_index];

    column_table_t* table = database_t::instance.find_table(src->zName);
    IF_RETURN_FAILED(table == NULL);

    ret = table->init_cursor(&m_cursor);
    IF_RETURN_FAILED(ret != RT_SUCCEEDED);

    return RT_SUCCEEDED;
}


void scan_node_t::uninit()
{

}



// 在调用next之前，应用需要先调用该节点size()，计算用来缓存行号的空间，并且双方已经协商好数据格式
result_t scan_node_t::next(rowset_t* rs, mem_stack_t* mem)
{
    assert(rs->mode == SINGLE_TABLE_MODE);
    single_rowset_t* srs = (single_rowset_t*)rs;

    if (m_where == NULL) {
        // TODO(scott.zgeng): 后面优化成不同的执行节点，比如子查询，视图等        
        return m_cursor.next_segment(srs);
    }

    // 从上次的缓存中拷贝上次剩余结果
    rowid_t* result_rows = srs->rows;
    db_int32 result_count = m_odd_count;
    if (result_count != 0) {        
        memcpy(result_rows, m_odd_rows, result_count * sizeof(rowid_t));
        result_rows += result_count;
    }

    result_t ret;
    while (!m_eof) {
        ret = m_cursor.next_segment(&m_rowset);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);

        if UNLIKELY(m_rowset.count == 0) break;

        // 如果获取的结果少于最大容量，则说明已经到了最后一段        
        m_eof = m_rowset.count < SEGMENT_SIZE;

        mem_handle_t result;
        ret = m_where->calc(&m_rowset, mem, result);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);

        db_bool* expr_result = (db_bool*)result.ptr();

        rowid_t* in_rows = m_rowset.rows;
        rowid_t* out_rows = m_rowset.rows;

        db_int32 count = 0;
        // 获取所有有效的结果集合，去掉不符合的行        
        for (db_uint32 i = 0; i < m_rowset.count; i++) {
            if (expr_result[i]) {
                out_rows[count] = in_rows[i];
                count++;
            }
        }

        if (result_count + count >= SEGMENT_SIZE) {
            db_int32 copy_count = SEGMENT_SIZE - result_count;
            memcpy(result_rows, out_rows, copy_count * sizeof(rowid_t));

            m_odd_rows = m_rowset.rows + copy_count;
            m_odd_count = count - copy_count;
            
            srs->count = SEGMENT_SIZE;            
            srs->segment_id = single_rowset_t::INVALID_SEGMENT;
            return RT_SUCCEEDED;

        } else {
            memcpy(result_rows, out_rows, count * sizeof(rowid_t));
            result_count += count;
            result_rows += count;
        }
    }

    rs->count = result_count;
    srs->segment_id = single_rowset_t::INVALID_SEGMENT;
    return RT_SUCCEEDED;
}

