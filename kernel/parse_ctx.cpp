// parse_ctx.cpp by scott.zgeng@gmail.com 2014.07.11

#include <assert.h>



#include "parse_ctx.h"


int sqlite3SelectCreatePlan(Parse* pParse, Select* pSelete)
{
    if (!pParse->columnStorage) return SQLITE_OK;

    node_generator_t generator(pParse, pSelete);

    node_base_t* root = NULL;
    if (generator.build(&root) != RT_SUCCEEDED)
        return SQLITE_ERROR;

    pParse->pVdbe->pRootNode = root;

    return SQLITE_OK;
}



query_pack_t::query_pack_t()
{

}

query_pack_t::~query_pack_t()
{

}


result_t query_pack_t::generate_data(int32 table_id, int32 column_id)
{
    return RT_SUCCEEDED;
}



node_generator_t::node_generator_t(Parse* parse, Select* select)
{
    m_parse = parse;
    m_select = select;
}

node_generator_t::~node_generator_t()
{

}



result_t node_generator_t::build_join(node_base_t** scan_nodes, int32 tab_num, node_base_t** root)
{
    if (tab_num == 1) return RT_SUCCEEDED;

    result_t ret;
    node_base_t* node = scan_nodes[0];
    for (int32 i = 1; i < tab_num; i++) {
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
    int32 tab_num = m_select->pSrc->nSrc;

    node_base_t* scan_nodes[MAX_JOIN_TABLE];    
    for (int32 i = 0; i < tab_num; i++) {
        
        // TODO(scott.zgeng@gmail.com): 需要增加异常情况退出的内存泄露
        scan_nodes[i] = new scan_node_t(i);
        IF_RETURN_FAILED(scan_nodes[i] == NULL);
        
        ret = scan_nodes[i]->init(m_parse, m_select);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);
    }

    node_base_t* root = scan_nodes[0];    
    ret = build_join(scan_nodes, tab_num, &root);
    IF_RETURN_FAILED(ret != RT_SUCCEEDED);


    return RT_SUCCEEDED;
}

result_t node_generator_t::build_expression(Expr* expr, expr_base_t** root)
{
    assert(expr);
    result_t ret;

    expr_base_t* expr_node = create_expression(expr);
    IF_RETURN_FAILED(expr_node == NULL);
    
    ret = expr_node->init(expr);
    IF_RETURN_FAILED(expr_node == NULL);
    
    if (expr->pLeft != NULL) {
        ret = build_expression(expr->pLeft, &expr_node->m_left);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);
    }

    if (expr->pRight != NULL) {
        ret = build_expression(expr->pRight, &expr_node->m_right);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);
    }

    *root = expr_node;
    return RT_SUCCEEDED;
}

expr_base_t* node_generator_t::create_expression(Expr* expr)
{
    switch (expr->op)
    {
    case TK_COLUMN:
        return new expr_column_t();
    case TK_INTEGER:
        return new expr_integer_t();
    //case TK_FLOAT:
        //return new expr_float_t();
    
    case TK_GT:
        return new expr_logic_op_t<TK_GT>();
    case TK_LT:
        return new expr_logic_op_t<TK_LT>();
    case TK_NE:
        return new expr_logic_op_t<TK_NE>();

    default:        
        return NULL;
    }
}


scan_node_t::scan_node_t(int index)
{
    m_index = index;
    m_condition = NULL;
}

scan_node_t::~scan_node_t()
{

}

result_t scan_node_t::init(Parse* parse, Select* select)
{
    node_generator_t  generator(parse, select);
    result_t ret;
    
    ret = generator.build_expression(select->pWhere, &m_condition);
    IF_RETURN_FAILED(ret != RT_SUCCEEDED);

    return RT_SUCCEEDED;
}

void scan_node_t::uninit()
{

}

const char* scan_node_t::name()
{
    return "SCAN_NODE";
}

result_t scan_node_t::next(query_pack_t* pack)
{
    return RT_FAILED;
}



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

result_t join_node_t::next(query_pack_t* pack)
{
    return RT_FAILED;
}

const char* join_node_t::name()
{
    return "JOIN_NODE";
}


//--------------------------------------------------------------
// expr_column_t
//--------------------------------------------------------------
expr_plus_t::expr_plus_t()
{
}
expr_plus_t::~expr_plus_t()
{
}

result_t expr_plus_t::init(Expr* expr)
{
    return RT_FAILED;
}
    
//template<typename T1, typename T2>
result_t expr_plus_t::calc(query_pack_t* pack)
{
    assert(m_left && m_right);

    //result_t ret;

    //ret = m_left->calc(pack);
    //IF_RETURN_FAILED(ret != RT_SUCCEEDED);

    //pack->move_out_2_left();

    //m_right->calc(pack);
    //IF_RETURN_FAILED(ret != RT_SUCCEEDED);

    //// 获取刚刚计算得到的BUFFER
    //int32* left_data = (int32*)pack->get_left_data();
    //int32* right_data = (int32*)pack->get_right_data();
    //int32* out_data = (int32*)pack->get_out_data();
    //int32 count = pack->get_row_count();

    //for (int i = 0; i < count; i++) {
    //    out_data[i] = left_data[i] + right_data[i];
    //}

    //return ret;
    return RT_FAILED;
}

//--------------------------------------------------------------
// expr_column_t
//--------------------------------------------------------------

expr_integer_t::expr_integer_t()
{

}
    
expr_integer_t::~expr_integer_t()
{
}

result_t expr_integer_t::init(Expr* expr)
{
    //expr->
    return RT_SUCCEEDED;
}
    
result_t expr_integer_t::calc(query_pack_t* pack)
{
    return RT_SUCCEEDED;
}



//--------------------------------------------------------------
// expr_column_t
//--------------------------------------------------------------
expr_column_t::expr_column_t()
{

}


expr_column_t::~expr_column_t()
{

}

result_t expr_column_t::init(Expr* expr)
{
    m_table_id = expr->iTable;
    m_column_id = expr->iColumn;
    return RT_SUCCEEDED;
}



result_t expr_column_t::calc(query_pack_t* pack)
{
    assert(m_left == NULL && m_right == NULL);

    // 获取已经存储在PACK中的行
    result_t ret;
    ret = pack->generate_data(m_table_id, m_column_id);
    IF_RETURN_FAILED(ret != RT_SUCCEEDED);

    return RT_SUCCEEDED;
}

