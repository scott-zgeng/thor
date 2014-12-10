// exec_node.cpp by scott.zgeng@gmail.com 2014.08.08

#include "exec_node.h"

#include "expression.h"


#include "project_node.h"
#include "scan_node.h"
#include "join_node.h"
#include "hash_group_node.h"
#include "sort_node.h"


//-----------------------------------------------------------------------------
// node_generator_t
//-----------------------------------------------------------------------------
node_generator_t::node_generator_t(statement_t* stmt, Parse* parse, Select* select)
{
    m_stmt = stmt;
    m_parse = parse;
    m_select = select;
}

node_generator_t::~node_generator_t()
{
}



db_double node_generator_t::calc_mst(db_int32 table_num, db_double* cost_matrix, db_int32* closest, db_int32 start_no)
{
    db_double low_cost[MAX_JOIN_TABLE] = { 0 };

    for (db_int32 i = 0; i < table_num; i++) {
        low_cost[i] = cost_matrix[start_no * MAX_JOIN_TABLE + i];
        closest[i] = start_no;
    }

    db_double total_cost = 0; // 用来记录最小生成树的总成本
    for (db_int32 i = 1; i < table_num; i++) {
        db_double min_cost = DB_DBL_MAX;
        db_int32 join_tab_no = start_no;

        for (db_int32 n = 0; n < table_num; n++) {
            if (low_cost[n] != 0 && low_cost[n] <= min_cost) {
                min_cost = low_cost[n];
                join_tab_no = n;
            }
        }

        // 将找到的最近节点加入最小生成树
        total_cost += low_cost[join_tab_no];
        low_cost[join_tab_no] = 0;

        // 修正其他节点到最小生成树的距离
        for (db_int32 n = 0; n < table_num; n++) {
            if (low_cost[n] > cost_matrix[n * MAX_JOIN_TABLE + join_tab_no]) {
                low_cost[n] = cost_matrix[n * MAX_JOIN_TABLE + join_tab_no];
                closest[n] = join_tab_no;
            }
        }
    }

    return total_cost;
}


result_t node_generator_t::gen_join_plan(db_int32 table_num, db_double* cost_matrix, db_int32* closest, db_int32* start_no)
{
    db_int32 start = 0;
    db_int32 tmp_closest[MAX_JOIN_TABLE];    
    db_double min_cost = DB_DBL_MAX;
    db_double cost;    

    for (db_int32 i = 0; i < table_num; i++) {
        cost = calc_mst(table_num, cost_matrix, tmp_closest, i);

        if (min_cost > cost) {
            min_cost = cost;
            memcpy(closest, tmp_closest, sizeof(db_int32)* table_num);
            start = i;
        }
    }

    IF_RETURN_FAILED(table_num > 1 && min_cost == 0);
    DB_TRACE("JOIN start = %d, min cost = %f", start, min_cost);

    *start_no = start;
    return RT_SUCCEEDED;
}

db_double node_generator_t::calc_join_cost(scan_node_t* node1, scan_node_t* node2)
{    
    const char* table1 = node1->table_name();
    const char* table2 = node2->table_name();

    if (strcmp(table1, table2) == 0)
        return 0;




    return 0;
}

