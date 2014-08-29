// hash_group_node.cpp by scott.zgeng@gmail.com 2014.08.29


#include "hash_group_node.h"


hash_group_node_t::hash_group_node_t(database_t* db, node_base_t* children) : m_aggr_table(db)
{
    m_children = children;
    m_database = db;
}

hash_group_node_t::~hash_group_node_t()
{

}


result_t hash_group_node_t::init(Parse* parse, Select* select)
{
    expr_factory_t factory(m_database);

    //if (epxr->op == TK_AGG_COLUMN) 表示group by 后的列
    //if (epxr->op == TK_AGG_FUNCTION) 表示group by 的聚合函数列
    //iAgg  /* Which entry in pAggInfo->aCol[] or ->aFunc[] */
    //x->pList  /* op = IN, EXISTS, SELECT, CASE, FUNCTION, BETWEEN */    
    //pAggInfo   /* Used by TK_AGG_COLUMN and TK_AGG_FUNCTION */

    // NOTE(scott.zgeng): 目前计划有最大支持列数限制，如果有必要可以去掉
    IF_RETURN_FAILED(select->pEList->nExpr > MAX_AGGR_COLUMNS);

    result_t ret;
    expr_base_t* expr;

    for (db_int32 i = 0; i < select->pEList->nExpr; i++) {        
        Expr* org_expr = select->pEList->a[i].pExpr;

        //IF_RETURN_FAILED(org_expr->op != TK_AGG_COLUMN && org_expr->op != TK_AGG_FUNCTION);


        ret = factory.build(select->pEList->a[i].pExpr, &expr);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);

        bool is_succ = m_aggr_columns.push_back(expr);
        IF_RETURN_FAILED(!is_succ);
    }

    for (db_int32 i = 0; i < select->pGroupBy->nExpr; i++) {
        ret = factory.build(select->pGroupBy->a[i].pExpr, &expr);
        IF_RETURN_FAILED(ret != RT_SUCCEEDED);

        bool is_succ = m_group_columns.push_back(expr);
        IF_RETURN_FAILED(!is_succ);
    }





    return RT_FAILED;
}

void hash_group_node_t::uninit()
{

}

result_t hash_group_node_t::next(rowset_t* rows, mem_stack_t* mem)
{
    ////for ()

    //m_aggr_table.
    return RT_FAILED;
}


db_int32 hash_group_node_t::rowid_size()
{
    return 0;
}


