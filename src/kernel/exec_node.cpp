// exec_node.cpp by scott.zgeng@gmail.com 2014.08.08

#include "exec_node.h"
#include "expression.h"



//-----------------------------------------------------------------------------
// node_generator_t
//-----------------------------------------------------------------------------
node_generator_t::node_generator_t(Parse* parse, Select* select)
{
    m_parse = parse;
    m_select = select;
}

node_generator_t::~node_generator_t()
{
}


result_t node_generator_t::build_join(node_base_t** scan_nodes, db_int32 tab_num, node_base_t** root)
{
    if (tab_num == 1) return RT_SUCCEEDED;

    result_t ret;
    node_base_t* node = scan_nodes[0];
    for (db_int32 i = 1; i < tab_num; i++) {
        node_base_t* new_node = new join_node_t(node, scan_nodes[i]);
        IF_RETURN_FAILED(new_node == NULL);

        ret = new_node->init(m_parse, m_select);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);

        node = new_node;
    }

    return RT_SUCCEEDED;
}

result_t node_generator_t::build(node_base_t** root_node)
{
    IF_RETURN_FAILED(m_select->pSrc->nSrc != m_select->pSrc->nAlloc);
    IF_RETURN_FAILED(m_select->pSrc->nSrc > MAX_JOIN_TABLE);

    result_t ret;
    db_int32 tab_num = m_select->pSrc->nSrc;

    node_base_t* scan_nodes[MAX_JOIN_TABLE];
    for (db_int32 i = 0; i < tab_num; i++) {

        // TODO(scott.zgeng@gmail.com): 需要增加异常情况退出的内存泄露
        scan_nodes[i] = new scan_node_t(i);
        IF_RETURN_FAILED(scan_nodes[i] == NULL);

        ret = scan_nodes[i]->init(m_parse, m_select);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);
    }

    node_base_t* root = scan_nodes[0];
    ret = build_join(scan_nodes, tab_num, &root);
    IF_RETURN_FAILED(ret != RT_SUCCEEDED);

    node_base_t* project_node = new project_node_t(root);
    IF_RETURN_FAILED(project_node == NULL);

    ret = project_node->init(m_parse, m_select);
    IF_RETURN_FAILED(ret != RT_SUCCEEDED);

    *root_node = project_node;
    return RT_SUCCEEDED;
}




//-----------------------------------------------------------------------------
// project_node_t
//-----------------------------------------------------------------------------
project_node_t::project_node_t(node_base_t* children)
{
    m_children = children;
}

project_node_t::~project_node_t()
{

}

result_t project_node_t::init(Parse* parse, Select* select)
{
    ExprList* expr_list = select->pEList;

    result_t ret;
    for (db_int32 i = 0; i < expr_list->nExpr; i++) {
        expr_base_t* expr = NULL;
        ret = expr_base_t::build(expr_list->a[i].pExpr, &expr);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);

        m_expr_columns.push_back(expr);
    }

    ret = m_sub_rows.init(m_children->rowid_size());
    IF_RETURN_FAILED(ret != RT_SUCCEEDED);
    
    m_expr_values.resize(expr_list->nExpr);

    m_curr_idx = -1;

    return RT_SUCCEEDED;
}

void project_node_t::uninit()
{
}

db_int32 project_node_t::rowid_size()
{
    return 0;
}


result_t project_node_t::next(row_set_t* rows, mem_stack_t* mem)
{
    return RT_FAILED;
}


result_t project_node_t::next()
{
    result_t ret;

    m_mem.reset();

    ret = m_children->next(&m_sub_rows, &m_mem);
    IF_RETURN_FAILED(ret != RT_SUCCEEDED);
    
    for (db_uint32 i = 0; i < m_expr_columns.size(); i++) {

        mem_handle_t mem_handle;
        ret = m_expr_columns[i]->calc(&m_sub_rows, &m_mem, mem_handle);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);

        m_expr_values[i] = mem_handle.transfer();        
    }

    return RT_SUCCEEDED;
}



//-----------------------------------------------------------------------------
// scan_node_t
//-----------------------------------------------------------------------------
scan_node_t::scan_node_t(int index)
{
    m_index = index;
    m_where = NULL;
    
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
result_t scan_node_t::next(row_set_t* rows, mem_stack_t* mem)
{
    assert(!m_where->has_null());
    result_t ret;
    mem_handle_t result;

    ret = m_cursor.next_segment(rows);
    IF_RETURN_FAILED(ret != RT_SUCCEEDED);

    if (m_where == NULL) 
        return RT_SUCCEEDED;        
    

    ret = m_where->calc(rows, mem, result);
    IF_RETURN_FAILED(ret != RT_SUCCEEDED);

    db_bool* expr_result = (db_bool*)result.ptr();
    
    rowid_t* in_rows = rows->data();
    rowid_t* out_rows = rows->data();

    // 获取所有有效的结果集合，去掉不符合的行
    db_int32 count = 0;
    for (int i = 0; i < rows->count(); i++) {
        if (expr_result[i]) {
            out_rows[count] = in_rows[i];
            count++;
        }
    }

    rows->set_mode(row_set_t::RANDOM_MODE);
    rows->set_count(count);
    return RT_SUCCEEDED;
}


//-----------------------------------------------------------------------------
// join_node_t
//-----------------------------------------------------------------------------
join_node_t::join_node_t(node_base_t* left, node_base_t* right)
{
    m_left = left;
    m_right = right;
}

join_node_t::~join_node_t()
{

}


result_t join_node_t::init(Parse* parse, Select* select)
{
    return RT_FAILED;
}

void join_node_t::uninit()
{

}

db_int32 join_node_t::rowid_size()
{
    return m_left->rowid_size() + m_right->rowid_size();
}


result_t join_node_t::next(row_set_t* rows, mem_stack_t* mem)
{
    return RT_FAILED;
}
