// parse_ctx.cpp by scott.zgeng@gmail.com 2014.07.11

#include <assert.h>



#include "parse_ctx.h"
#include "expression.h"
#include "exec_node.h"


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

int sqlite3VectorStep(void* root)
{    
    project_node_t* root_node = (project_node_t*)root;

    if (root_node->m_curr_idx >= SEGMENT_SIZE) {
        root_node->project();
        root_node->m_curr_idx = 0;
    } else {
        root_node->m_curr_idx++;
    }

    return SQLITE_DONE;
}



void sqlite3VectorFinalize(void* root)
{

}

int sqlite3VectorColumnInt(void* root, int index)
{
    project_node_t* root_node = (project_node_t*)root;

    db_int32* ptr = (db_int32*)root_node->m_expr_mem[index].ptr();
    return ptr[root_node->m_curr_idx];
}


const char* sqlite3VectorColumnString(void* root, int index)
{
    node_base_t* root_node = (node_base_t*)root;
    return NULL;
}

int sqlite3VectorColumnType(void* root, int index)
{
    project_node_t* root_node = (project_node_t*)root;    
    return root_node->m_result_expr[index]->type();
}