result_t node_generator_t::build_scan_join(node_base_t** root_node)
{
    // SQLITE DEFINE
    //#define JT_INNER     0x0001    /* Any kind of inner or cross join */
    //#define JT_CROSS     0x0002    /* Explicit use of the CROSS keyword */
    //#define JT_NATURAL   0x0004    /* True for a "natural" join */
    //#define JT_LEFT      0x0008    /* Left outer join */
    //#define JT_RIGHT     0x0010    /* Right outer join */
    //#define JT_OUTER     0x0020    /* The "OUTER" keyword is present */
    //#define JT_ERROR     0x0040    /* unknown or unsupported join type */

    result_t ret;
    db_int32 table_num = m_select->pSrc->nSrc;

    // TODO(scott.zgeng): 目前不支持多表过滤有OR条件
    IF_RETURN_FAILED(table_num > 1 && expr_factory_t::check_or(m_select->pWhere));

    scan_node_t* scan_nodes[MAX_JOIN_TABLE];
    for (db_int32 i = 0; i < table_num; i++) {

        // TODO(scott.zgeng@gmail.com): 需要增加异常情况退出的内存泄露
        scan_nodes[i] = new scan_node_t(m_stmt, i);
        IF_RETURN_FAILED(scan_nodes[i] == NULL);

        ret = scan_nodes[i]->init(m_parse, m_select);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);
    }
    
    if (table_num == 1) {
        *root_node = scan_nodes[0];
        return RT_SUCCEEDED;
    }

    // 计算每个表的连接成本
    db_double cost_matrix[MAX_JOIN_TABLE * MAX_JOIN_TABLE];
    for (db_int32 i = 0; i < table_num; i++) {
        for (db_int32 n = 0; n < table_num; n++) {        
            cost_matrix[i * MAX_JOIN_TABLE + n] = calc_join_cost(scan_nodes[i], scan_nodes[n]);
        }
    }

    // 获取最小的连接树顺序
    db_int32 start = 0;
    db_int32 closest[MAX_JOIN_TABLE];
    ret = gen_join_plan(table_num, cost_matrix, closest, &start);
    IF_RETURN_FAILED(ret != RT_SUCCEEDED);

    node_base_t* curr_node = scan_nodes[start];
    scan_nodes[start] = NULL;

    for (db_int32 i = 1; i < table_num; i++) {
        for (db_int32 n = 0; n < table_num; i++) {
            db_int32 parent_idx = closest[n];

            // 如果本节点已经加入，或者父节点还没加入，先跳过
            if (scan_nodes[n] == NULL || scan_nodes[parent_idx] == NULL)
                continue;

            node_base_t* left_node = curr_node;
            node_base_t* right_node = scan_nodes[n];
            scan_nodes[n] = NULL;

            curr_node = new join_node_t(left_node, right_node);
            IF_RETURN_FAILED(curr_node == NULL);

            ret = curr_node->init(m_parse, m_select);
            IF_RETURN_FAILED(ret != RT_SUCCEEDED);
        }
    }

    *root_node = curr_node;
    return RT_SUCCEEDED;
}

result_t node_generator_t::create_tree(node_base_t** root)
{
    result_t ret;
    node_base_t* curr_node = NULL;
    ret = build(&curr_node);
    IF_RETURN_FAILED(ret != RT_SUCCEEDED);

    ret = transform(curr_node);
    IF_RETURN_FAILED(ret != RT_SUCCEEDED);

    *root = curr_node;    
    return RT_SUCCEEDED;
}


result_t node_generator_t::transform(node_base_t* root)
{
    //db_uint32 parallel_count = 4; // 目前先简单设置一下
    //result_t ret = root->transform(NULL);

    return RT_SUCCEEDED;
}







result_t node_generator_t::build(node_base_t** root)
{
    IF_RETURN_FAILED((db_uint32)m_select->pSrc->nSrc != m_select->pSrc->nAlloc);
    IF_RETURN_FAILED(m_select->pSrc->nSrc > MAX_JOIN_TABLE);

    result_t ret;
    node_base_t* curr_node = NULL;
    ret = build_scan_join(&curr_node);
    IF_RETURN_FAILED(ret != RT_SUCCEEDED);

    if (m_select->pGroupBy != NULL && m_select->pGroupBy->nExpr > 0) {
        hash_group_node_t* group_node = new hash_group_node_t(m_stmt, curr_node);
        IF_RETURN_FAILED(group_node == NULL);
        
        ret = group_node->init(m_parse, m_select);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);

        curr_node = group_node;
    }

    if (m_select->pOrderBy != NULL && m_select->pOrderBy->nExpr > 0) {
        sort_node_t* sort_node = new sort_node_t(curr_node);
        IF_RETURN_FAILED(sort_node == NULL);

        ret = sort_node->init(m_parse, m_select);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);

        curr_node = sort_node;
    }

    node_base_t* project_node = new project_node_t(m_stmt, curr_node);
    IF_RETURN_FAILED(project_node == NULL);

    ret = project_node->init(m_parse, m_select);
    IF_RETURN_FAILED(ret != RT_SUCCEEDED);


    *root = project_node;
    return RT_SUCCEEDED;
}



