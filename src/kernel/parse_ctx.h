// parse_ctx.h by scott.zgeng@gmail.com 2014.07.11

 
#ifndef  __PARSE_CTX_H__
#define  __PARSE_CTX_H__

extern "C" {
#include "sqliteInt.h"
#include "vdbeInt.h"
}



#include "define.h"

class project_node_t;
project_node_t* get_statement_root_node(sqlite3_stmt* stmt);


#include "packet.h"
variant_t sqlite3_vector_column_variant(sqlite3_stmt* stmt, int col_idx, int row_idx);

#endif //__PARSE_CTX_H__

