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
    
    node_base_t* root_node = (node_base_t*)root;
    root_node->next(NULL);

    return SQLITE_DONE;
}
