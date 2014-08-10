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

    return RT_SUCCEEDED;
}

void scan_node_t::uninit()
{

}

db_int32 scan_node_t::size()
{
    return sizeof(rowid_t);
}


// 在调用next之前，应用需要先调用该节点size()，计算用来缓存行号的空间，并且双方已经协商好数据格式
result_t scan_node_t::next(row_set_t* rows)
{
    assert(!m_where->has_null());
    result_t ret;
    mem_handle_t result;

    expr_context_t* ectx = new expr_context_t(rows);

    ret = m_where->calc(ectx, result);
    IF_RETURN_FAILED(ret != RT_SUCCEEDED);

    row_set_t* in_rows = ectx->row_set();
    db_int8* expr_result = (db_int8*)result.ptr();    
    rowid_t* in_ptr = (rowid_t*)in_rows->ptr();
    rowid_t* out_ptr = (rowid_t*)rows->ptr();

    // 获取所有有效的结果集合，去掉不符合的行
    db_int32 count = 0;
    for (int i = 0; i < ectx->row_count(); i++) {
        if (expr_result[i]) {
            out_ptr[count] = in_ptr[i];
            count++;
        }
    }

    rows->row_count = count;

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

db_int32 join_node_t::size()
{
    return m_left->size() + m_right->size();
}


result_t join_node_t::next(row_set_t* rows)
{
    return RT_FAILED; 
}


//-----------------------------------------------------------------------------
// project_node_t
//-----------------------------------------------------------------------------
project_node_t::project_node_t(node_base_t* children)
{
    m_children = children;
    m_expr_num = 0;
}

project_node_t::~project_node_t()
{

}

result_t project_node_t::init(Parse* parse, Select* select)
{
    // TODO(scott.zgeng@gmail.com): 后续换成vector，这里先简单用
    ExprList* expr_list = select->pEList;
    assert(expr_list->nExpr <= MAX_SELECT_RESULT_NUM);

    result_t ret;
    for (db_int32 i = 0; i < expr_list->nExpr; i++) {
        expr_base_t* expr = NULL;
        ret = expr_base_t::build(expr_list->a[i].pExpr, &expr);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);

        m_result_expr[i] = expr;
    }

    m_expr_num = expr_list->nExpr;

    db_int32 alloc_size = m_children->size() * SEGMENT_SIZE;
    m_row_buff = malloc(alloc_size);
    IF_RETURN_FAILED(m_row_buff == NULL);
    
    return RT_SUCCEEDED;
}

void project_node_t::uninit()
{

}

db_int32 project_node_t::size()
{
    return 0;
}


result_t project_node_t::next(row_set_t* rows)
{
    return RT_SUCCEEDED;
}


result_t project_node_t::project()
{
    result_t ret;

    row_set_t sub_rows(m_row_buff);
    ret = m_children->next(&sub_rows);
    IF_RETURN_FAILED(ret != RT_SUCCEEDED);

    expr_context_t ectx(&sub_rows);
    
    for (db_int32 i = 0; i < m_expr_num; i++) {
        ret = m_result_expr[i]->calc(&ectx, m_expr_mem[i]);
    }

    return RT_SUCCEEDED;


}


