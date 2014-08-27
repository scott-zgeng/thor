// scan_node.cpp by scott.zgeng@gmail.com 2014.08.27



#include "scan_node.h"


//-----------------------------------------------------------------------------
// scan_node_t
//-----------------------------------------------------------------------------
scan_node_t::scan_node_t(int index)
{
    m_index = index;
    m_where = NULL;
    m_eof = false;
}


scan_node_t::~scan_node_t()
{

}


result_t scan_node_t::init(Parse* parse, Select* select)
{
    result_t ret;

    ret = expr_base_t::build(select->pWhere, &m_where);
    IF_RETURN_FAILED(ret != RT_SUCCEEDED);

    SrcList::SrcList_item* src = &select->pSrc->a[m_index];

    column_table_t* table = database_t::instance.find_table(src->zName);
    IF_RETURN_FAILED(table == NULL);

    ret = table->init_cursor(&m_cursor);
    IF_RETURN_FAILED(ret != RT_SUCCEEDED);

    m_rowset.init(m_rows);

    return RT_SUCCEEDED;
}


void scan_node_t::uninit()
{

}


db_int32 scan_node_t::rowid_size()
{
    return sizeof(rowid_t);
}



// 在调用next之前，应用需要先调用该节点size()，计算用来缓存行号的空间，并且双方已经协商好数据格式
result_t scan_node_t::next(rowset_t* set, mem_stack_t* mem)
{
    if UNLIKELY(m_where == NULL) {
        // TODO(scott.zgeng): 后面优化成不同的执行节点
        return m_cursor.next_segment(set);
    }

    // 从上次的缓存中拷贝上次剩余结果
    rowid_t* result_rows = set->data();
    db_int32 result_count = m_rowset.count();
    if (result_count != 0) {
        memcpy(result_rows, m_rowset.data(), result_count * sizeof(rowid_t));
        result_rows += result_count;
    }

    result_t ret;
    m_rowset.init(m_rows);
    while (!m_eof) {
        ret = m_cursor.next_segment(&m_rowset);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);

        if UNLIKELY(m_rowset.count() == 0) break;

        // 如果获取的结果少于最大容量，则说明已经到了最后一段        
        m_eof = m_rowset.count() < SEGMENT_SIZE;

        mem_handle_t result;
        ret = m_where->calc(&m_rowset, mem, result);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);

        db_bool* expr_result = (db_bool*)result.ptr();

        rowid_t* in_rows = m_rowset.data();
        rowid_t* out_rows = m_rowset.data();

        db_int32 count = 0;
        // 获取所有有效的结果集合，去掉不符合的行        
        for (int i = 0; i < m_rowset.count(); i++) {
            if (expr_result[i]) {
                out_rows[count] = in_rows[i];
                count++;
            }
        }

        if (result_count + count >= SEGMENT_SIZE) {
            db_int32 copy_count = SEGMENT_SIZE - result_count;
            memcpy(result_rows, out_rows, copy_count * sizeof(rowid_t));

            m_rowset.init(m_rows + copy_count);
            m_rowset.set_count(count - copy_count);

            set->set_count(SEGMENT_SIZE);
            set->set_mode(rowset_t::RANDOM_MODE);
            return RT_SUCCEEDED;

        }
        else {
            memcpy(result_rows, out_rows, count * sizeof(rowid_t));
            result_count += count;
            result_rows += count;
        }
    }

    set->set_count(result_count);
    set->set_mode(rowset_t::RANDOM_MODE);
    return RT_SUCCEEDED;
}

