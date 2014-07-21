// parse_ctx.h by scott.zgeng@gmail.com 2014.07.11

#ifndef  __PARSE_CTX__
#define  __PARSE_CTX__

#include "../sql/sqliteInt.h"
void thorSyntaxError(Parse* pParse, Token* pToken);
void thorStackOverflow(Parse* pParse);


#endif //__PARSE_CTX__

