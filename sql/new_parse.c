/* Driver template for the LEMON parser generator.
** The author disclaims copyright to this source code.
*/
/* First off, code is included that follows the "include" declaration
** in the input grammar file. */
#include <stdio.h>
#line 35 ".\\thor\\sql\\new_parse.y"

#include "sqliteInt.h"
#include "../kernel/parse_ctx.h"

/*
** Disable all error recovery processing in the parser push-down
** automaton.
*/
#define YYNOERRORRECOVERY 1

/*
** Make yytestcase() the same as testcase()
*/
#define yytestcase(X) testcase(X)

/*
** An instance of this structure holds information about the
** LIMIT clause of a SELECT statement.
*/
struct LimitVal {
  Expr *pLimit;    /* The LIMIT expression.  NULL if there is no limit */
  Expr *pOffset;   /* The OFFSET expression.  NULL if there is none */
};

/*
** An instance of this structure is used to store the LIKE,
** GLOB, NOT LIKE, and NOT GLOB operators.
*/
struct LikeOp {
  Token eOperator;  /* "like" or "glob" or "regexp" */
  int bNot;         /* True if the NOT keyword is present */
};

#line 442 ".\\thor\\sql\\new_parse.y"

  /* This is a utility routine used to set the ExprSpan.zStart and
  ** ExprSpan.zEnd values of pOut so that the span covers the complete
  ** range of text beginning with pStart and going to the end of pEnd.
  */
  static void spanSet(ExprSpan *pOut, Token *pStart, Token *pEnd){
    pOut->zStart = pStart->z;
    pOut->zEnd = &pEnd->z[pEnd->n];
  }

  /* Construct a new Expr object from a single identifier.  Use the
  ** new Expr to populate pOut.  Set the span of pOut to be the identifier
  ** that created the expression.
  */
  static void spanExpr(ExprSpan *pOut, Parse *pParse, int op, Token *pValue){
    pOut->pExpr = sqlite3PExpr(pParse, op, 0, 0, pValue);
    pOut->zStart = pValue->z;
    pOut->zEnd = &pValue->z[pValue->n];
  }
#line 528 ".\\thor\\sql\\new_parse.y"

  /* This routine constructs a binary expression node out of two ExprSpan
  ** objects and uses the result to populate a new ExprSpan object.
  */
  static void spanBinaryExpr(
    ExprSpan *pOut,     /* Write the result here */
    Parse *pParse,      /* The parsing context.  Errors accumulate here */
    int op,             /* The binary operation */
    ExprSpan *pLeft,    /* The left operand */
    ExprSpan *pRight    /* The right operand */
  ){
    pOut->pExpr = sqlite3PExpr(pParse, op, pLeft->pExpr, pRight->pExpr, 0);
    pOut->zStart = pLeft->zStart;
    pOut->zEnd = pRight->zEnd;
  }
#line 582 ".\\thor\\sql\\new_parse.y"

  /* Construct an expression node for a unary postfix operator
  */
  static void spanUnaryPostfix(
    ExprSpan *pOut,        /* Write the new expression node here */
    Parse *pParse,         /* Parsing context to record errors */
    int op,                /* The operator */
    ExprSpan *pOperand,    /* The operand */
    Token *pPostOp         /* The operand token for setting the span */
  ){
    pOut->pExpr = sqlite3PExpr(pParse, op, pOperand->pExpr, 0, 0);
    pOut->zStart = pOperand->zStart;
    pOut->zEnd = &pPostOp->z[pPostOp->n];
  }                           
#line 601 ".\\thor\\sql\\new_parse.y"

  /* A routine to convert a binary TK_IS or TK_ISNOT expression into a
  ** unary TK_ISNULL or TK_NOTNULL expression. */
  static void binaryToUnaryIfNull(Parse *pParse, Expr *pY, Expr *pA, int op){
    sqlite3 *db = pParse->db;
    if( db->mallocFailed==0 && pY->op==TK_NULL ){
      pA->op = (u8)op;
      sqlite3ExprDelete(db, pA->pRight);
      pA->pRight = 0;
    }
  }
#line 629 ".\\thor\\sql\\new_parse.y"

  /* Construct an expression node for a unary prefix operator
  */
  static void spanUnaryPrefix(
    ExprSpan *pOut,        /* Write the new expression node here */
    Parse *pParse,         /* Parsing context to record errors */
    int op,                /* The operator */
    ExprSpan *pOperand,    /* The operand */
    Token *pPreOp         /* The operand token for setting the span */
  ){
    pOut->pExpr = sqlite3PExpr(pParse, op, pOperand->pExpr, 0, 0);
    pOut->zStart = pPreOp->z;
    pOut->zEnd = pOperand->zEnd;
  }
#line 120 ".\\thor\\sql\\new_parse.c"
/* Next is all token values, in a form suitable for use by makeheaders.
** This section will be null unless lemon is run with the -m switch.
*/
/* 
** These constants (all generated automatically by the parser generator)
** specify the various kinds of tokens (terminals) that the parser
** understands. 
**
** Each symbol here is a terminal symbol in the grammar.
*/
/* Make sure the INTERFACE macro is defined.
*/
#ifndef INTERFACE
# define INTERFACE 1
#endif
/* The next thing included is series of defines which control
** various aspects of the generated parser.
**    YYCODETYPE         is the data type used for storing terminal
**                       and nonterminal numbers.  "unsigned char" is
**                       used if there are fewer than 250 terminals
**                       and nonterminals.  "int" is used otherwise.
**    YYNOCODE           is a number of type YYCODETYPE which corresponds
**                       to no legal terminal or nonterminal number.  This
**                       number is used to fill in empty slots of the hash 
**                       table.
**    YYFALLBACK         If defined, this indicates that one or more tokens
**                       have fall-back values which should be used if the
**                       original value of the token will not parse.
**    YYACTIONTYPE       is the data type used for storing terminal
**                       and nonterminal numbers.  "unsigned char" is
**                       used if there are fewer than 250 rules and
**                       states combined.  "int" is used otherwise.
**    thorParserTOKENTYPE     is the data type used for minor tokens given 
**                       directly to the parser from the tokenizer.
**    YYMINORTYPE        is the data type used for all minor tokens.
**                       This is typically a union of many types, one of
**                       which is thorParserTOKENTYPE.  The entry in the union
**                       for base tokens is called "yy0".
**    YYSTACKDEPTH       is the maximum depth of the parser's stack.  If
**                       zero the stack is dynamically sized using realloc()
**    thorParserARG_SDECL     A static variable declaration for the %extra_argument
**    thorParserARG_PDECL     A parameter declaration for the %extra_argument
**    thorParserARG_STORE     Code to store %extra_argument into yypParser
**    thorParserARG_FETCH     Code to extract %extra_argument from yypParser
**    YYNSTATE           the combined number of states.
**    YYNRULE            the number of rules in the grammar
**    YYERRORSYMBOL      is the code number of the error symbol.  If not
**                       defined, then do no error processing.
*/
#define YYCODETYPE unsigned char
#define YYNOCODE 164
#define YYACTIONTYPE unsigned short int
#define YYWILDCARD 59
#define thorParserTOKENTYPE Token
typedef union {
  int yyinit;
  thorParserTOKENTYPE yy0;
  IdList* yy6;
  Expr* yy66;
  ExprList* yy70;
  u16 yy89;
  With* yy197;
  struct LikeOp yy260;
  ExprSpan yy266;
  struct LimitVal yy267;
  SrcList* yy269;
  Select* yy273;
  int yy320;
} YYMINORTYPE;
#ifndef YYSTACKDEPTH
#define YYSTACKDEPTH 100
#endif
#define thorParserARG_SDECL Parse *pParse;
#define thorParserARG_PDECL ,Parse *pParse
#define thorParserARG_FETCH Parse *pParse = yypParser->pParse
#define thorParserARG_STORE yypParser->pParse = pParse
#define YYNSTATE 218
#define YYNRULE 126
#define YYFALLBACK 1
#define YY_NO_ACTION      (YYNSTATE+YYNRULE+2)
#define YY_ACCEPT_ACTION  (YYNSTATE+YYNRULE+1)
#define YY_ERROR_ACTION   (YYNSTATE+YYNRULE)

/* The yyzerominor constant is used to initialize instances of
** YYMINORTYPE objects to zero. */
static const YYMINORTYPE yyzerominor = { 0 };

/* Define the yytestcase() macro to be a no-op if is not already defined
** otherwise.
**
** Applications can choose to define yytestcase() in the %include section
** to a macro that can assist in verifying code coverage.  For production
** code the yytestcase() macro should be turned off.  But it is useful
** for testing.
*/
#ifndef yytestcase
# define yytestcase(X)
#endif


/* Next are the tables used to determine what action to take based on the
** current state and lookahead token.  These tables are used to implement
** functions that take a state number and lookahead value and return an
** action integer.  
**
** Suppose the action integer is N.  Then the action is determined as
** follows
**
**   0 <= N < YYNSTATE                  Shift N.  That is, push the lookahead
**                                      token onto the stack and goto state N.
**
**   YYNSTATE <= N < YYNSTATE+YYNRULE   Reduce by rule N-YYNSTATE.
**
**   N == YYNSTATE+YYNRULE              A syntax error has occurred.
**
**   N == YYNSTATE+YYNRULE+1            The parser accepts its input.
**
**   N == YYNSTATE+YYNRULE+2            No such action.  Denotes unused
**                                      slots in the yy_action[] table.
**
** The action table is constructed as a single large table named yy_action[].
** Given state S and lookahead X, the action is computed as
**
**      yy_action[ yy_shift_ofst[S] + X ]
**
** If the index value yy_shift_ofst[S]+X is out of range or if the value
** yy_lookahead[yy_shift_ofst[S]+X] is not equal to X or if yy_shift_ofst[S]
** is equal to YY_SHIFT_USE_DFLT, it means that the action is not in the table
** and that yy_default[S] should be used instead.  
**
** The formula above is for computing the action when the lookahead is
** a terminal symbol.  If the lookahead is a non-terminal (as occurs after
** a reduce action) then the yy_reduce_ofst[] array is used in place of
** the yy_shift_ofst[] array and YY_REDUCE_USE_DFLT is used in place of
** YY_SHIFT_USE_DFLT.
**
** The following are the tables generated in this section:
**
**  yy_action[]        A single table containing all actions.
**  yy_lookahead[]     A table containing the lookahead for each entry in
**                     yy_action.  Used to detect hash collisions.
**  yy_shift_ofst[]    For each state, the offset into yy_action for
**                     shifting terminals.
**  yy_reduce_ofst[]   For each state, the offset into yy_action for
**                     shifting non-terminals after a reduce.
**  yy_default[]       Default action for each state.
*/
#define YY_ACTTAB_COUNT (844)
static const YYACTIONTYPE yy_action[] = {
 /*     0 */   169,  228,  199,  199,   61,   61,   61,   61,   54,   59,
 /*    10 */    59,   59,   59,   58,   58,   57,   57,   57,   56,  122,
 /*    20 */    58,   58,   57,   57,   57,   56,  122,  215,  199,  199,
 /*    30 */    63,  120,   53,  193,  192,  194,  194,   60,   60,   61,
 /*    40 */    61,   61,   61,  229,   59,   59,   59,   59,   58,   58,
 /*    50 */    57,   57,   57,   56,  122,   62,   63,  120,   53,  193,
 /*    60 */   192,  194,  194,   60,   60,   61,   61,   61,   61,   83,
 /*    70 */    59,   59,   59,   59,   58,   58,   57,   57,   57,   56,
 /*    80 */   122,  180,  169,   55,   52,   89,  205,  205,   88,  199,
 /*    90 */   199,  218,  216,  125,   38,  190,   92,   97,  174,   61,
 /*   100 */    61,   61,   61,  174,   59,   59,   59,   59,   58,   58,
 /*   110 */    57,   57,   57,   56,  122,   95,   62,   63,  120,   53,
 /*   120 */   193,  192,  194,  194,   60,   60,   61,   61,   61,   61,
 /*   130 */    94,   59,   59,   59,   59,   58,   58,   57,   57,   57,
 /*   140 */    56,  122,  205,  205,  179,  100,  189,  205,  205,  227,
 /*   150 */   198,  198,  159,   41,   37,  178,  205,  205,  153,  214,
 /*   160 */   188,  205,  205,   75,  213,  187,  199,  199,  204,  203,
 /*   170 */    59,   59,   59,   59,   58,   58,   57,   57,   57,   56,
 /*   180 */   122,  196,  195,  210,  173,  134,   57,   57,   57,   56,
 /*   190 */   122,  211,   41,   62,   63,  120,   53,  193,  192,  194,
 /*   200 */   194,   60,   60,   61,   61,   61,   61,  163,   59,   59,
 /*   210 */    59,   59,   58,   58,   57,   57,   57,   56,  122,   55,
 /*   220 */    52,   89,  205,  205,  204,  203,  197,  199,  199,  204,
 /*   230 */   203,   75,  132,    1,   55,   52,   89,  161,  204,  203,
 /*   240 */   184,  133,  176,  204,  203,  172,   82,   12,   90,   86,
 /*   250 */   345,   80,  154,   13,   62,   63,  120,   53,  193,  192,
 /*   260 */   194,  194,   60,   60,   61,   61,   61,   61,  183,   59,
 /*   270 */    59,   59,   59,   58,   58,   57,   57,   57,   56,  122,
 /*   280 */    46,  150,   44,   28,   75,  144,  182,  207,  199,  199,
 /*   290 */   177,  208,   55,   52,   89,   99,   86,  143,  145,  150,
 /*   300 */    10,   17,   75,  142,  204,  203,  126,   36,  157,  157,
 /*   310 */     9,  209,   75,  147,  171,   62,   63,  120,   53,  193,
 /*   320 */   192,  194,  194,   60,   60,   61,   61,   61,   61,  209,
 /*   330 */    59,   59,   59,   59,   58,   58,   57,   57,   57,   56,
 /*   340 */   122,  150,  104,   28,  150,  150,   28,    6,   91,  199,
 /*   350 */   199,   33,  140,   88,  101,  156,  212,  143,  141,  166,
 /*   360 */   143,  128,  210,  174,  130,  130,  105,  127,  174,  207,
 /*   370 */   211,  209,   45,  138,  209,  209,   62,   63,  120,   53,
 /*   380 */   193,  192,  194,  194,   60,   60,   61,   61,   61,   61,
 /*   390 */   169,   59,   59,   59,   59,   58,   58,   57,   57,   57,
 /*   400 */    56,  122,  150,   96,   27,  150,  129,   28,  217,   13,
 /*   410 */   199,  199,   56,  122,  232,  232,  216,  125,   85,    2,
 /*   420 */     8,  136,  160,  158,  124,  168,  165,  155,  113,  164,
 /*   430 */    64,  103,  209,   42,   93,  209,  115,   62,   51,  120,
 /*   440 */    53,  193,  192,  194,  194,   60,   60,   61,   61,   61,
 /*   450 */    61,   43,   59,   59,   59,   59,   58,   58,   57,   57,
 /*   460 */    57,   56,  122,  150,  185,   28,  112,  199,  199,  137,
 /*   470 */   110,   35,  169,  139,  111,   39,   98,  119,  181,  121,
 /*   480 */    34,  118,   84,  116,  146,   81,   92,  102,    4,  107,
 /*   490 */   117,  199,  199,  209,   62,   63,  120,   53,  193,  192,
 /*   500 */   194,  194,   60,   60,   61,   61,   61,   61,  108,   59,
 /*   510 */    59,   59,   59,   58,   58,   57,   57,   57,   56,  122,
 /*   520 */   120,   53,  193,  192,  194,  194,   60,   60,   61,   61,
 /*   530 */    61,   61,  175,   59,   59,   59,   59,   58,   58,   57,
 /*   540 */    57,   57,   56,  122,  123,  123,  106,  122,  150,  346,
 /*   550 */    22,  149,  202,  186,  170,  167,  114,   87,  135,  150,
 /*   560 */   346,   32,  150,  150,   30,   71,  109,  150,  150,   70,
 /*   570 */    72,  346,  150,  150,   73,   74,  346,  346,  209,  150,
 /*   580 */   150,   76,   65,  150,  346,   69,  150,  346,   68,  209,
 /*   590 */   346,  346,  209,  209,  346,  346,  191,  209,  209,  346,
 /*   600 */   346,   50,  209,  209,  346,  346,  346,  123,  123,  209,
 /*   610 */   209,  346,  346,  209,  346,  346,  209,  346,   47,   48,
 /*   620 */   162,  346,  346,  346,  346,   49,  152,  151,  150,  346,
 /*   630 */    29,  150,  150,   67,   31,    3,  150,  150,   79,   78,
 /*   640 */   346,  346,  150,  150,   77,   16,  346,  346,  206,  201,
 /*   650 */   201,  201,  200,  148,   11,  150,  346,   26,  209,  191,
 /*   660 */   346,  209,  209,  150,   50,   25,  209,  209,  346,  346,
 /*   670 */   123,  123,  209,  209,  346,  150,  346,   15,  346,  346,
 /*   680 */   346,   47,   48,  346,  346,  209,  346,  346,   49,  152,
 /*   690 */   151,  346,  346,  209,  150,  150,   24,   23,    3,  341,
 /*   700 */   150,  150,   66,   14,  346,  209,  150,  150,   21,   20,
 /*   710 */   346,  206,  201,  201,  201,  200,  148,   11,  346,  150,
 /*   720 */   346,    7,  191,  346,  209,  209,  346,   50,  346,  346,
 /*   730 */   209,  209,  346,  123,  123,  346,  209,  209,  346,  346,
 /*   740 */   150,  346,   19,  346,   47,   48,  346,  346,  346,  209,
 /*   750 */   346,   49,  152,  151,  150,  346,   18,  131,  346,    5,
 /*   760 */   346,    3,  346,  346,  346,  346,  346,  346,  346,  346,
 /*   770 */   209,  346,  346,  346,  206,  201,  201,  201,  200,  148,
 /*   780 */    11,  346,  346,  346,  209,  191,  346,  209,  346,  346,
 /*   790 */    40,  346,  346,  346,  346,  346,  346,  346,  346,  346,
 /*   800 */   346,  346,  346,  346,  346,  346,  346,   47,   48,  346,
 /*   810 */   346,  346,  346,  346,   49,  152,  151,  346,  346,  346,
 /*   820 */   346,  346,  346,  346,    3,  346,  346,  346,  346,  346,
 /*   830 */   346,  346,  346,  346,  346,  346,  346,  206,  201,  201,
 /*   840 */   201,  200,  148,   11,
};
static const YYCODETYPE yy_lookahead[] = {
 /*     0 */     5,  100,   33,   34,   70,   71,   72,   73,   74,   75,
 /*    10 */    76,   77,   78,   79,   80,   81,   82,   83,   84,   85,
 /*    20 */    79,   80,   81,   82,   83,   84,   85,    1,   33,   34,
 /*    30 */    61,   62,   63,   64,   65,   66,   67,   68,   69,   70,
 /*    40 */    71,   72,   73,  100,   75,   76,   77,   78,   79,   80,
 /*    50 */    81,   82,   83,   84,   85,   60,   61,   62,   63,   64,
 /*    60 */    65,   66,   67,   68,   69,   70,   71,   72,   73,   96,
 /*    70 */    75,   76,   77,   78,   79,   80,   81,   82,   83,   84,
 /*    80 */    85,  155,   87,  157,  158,  159,    5,    6,   88,   33,
 /*    90 */    34,    0,    1,    2,   38,   97,  101,   97,   98,   70,
 /*   100 */    71,   72,   73,  103,   75,   76,   77,   78,   79,   80,
 /*   110 */    81,   82,   83,   84,   85,  100,   60,   61,   62,   63,
 /*   120 */    64,   65,   66,   67,   68,   69,   70,   71,   72,   73,
 /*   130 */   100,   75,   76,   77,   78,   79,   80,   81,   82,   83,
 /*   140 */    84,   85,    5,    6,   11,   96,   25,    5,    6,  100,
 /*   150 */    33,   34,   97,   98,   98,   22,    5,    6,  124,  125,
 /*   160 */    97,    5,    6,  129,  130,   97,   33,   34,   87,   88,
 /*   170 */    75,   76,   77,   78,   79,   80,   81,   82,   83,   84,
 /*   180 */    85,   64,   65,   91,  103,    6,   81,   82,   83,   84,
 /*   190 */    85,   99,   98,   60,   61,   62,   63,   64,   65,   66,
 /*   200 */    67,   68,   69,   70,   71,   72,   73,  145,   75,   76,
 /*   210 */    77,   78,   79,   80,   81,   82,   83,   84,   85,  157,
 /*   220 */   158,  159,    5,    6,   87,   88,  109,   33,   34,   87,
 /*   230 */    88,  129,  130,   96,  157,  158,  159,   81,   87,   88,
 /*   240 */    97,   62,   98,   87,   88,  103,  102,   96,  146,  147,
 /*   250 */   120,  121,  122,  123,   60,   61,   62,   63,   64,   65,
 /*   260 */    66,   67,   68,   69,   70,   71,   72,   73,   97,   75,
 /*   270 */    76,   77,   78,   79,   80,   81,   82,   83,   84,   85,
 /*   280 */   116,  126,  118,  128,  129,  130,   97,    5,   33,   34,
 /*   290 */   155,   97,  157,  158,  159,  146,  147,  142,  143,  126,
 /*   300 */    15,  128,  129,  130,   87,   88,   90,   98,   92,   93,
 /*   310 */    15,  156,  129,  130,  103,   60,   61,   62,   63,   64,
 /*   320 */    65,   66,   67,   68,   69,   70,   71,   72,   73,  156,
 /*   330 */    75,   76,   77,   78,   79,   80,   81,   82,   83,   84,
 /*   340 */    85,  126,  135,  128,  126,  126,  128,  128,   15,   33,
 /*   350 */    34,  144,   81,   88,  131,  132,  132,  142,  143,    6,
 /*   360 */   142,  143,   91,   98,  141,  141,   94,   95,  103,   87,
 /*   370 */    99,  156,  117,  154,  156,  156,   60,   61,   62,   63,
 /*   380 */    64,   65,   66,   67,   68,   69,   70,   71,   72,   73,
 /*   390 */     5,   75,   76,   77,   78,   79,   80,   81,   82,   83,
 /*   400 */    84,   85,  126,   97,  128,  126,   98,  128,  122,  123,
 /*   410 */    33,   34,   84,   85,   94,   95,    1,    2,  100,   96,
 /*   420 */    96,  142,   97,   91,    3,  152,  152,    4,  127,  152,
 /*   430 */    89,  161,  156,  117,  100,  156,  160,   60,   61,   62,
 /*   440 */    63,   64,   65,   66,   67,   68,   69,   70,   71,   72,
 /*   450 */    73,  116,   75,   76,   77,   78,   79,   80,   81,   82,
 /*   460 */    83,   84,   85,  126,  149,  128,  137,   33,   34,  106,
 /*   470 */   139,  107,   87,  105,  138,  108,  149,  151,  140,  142,
 /*   480 */   104,  151,  134,  136,  162,  133,  101,  145,  134,  145,
 /*   490 */   151,   33,   34,  156,   60,   61,   62,   63,   64,   65,
 /*   500 */    66,   67,   68,   69,   70,   71,   72,   73,  150,   75,
 /*   510 */    76,   77,   78,   79,   80,   81,   82,   83,   84,   85,
 /*   520 */    62,   63,   64,   65,   66,   67,   68,   69,   70,   71,
 /*   530 */    72,   73,  148,   75,   76,   77,   78,   79,   80,   81,
 /*   540 */    82,   83,   84,   85,    5,    6,  145,   85,  126,  163,
 /*   550 */   128,  126,  126,  126,  126,  126,  126,  126,  126,  126,
 /*   560 */   163,  128,  126,  126,  128,  128,  126,  126,  126,  128,
 /*   570 */   128,  163,  126,  126,  128,  128,  163,  163,  156,  126,
 /*   580 */   126,  128,  128,  126,  163,  128,  126,  163,  128,  156,
 /*   590 */   163,  163,  156,  156,  163,  163,   57,  156,  156,  163,
 /*   600 */   163,   62,  156,  156,  163,  163,  163,    5,    6,  156,
 /*   610 */   156,  163,  163,  156,  163,  163,  156,  163,   79,   80,
 /*   620 */    81,  163,  163,  163,  163,   86,   87,   88,  126,  163,
 /*   630 */   128,  126,  126,  128,  128,   96,  126,  126,  128,  128,
 /*   640 */   163,  163,  126,  126,  128,  128,  163,  163,  109,  110,
 /*   650 */   111,  112,  113,  114,  115,  126,  163,  128,  156,   57,
 /*   660 */   163,  156,  156,  126,   62,  128,  156,  156,  163,  163,
 /*   670 */     5,    6,  156,  156,  163,  126,  163,  128,  163,  163,
 /*   680 */   163,   79,   80,  163,  163,  156,  163,  163,   86,   87,
 /*   690 */    88,  163,  163,  156,  126,  126,  128,  128,   96,   97,
 /*   700 */   126,  126,  128,  128,  163,  156,  126,  126,  128,  128,
 /*   710 */   163,  109,  110,  111,  112,  113,  114,  115,  163,  126,
 /*   720 */   163,  128,   57,  163,  156,  156,  163,   62,  163,  163,
 /*   730 */   156,  156,  163,    5,    6,  163,  156,  156,  163,  163,
 /*   740 */   126,  163,  128,  163,   79,   80,  163,  163,  163,  156,
 /*   750 */   163,   86,   87,   88,  126,  163,  128,  126,  163,  128,
 /*   760 */   163,   96,  163,  163,  163,  163,  163,  163,  163,  163,
 /*   770 */   156,  163,  163,  163,  109,  110,  111,  112,  113,  114,
 /*   780 */   115,  163,  163,  163,  156,   57,  163,  156,  163,  163,
 /*   790 */    62,  163,  163,  163,  163,  163,  163,  163,  163,  163,
 /*   800 */   163,  163,  163,  163,  163,  163,  163,   79,   80,  163,
 /*   810 */   163,  163,  163,  163,   86,   87,   88,  163,  163,  163,
 /*   820 */   163,  163,  163,  163,   96,  163,  163,  163,  163,  163,
 /*   830 */   163,  163,  163,  163,  163,  163,  163,  109,  110,  111,
 /*   840 */   112,  113,  114,  115,
};
#define YY_SHIFT_USE_DFLT (-100)
#define YY_SHIFT_COUNT (153)
#define YY_SHIFT_MIN   (-99)
#define YY_SHIFT_MAX   (728)
static const short yy_shift_ofst[] = {
 /*     0 */   415,  602,  665,  665,  665,   -5,  133,  133,  665,  665,
 /*    10 */   665,  665,  320, -100,   56,  316,  255,  194,  434,  434,
 /*    20 */   434,  434,  434,  434,  434,  434,  434,  434,  434,  377,
 /*    30 */   -31,  458,  458,  539,  665,  665,  665,  665,  665,  665,
 /*    40 */   665,  665,  665,  665,  665,  665,  665,  665,  665,  665,
 /*    50 */   665,  665,  665,  728,  665,  665,  665,  665,  665,  665,
 /*    60 */   665,  665,  665,  665,  665,  -66,   29,   29,   29,   29,
 /*    70 */    29,   95,  -59,  105,  328,  272,  462, -100, -100, -100,
 /*    80 */    91,  272, -100, -100, -100,  156,  151,  142,   81,  137,
 /*    90 */     0,  217,  217,  217,  217,  217,  385,  385,  385,  265,
 /*   100 */   271,  216,  179,  164,  144,   92,  376,  376,  376,  334,
 /*   110 */   367,  368,  364,  363,  334,  335,  341, -100, -100, -100,
 /*   120 */   117,   55,  282,   49,  423,  421,  332,  324,  325,  323,
 /*   130 */   308,  318,  306,  353,  333,  211,   94,  295,  209,  285,
 /*   140 */   189,  171,  143,   94,   68,   63,  121,   -2,  -27,   30,
 /*   150 */    15,  -57,  -99,   26,
};
#define YY_REDUCE_USE_DFLT (-75)
#define YY_REDUCE_COUNT (119)
#define YY_REDUCE_MIN   (-74)
#define YY_REDUCE_MAX   (631)
static const short yy_reduce_ofst[] = {
 /*     0 */   130,  155,  218,  173,  215,   62,  135,  -74,  337,  279,
 /*    10 */   219,  276,  102,   34,   77,   77,   77,   77,   77,   77,
 /*    20 */    77,   77,   77,   77,   77,   77,   77,   77,   77,   77,
 /*    30 */    77,   77,   77,  631,  628,  614,  593,  581,  580,  575,
 /*    40 */   574,  569,  568,  549,  537,  529,  517,  516,  511,  510,
 /*    50 */   506,  505,  502,  460,  457,  454,  453,  447,  446,  442,
 /*    60 */   441,  437,  436,  433,  422,   77,   77,   77,   77,   77,
 /*    70 */    77,   77,   77,   77,   77,  223,   77,   77,   77,   77,
 /*    80 */   286,  224,  149,  183,  207,  425,  440,  432,  431,  430,
 /*    90 */   384,  429,  428,  427,  426,  425,  401,  344,  342,  384,
 /*   100 */   354,  352,  358,  322,  347,  348,  339,  330,  326,  327,
 /*   110 */   338,  331,  336,  329,  315,  270,  301,  277,  274,  273,
};
static const YYACTIONTYPE yy_default[] = {
 /*     0 */   223,  232,  341,  232,  341,  254,  280,  280,  344,  344,
 /*    10 */   344,  339,  258,  232,  286,  344,  344,  344,  268,  284,
 /*    20 */   288,  287,  231,  342,  335,  336,  334,  338,  343,  344,
 /*    30 */   304,  319,  303,  344,  344,  344,  344,  344,  344,  344,
 /*    40 */   344,  344,  344,  344,  344,  344,  344,  344,  344,  344,
 /*    50 */   344,  344,  344,  344,  344,  344,  344,  344,  344,  344,
 /*    60 */   344,  344,  344,  344,  344,  313,  318,  325,  317,  314,
 /*    70 */   306,  305,  307,  308,  309,  344,  310,  322,  321,  320,
 /*    80 */   223,  344,  258,  232,  248,  344,  344,  344,  344,  344,
 /*    90 */   344,  344,  344,  344,  344,  344,  254,  254,  254,  256,
 /*   100 */   246,  234,  270,  337,  255,  246,  269,  269,  269,  262,
 /*   110 */   285,  274,  283,  281,  262,  344,  230,  273,  273,  273,
 /*   120 */   344,  344,  344,  292,  344,  224,  237,  344,  344,  344,
 /*   130 */   241,  344,  344,  344,  344,  344,  282,  344,  275,  344,
 /*   140 */   344,  344,  344,  340,  344,  344,  344,  344,  344,  294,
 /*   150 */   344,  293,  297,  344,  220,  225,  235,  239,  238,  242,
 /*   160 */   243,  251,  250,  249,  260,  261,  272,  271,  259,  253,
 /*   170 */   252,  267,  266,  265,  264,  257,  247,  277,  279,  278,
 /*   180 */   276,  240,  301,  300,  329,  331,  263,  330,  328,  333,
 /*   190 */   332,  302,  326,  323,  315,  327,  324,  316,  312,  311,
 /*   200 */   298,  296,  295,  229,  228,  227,  291,  299,  290,  289,
 /*   210 */   245,  244,  236,  233,  226,  222,  221,  219,
};

/* The next table maps tokens into fallback tokens.  If a construct
** like the following:
** 
**      %fallback ID X Y Z.
**
** appears in the grammar, then ID becomes a fallback token for X, Y,
** and Z.  Whenever one of the tokens X, Y, or Z is input to the parser
** but it does not parse, the type of the token is changed to ID and
** the parse is retried before an error is thrown.
*/
#ifdef YYFALLBACK
static const YYCODETYPE yyFallback[] = {
    0,  /*          $ => nothing */
    0,  /*       SEMI => nothing */
    5,  /*    EXPLAIN => ID */
    5,  /*      QUERY => ID */
    5,  /*       PLAN => ID */
    0,  /*         ID => nothing */
    0,  /*    INDEXED => nothing */
    5,  /*      ABORT => ID */
    5,  /*     ACTION => ID */
    5,  /*      AFTER => ID */
    5,  /*    ANALYZE => ID */
    5,  /*        ASC => ID */
    5,  /*     ATTACH => ID */
    5,  /*     BEFORE => ID */
    5,  /*      BEGIN => ID */
    5,  /*         BY => ID */
    5,  /*    CASCADE => ID */
    5,  /*       CAST => ID */
    5,  /*   COLUMNKW => ID */
    5,  /*   CONFLICT => ID */
    5,  /*   DATABASE => ID */
    5,  /*   DEFERRED => ID */
    5,  /*       DESC => ID */
    5,  /*     DETACH => ID */
    5,  /*       EACH => ID */
    5,  /*        END => ID */
    5,  /*  EXCLUSIVE => ID */
    5,  /*       FAIL => ID */
    5,  /*        FOR => ID */
    5,  /*     IGNORE => ID */
    5,  /*  IMMEDIATE => ID */
    5,  /*  INITIALLY => ID */
    5,  /*    INSTEAD => ID */
    5,  /*    LIKE_KW => ID */
    5,  /*      MATCH => ID */
    5,  /*         NO => ID */
    5,  /*        KEY => ID */
    5,  /*         OF => ID */
    5,  /*     OFFSET => ID */
    5,  /*     PRAGMA => ID */
    5,  /*      RAISE => ID */
    5,  /*  RECURSIVE => ID */
    5,  /*    RELEASE => ID */
    5,  /*    REPLACE => ID */
    5,  /*   RESTRICT => ID */
    5,  /*        ROW => ID */
    5,  /*   ROLLBACK => ID */
    5,  /*  SAVEPOINT => ID */
    5,  /*       TEMP => ID */
    5,  /*    TRIGGER => ID */
    5,  /*     VACUUM => ID */
    5,  /*       VIEW => ID */
    5,  /*    VIRTUAL => ID */
    5,  /*       WITH => ID */
    5,  /*    WITHOUT => ID */
    5,  /*    REINDEX => ID */
    5,  /*     RENAME => ID */
    5,  /*   CTIME_KW => ID */
    5,  /*         IF => ID */
};
#endif /* YYFALLBACK */

/* The following structure represents a single element of the
** parser's stack.  Information stored includes:
**
**   +  The state number for the parser at this level of the stack.
**
**   +  The value of the token stored at this level of the stack.
**      (In other words, the "major" token.)
**
**   +  The semantic value stored at this level of the stack.  This is
**      the information used by the action routines in the grammar.
**      It is sometimes called the "minor" token.
*/
struct yyStackEntry {
  YYACTIONTYPE stateno;  /* The state-number */
  YYCODETYPE major;      /* The major token value.  This is the code
                         ** number for the token at this stack level */
  YYMINORTYPE minor;     /* The user-supplied minor token value.  This
                         ** is the value of the token  */
};
typedef struct yyStackEntry yyStackEntry;

/* The state of the parser is completely contained in an instance of
** the following structure */
struct yyParser {
  int yyidx;                    /* Index of top element in stack */
#ifdef YYTRACKMAXSTACKDEPTH
  int yyidxMax;                 /* Maximum value of yyidx */
#endif
  int yyerrcnt;                 /* Shifts left before out of the error */
  thorParserARG_SDECL                /* A place to hold %extra_argument */
#if YYSTACKDEPTH<=0
  int yystksz;                  /* Current side of the stack */
  yyStackEntry *yystack;        /* The parser's stack */
#else
  yyStackEntry yystack[YYSTACKDEPTH];  /* The parser's stack */
#endif
};
typedef struct yyParser yyParser;

#ifndef NDEBUG
#include <stdio.h>
static FILE *yyTraceFILE = 0;
static char *yyTracePrompt = 0;
#endif /* NDEBUG */

#ifndef NDEBUG
/* 
** Turn parser tracing on by giving a stream to which to write the trace
** and a prompt to preface each trace message.  Tracing is turned off
** by making either argument NULL 
**
** Inputs:
** <ul>
** <li> A FILE* to which trace output should be written.
**      If NULL, then tracing is turned off.
** <li> A prefix string written at the beginning of every
**      line of trace output.  If NULL, then tracing is
**      turned off.
** </ul>
**
** Outputs:
** None.
*/
void thorParserTrace(FILE *TraceFILE, char *zTracePrompt){
  yyTraceFILE = TraceFILE;
  yyTracePrompt = zTracePrompt;
  if( yyTraceFILE==0 ) yyTracePrompt = 0;
  else if( yyTracePrompt==0 ) yyTraceFILE = 0;
}
#endif /* NDEBUG */

#ifndef NDEBUG
/* For tracing shifts, the names of all terminals and nonterminals
** are required.  The following table supplies these names */
static const char *const yyTokenName[] = { 
  "$",             "SEMI",          "EXPLAIN",       "QUERY",       
  "PLAN",          "ID",            "INDEXED",       "ABORT",       
  "ACTION",        "AFTER",         "ANALYZE",       "ASC",         
  "ATTACH",        "BEFORE",        "BEGIN",         "BY",          
  "CASCADE",       "CAST",          "COLUMNKW",      "CONFLICT",    
  "DATABASE",      "DEFERRED",      "DESC",          "DETACH",      
  "EACH",          "END",           "EXCLUSIVE",     "FAIL",        
  "FOR",           "IGNORE",        "IMMEDIATE",     "INITIALLY",   
  "INSTEAD",       "LIKE_KW",       "MATCH",         "NO",          
  "KEY",           "OF",            "OFFSET",        "PRAGMA",      
  "RAISE",         "RECURSIVE",     "RELEASE",       "REPLACE",     
  "RESTRICT",      "ROW",           "ROLLBACK",      "SAVEPOINT",   
  "TEMP",          "TRIGGER",       "VACUUM",        "VIEW",        
  "VIRTUAL",       "WITH",          "WITHOUT",       "REINDEX",     
  "RENAME",        "CTIME_KW",      "IF",            "ANY",         
  "OR",            "AND",           "NOT",           "IS",          
  "BETWEEN",       "IN",            "ISNULL",        "NOTNULL",     
  "NE",            "EQ",            "GT",            "LE",          
  "LT",            "GE",            "ESCAPE",        "BITAND",      
  "BITOR",         "LSHIFT",        "RSHIFT",        "PLUS",        
  "MINUS",         "STAR",          "SLASH",         "REM",         
  "CONCAT",        "COLLATE",       "BITNOT",        "STRING",      
  "JOIN_KW",       "WHERE",         "UNION",         "ALL",         
  "EXCEPT",        "INTERSECT",     "SELECT",        "VALUES",      
  "LP",            "RP",            "COMMA",         "DISTINCT",    
  "DOT",           "AS",            "FROM",          "JOIN",        
  "ON",            "ORDER",         "GROUP",         "HAVING",      
  "LIMIT",         "NULL",          "INTEGER",       "FLOAT",       
  "BLOB",          "VARIABLE",      "EXISTS",        "CASE",        
  "WHEN",          "THEN",          "ELSE",          "error",       
  "input",         "cmdlist",       "ecmd",          "explain",     
  "cmdx",          "cmd",           "nm",            "where_opt",   
  "expr",          "with",          "select",        "selectnowith",
  "oneselect",     "multiselect_op",  "distinct",      "selcollist",  
  "from",          "groupby_opt",   "having_opt",    "orderby_opt", 
  "limit_opt",     "values",        "nexprlist",     "exprlist",    
  "sclp",          "as",            "seltablist",    "stl_prefix",  
  "joinop",        "dbnm",          "indexed_opt",   "on_opt",      
  "using_opt",     "joinop2",       "sortlist",      "sortorder",   
  "term",          "likeop",        "between_op",    "in_op",       
  "case_operand",  "case_exprlist",  "case_else",   
};
#endif /* NDEBUG */

#ifndef NDEBUG
/* For tracing reduce actions, the names of all rules are required.
*/
static const char *const yyRuleName[] = {
 /*   0 */ "input ::= cmdlist",
 /*   1 */ "cmdlist ::= cmdlist ecmd",
 /*   2 */ "cmdlist ::= ecmd",
 /*   3 */ "ecmd ::= SEMI",
 /*   4 */ "ecmd ::= explain cmdx SEMI",
 /*   5 */ "explain ::=",
 /*   6 */ "explain ::= EXPLAIN",
 /*   7 */ "explain ::= EXPLAIN QUERY PLAN",
 /*   8 */ "cmdx ::= cmd",
 /*   9 */ "nm ::= ID|INDEXED",
 /*  10 */ "nm ::= STRING",
 /*  11 */ "nm ::= JOIN_KW",
 /*  12 */ "where_opt ::=",
 /*  13 */ "where_opt ::= WHERE expr",
 /*  14 */ "with ::=",
 /*  15 */ "cmd ::= select",
 /*  16 */ "select ::= with selectnowith",
 /*  17 */ "selectnowith ::= oneselect",
 /*  18 */ "selectnowith ::= selectnowith multiselect_op oneselect",
 /*  19 */ "multiselect_op ::= UNION",
 /*  20 */ "multiselect_op ::= UNION ALL",
 /*  21 */ "multiselect_op ::= EXCEPT|INTERSECT",
 /*  22 */ "oneselect ::= SELECT distinct selcollist from where_opt groupby_opt having_opt orderby_opt limit_opt",
 /*  23 */ "oneselect ::= values",
 /*  24 */ "values ::= VALUES LP nexprlist RP",
 /*  25 */ "values ::= values COMMA LP exprlist RP",
 /*  26 */ "distinct ::= DISTINCT",
 /*  27 */ "distinct ::= ALL",
 /*  28 */ "distinct ::=",
 /*  29 */ "sclp ::= selcollist COMMA",
 /*  30 */ "sclp ::=",
 /*  31 */ "selcollist ::= sclp expr as",
 /*  32 */ "selcollist ::= sclp STAR",
 /*  33 */ "selcollist ::= sclp nm DOT STAR",
 /*  34 */ "as ::= AS nm",
 /*  35 */ "as ::= ID|STRING",
 /*  36 */ "as ::=",
 /*  37 */ "from ::=",
 /*  38 */ "from ::= FROM seltablist",
 /*  39 */ "stl_prefix ::= seltablist joinop",
 /*  40 */ "stl_prefix ::=",
 /*  41 */ "seltablist ::= stl_prefix nm dbnm as indexed_opt on_opt using_opt",
 /*  42 */ "seltablist ::= stl_prefix LP select RP as on_opt using_opt",
 /*  43 */ "seltablist ::= stl_prefix LP seltablist RP as on_opt using_opt",
 /*  44 */ "dbnm ::=",
 /*  45 */ "dbnm ::= DOT nm",
 /*  46 */ "joinop ::= COMMA|JOIN",
 /*  47 */ "joinop ::= JOIN_KW JOIN",
 /*  48 */ "joinop ::= JOIN_KW nm JOIN",
 /*  49 */ "joinop ::= JOIN_KW nm nm JOIN",
 /*  50 */ "on_opt ::= ON expr",
 /*  51 */ "on_opt ::=",
 /*  52 */ "indexed_opt ::=",
 /*  53 */ "indexed_opt ::= INDEXED BY nm",
 /*  54 */ "indexed_opt ::= NOT INDEXED",
 /*  55 */ "using_opt ::=",
 /*  56 */ "orderby_opt ::=",
 /*  57 */ "orderby_opt ::= ORDER BY sortlist",
 /*  58 */ "sortlist ::= sortlist COMMA expr sortorder",
 /*  59 */ "sortlist ::= expr sortorder",
 /*  60 */ "sortorder ::= ASC",
 /*  61 */ "sortorder ::= DESC",
 /*  62 */ "sortorder ::=",
 /*  63 */ "groupby_opt ::=",
 /*  64 */ "groupby_opt ::= GROUP BY nexprlist",
 /*  65 */ "having_opt ::=",
 /*  66 */ "having_opt ::= HAVING expr",
 /*  67 */ "limit_opt ::=",
 /*  68 */ "limit_opt ::= LIMIT expr",
 /*  69 */ "limit_opt ::= LIMIT expr OFFSET expr",
 /*  70 */ "limit_opt ::= LIMIT expr COMMA expr",
 /*  71 */ "expr ::= term",
 /*  72 */ "expr ::= LP expr RP",
 /*  73 */ "term ::= NULL",
 /*  74 */ "expr ::= ID|INDEXED",
 /*  75 */ "expr ::= JOIN_KW",
 /*  76 */ "expr ::= nm DOT nm",
 /*  77 */ "expr ::= nm DOT nm DOT nm",
 /*  78 */ "term ::= INTEGER|FLOAT|BLOB",
 /*  79 */ "term ::= STRING",
 /*  80 */ "expr ::= VARIABLE",
 /*  81 */ "expr ::= expr COLLATE ID|STRING",
 /*  82 */ "expr ::= ID|INDEXED LP distinct exprlist RP",
 /*  83 */ "expr ::= ID|INDEXED LP STAR RP",
 /*  84 */ "term ::= CTIME_KW",
 /*  85 */ "expr ::= expr AND expr",
 /*  86 */ "expr ::= expr OR expr",
 /*  87 */ "expr ::= expr LT|GT|GE|LE expr",
 /*  88 */ "expr ::= expr EQ|NE expr",
 /*  89 */ "expr ::= expr BITAND|BITOR|LSHIFT|RSHIFT expr",
 /*  90 */ "expr ::= expr PLUS|MINUS expr",
 /*  91 */ "expr ::= expr STAR|SLASH|REM expr",
 /*  92 */ "expr ::= expr CONCAT expr",
 /*  93 */ "likeop ::= LIKE_KW|MATCH",
 /*  94 */ "likeop ::= NOT LIKE_KW|MATCH",
 /*  95 */ "expr ::= expr likeop expr",
 /*  96 */ "expr ::= expr likeop expr ESCAPE expr",
 /*  97 */ "expr ::= expr ISNULL|NOTNULL",
 /*  98 */ "expr ::= expr NOT NULL",
 /*  99 */ "expr ::= expr IS expr",
 /* 100 */ "expr ::= expr IS NOT expr",
 /* 101 */ "expr ::= NOT expr",
 /* 102 */ "expr ::= BITNOT expr",
 /* 103 */ "expr ::= MINUS expr",
 /* 104 */ "expr ::= PLUS expr",
 /* 105 */ "between_op ::= BETWEEN",
 /* 106 */ "between_op ::= NOT BETWEEN",
 /* 107 */ "expr ::= expr between_op expr AND expr",
 /* 108 */ "in_op ::= IN",
 /* 109 */ "in_op ::= NOT IN",
 /* 110 */ "expr ::= expr in_op LP exprlist RP",
 /* 111 */ "expr ::= LP select RP",
 /* 112 */ "expr ::= expr in_op LP select RP",
 /* 113 */ "expr ::= expr in_op nm dbnm",
 /* 114 */ "expr ::= EXISTS LP select RP",
 /* 115 */ "expr ::= CASE case_operand case_exprlist case_else END",
 /* 116 */ "case_exprlist ::= case_exprlist WHEN expr THEN expr",
 /* 117 */ "case_exprlist ::= WHEN expr THEN expr",
 /* 118 */ "case_else ::= ELSE expr",
 /* 119 */ "case_else ::=",
 /* 120 */ "case_operand ::= expr",
 /* 121 */ "case_operand ::=",
 /* 122 */ "exprlist ::= nexprlist",
 /* 123 */ "exprlist ::=",
 /* 124 */ "nexprlist ::= nexprlist COMMA expr",
 /* 125 */ "nexprlist ::= expr",
};
#endif /* NDEBUG */


#if YYSTACKDEPTH<=0
/*
** Try to increase the size of the parser stack.
*/
static void yyGrowStack(yyParser *p){
  int newSize;
  yyStackEntry *pNew;

  newSize = p->yystksz*2 + 100;
  pNew = realloc(p->yystack, newSize*sizeof(pNew[0]));
  if( pNew ){
    p->yystack = pNew;
    p->yystksz = newSize;
#ifndef NDEBUG
    if( yyTraceFILE ){
      fprintf(yyTraceFILE,"%sStack grows to %d entries!\n",
              yyTracePrompt, p->yystksz);
    }
#endif
  }
}
#endif

/* 
** This function allocates a new parser.
** The only argument is a pointer to a function which works like
** malloc.
**
** Inputs:
** A pointer to the function used to allocate memory.
**
** Outputs:
** A pointer to a parser.  This pointer is used in subsequent calls
** to thorParser and thorParserFree.
*/
void *thorParserAlloc(void *(*mallocProc)(size_t)){
  yyParser *pParser;
  pParser = (yyParser*)(*mallocProc)( (size_t)sizeof(yyParser) );
  if( pParser ){
    pParser->yyidx = -1;
#ifdef YYTRACKMAXSTACKDEPTH
    pParser->yyidxMax = 0;
#endif
#if YYSTACKDEPTH<=0
    pParser->yystack = NULL;
    pParser->yystksz = 0;
    yyGrowStack(pParser);
#endif
  }
  return pParser;
}

/* The following function deletes the value associated with a
** symbol.  The symbol can be either a terminal or nonterminal.
** "yymajor" is the symbol code, and "yypminor" is a pointer to
** the value.
*/
static void yy_destructor(
  yyParser *yypParser,    /* The parser */
  YYCODETYPE yymajor,     /* Type code for object to destroy */
  YYMINORTYPE *yypminor   /* The object to be destroyed */
){
  thorParserARG_FETCH;
  switch( yymajor ){
    /* Here is inserted the actions which take place when a
    ** terminal or non-terminal is destroyed.  This can happen
    ** when the symbol is popped from the stack during a
    ** reduce or during error processing or when a parser is 
    ** being destroyed before it is finished parsing.
    **
    ** Note: during a reduce, the only symbols destroyed are those
    ** which appear on the RHS of the rule, but which are not used
    ** inside the C code.
    */
    case 127: /* where_opt */
    case 138: /* having_opt */
    case 151: /* on_opt */
    case 160: /* case_operand */
    case 162: /* case_else */
{
#line 143 ".\\thor\\sql\\new_parse.y"
sqlite3ExprDelete(pParse->db, (yypminor->yy66));
#line 917 ".\\thor\\sql\\new_parse.c"
}
      break;
    case 128: /* expr */
    case 156: /* term */
{
#line 438 ".\\thor\\sql\\new_parse.y"
sqlite3ExprDelete(pParse->db, (yypminor->yy266).pExpr);
#line 925 ".\\thor\\sql\\new_parse.c"
}
      break;
    case 129: /* with */
{
#line 148 ".\\thor\\sql\\new_parse.y"
sqlite3WithDelete(pParse->db, (yypminor->yy197));
#line 932 ".\\thor\\sql\\new_parse.c"
}
      break;
    case 130: /* select */
    case 131: /* selectnowith */
    case 132: /* oneselect */
    case 141: /* values */
{
#line 164 ".\\thor\\sql\\new_parse.y"
sqlite3SelectDelete(pParse->db, (yypminor->yy273));
#line 942 ".\\thor\\sql\\new_parse.c"
}
      break;
    case 135: /* selcollist */
    case 137: /* groupby_opt */
    case 139: /* orderby_opt */
    case 142: /* nexprlist */
    case 143: /* exprlist */
    case 144: /* sclp */
    case 154: /* sortlist */
    case 161: /* case_exprlist */
{
#line 253 ".\\thor\\sql\\new_parse.y"
sqlite3ExprListDelete(pParse->db, (yypminor->yy70));
#line 956 ".\\thor\\sql\\new_parse.c"
}
      break;
    case 136: /* from */
    case 146: /* seltablist */
    case 147: /* stl_prefix */
{
#line 288 ".\\thor\\sql\\new_parse.y"
sqlite3SrcListDelete(pParse->db, (yypminor->yy269));
#line 965 ".\\thor\\sql\\new_parse.c"
}
      break;
    case 152: /* using_opt */
{
#line 376 ".\\thor\\sql\\new_parse.y"
sqlite3IdListDelete(pParse->db, (yypminor->yy6));
#line 972 ".\\thor\\sql\\new_parse.c"
}
      break;
    default:  break;   /* If no destructor action specified: do nothing */
  }
}

/*
** Pop the parser's stack once.
**
** If there is a destructor routine associated with the token which
** is popped from the stack, then call it.
**
** Return the major token number for the symbol popped.
*/
static int yy_pop_parser_stack(yyParser *pParser){
  YYCODETYPE yymajor;
  yyStackEntry *yytos = &pParser->yystack[pParser->yyidx];

  if( pParser->yyidx<0 ) return 0;
#ifndef NDEBUG
  if( yyTraceFILE && pParser->yyidx>=0 ){
    fprintf(yyTraceFILE,"%sPopping %s\n",
      yyTracePrompt,
      yyTokenName[yytos->major]);
  }
#endif
  yymajor = yytos->major;
  yy_destructor(pParser, yymajor, &yytos->minor);
  pParser->yyidx--;
  return yymajor;
}

/* 
** Deallocate and destroy a parser.  Destructors are all called for
** all stack elements before shutting the parser down.
**
** Inputs:
** <ul>
** <li>  A pointer to the parser.  This should be a pointer
**       obtained from thorParserAlloc.
** <li>  A pointer to a function used to reclaim memory obtained
**       from malloc.
** </ul>
*/
void thorParserFree(
  void *p,                    /* The parser to be deleted */
  void (*freeProc)(void*)     /* Function used to reclaim memory */
){
  yyParser *pParser = (yyParser*)p;
  if( pParser==0 ) return;
  while( pParser->yyidx>=0 ) yy_pop_parser_stack(pParser);
#if YYSTACKDEPTH<=0
  free(pParser->yystack);
#endif
  (*freeProc)((void*)pParser);
}

/*
** Return the peak depth of the stack for a parser.
*/
#ifdef YYTRACKMAXSTACKDEPTH
int thorParserStackPeak(void *p){
  yyParser *pParser = (yyParser*)p;
  return pParser->yyidxMax;
}
#endif

/*
** Find the appropriate action for a parser given the terminal
** look-ahead token iLookAhead.
**
** If the look-ahead token is YYNOCODE, then check to see if the action is
** independent of the look-ahead.  If it is, return the action, otherwise
** return YY_NO_ACTION.
*/
static int yy_find_shift_action(
  yyParser *pParser,        /* The parser */
  YYCODETYPE iLookAhead     /* The look-ahead token */
){
  int i;
  int stateno = pParser->yystack[pParser->yyidx].stateno;
 
  if( stateno>YY_SHIFT_COUNT
   || (i = yy_shift_ofst[stateno])==YY_SHIFT_USE_DFLT ){
    return yy_default[stateno];
  }
  assert( iLookAhead!=YYNOCODE );
  i += iLookAhead;
  if( i<0 || i>=YY_ACTTAB_COUNT || yy_lookahead[i]!=iLookAhead ){
    if( iLookAhead>0 ){
#ifdef YYFALLBACK
      YYCODETYPE iFallback;            /* Fallback token */
      if( iLookAhead<sizeof(yyFallback)/sizeof(yyFallback[0])
             && (iFallback = yyFallback[iLookAhead])!=0 ){
#ifndef NDEBUG
        if( yyTraceFILE ){
          fprintf(yyTraceFILE, "%sFALLBACK %s => %s\n",
             yyTracePrompt, yyTokenName[iLookAhead], yyTokenName[iFallback]);
        }
#endif
        return yy_find_shift_action(pParser, iFallback);
      }
#endif
#ifdef YYWILDCARD
      {
        int j = i - iLookAhead + YYWILDCARD;
        if( 
#if YY_SHIFT_MIN+YYWILDCARD<0
          j>=0 &&
#endif
#if YY_SHIFT_MAX+YYWILDCARD>=YY_ACTTAB_COUNT
          j<YY_ACTTAB_COUNT &&
#endif
          yy_lookahead[j]==YYWILDCARD
        ){
#ifndef NDEBUG
          if( yyTraceFILE ){
            fprintf(yyTraceFILE, "%sWILDCARD %s => %s\n",
               yyTracePrompt, yyTokenName[iLookAhead], yyTokenName[YYWILDCARD]);
          }
#endif /* NDEBUG */
          return yy_action[j];
        }
      }
#endif /* YYWILDCARD */
    }
    return yy_default[stateno];
  }else{
    return yy_action[i];
  }
}

/*
** Find the appropriate action for a parser given the non-terminal
** look-ahead token iLookAhead.
**
** If the look-ahead token is YYNOCODE, then check to see if the action is
** independent of the look-ahead.  If it is, return the action, otherwise
** return YY_NO_ACTION.
*/
static int yy_find_reduce_action(
  int stateno,              /* Current state number */
  YYCODETYPE iLookAhead     /* The look-ahead token */
){
  int i;
#ifdef YYERRORSYMBOL
  if( stateno>YY_REDUCE_COUNT ){
    return yy_default[stateno];
  }
#else
  assert( stateno<=YY_REDUCE_COUNT );
#endif
  i = yy_reduce_ofst[stateno];
  assert( i!=YY_REDUCE_USE_DFLT );
  assert( iLookAhead!=YYNOCODE );
  i += iLookAhead;
#ifdef YYERRORSYMBOL
  if( i<0 || i>=YY_ACTTAB_COUNT || yy_lookahead[i]!=iLookAhead ){
    return yy_default[stateno];
  }
#else
  assert( i>=0 && i<YY_ACTTAB_COUNT );
  assert( yy_lookahead[i]==iLookAhead );
#endif
  return yy_action[i];
}

/*
** The following routine is called if the stack overflows.
*/
static void yyStackOverflow(yyParser *yypParser, YYMINORTYPE *yypMinor){
   thorParserARG_FETCH;
   yypParser->yyidx--;
#ifndef NDEBUG
   if( yyTraceFILE ){
     fprintf(yyTraceFILE,"%sStack Overflow!\n",yyTracePrompt);
   }
#endif
   while( yypParser->yyidx>=0 ) yy_pop_parser_stack(yypParser);
   /* Here code is inserted which will execute if the parser
   ** stack every overflows */
#line 23 ".\\thor\\sql\\new_parse.y"

    UNUSED_PARAMETER(yypMinor); /* Silence some compiler warnings */  
    thorStackOverflow(pParse);
#line 1158 ".\\thor\\sql\\new_parse.c"
   thorParserARG_STORE; /* Suppress warning about unused %extra_argument var */
}

/*
** Perform a shift action.
*/
static void yy_shift(
  yyParser *yypParser,          /* The parser to be shifted */
  int yyNewState,               /* The new state to shift in */
  int yyMajor,                  /* The major token to shift in */
  YYMINORTYPE *yypMinor         /* Pointer to the minor token to shift in */
){
  yyStackEntry *yytos;
  yypParser->yyidx++;
#ifdef YYTRACKMAXSTACKDEPTH
  if( yypParser->yyidx>yypParser->yyidxMax ){
    yypParser->yyidxMax = yypParser->yyidx;
  }
#endif
#if YYSTACKDEPTH>0 
  if( yypParser->yyidx>=YYSTACKDEPTH ){
    yyStackOverflow(yypParser, yypMinor);
    return;
  }
#else
  if( yypParser->yyidx>=yypParser->yystksz ){
    yyGrowStack(yypParser);
    if( yypParser->yyidx>=yypParser->yystksz ){
      yyStackOverflow(yypParser, yypMinor);
      return;
    }
  }
#endif
  yytos = &yypParser->yystack[yypParser->yyidx];
  yytos->stateno = (YYACTIONTYPE)yyNewState;
  yytos->major = (YYCODETYPE)yyMajor;
  yytos->minor = *yypMinor;
#ifndef NDEBUG
  if( yyTraceFILE && yypParser->yyidx>0 ){
    int i;
    fprintf(yyTraceFILE,"%sShift %d\n",yyTracePrompt,yyNewState);
    fprintf(yyTraceFILE,"%sStack:",yyTracePrompt);
    for(i=1; i<=yypParser->yyidx; i++)
      fprintf(yyTraceFILE," %s",yyTokenName[yypParser->yystack[i].major]);
    fprintf(yyTraceFILE,"\n");
  }
#endif
}

/* The following table contains information about every rule that
** is used during the reduce.
*/
static const struct {
  YYCODETYPE lhs;         /* Symbol on the left-hand side of the rule */
  unsigned char nrhs;     /* Number of right-hand side symbols in the rule */
} yyRuleInfo[] = {
  { 120, 1 },
  { 121, 2 },
  { 121, 1 },
  { 122, 1 },
  { 122, 3 },
  { 123, 0 },
  { 123, 1 },
  { 123, 3 },
  { 124, 1 },
  { 126, 1 },
  { 126, 1 },
  { 126, 1 },
  { 127, 0 },
  { 127, 2 },
  { 129, 0 },
  { 125, 1 },
  { 130, 2 },
  { 131, 1 },
  { 131, 3 },
  { 133, 1 },
  { 133, 2 },
  { 133, 1 },
  { 132, 9 },
  { 132, 1 },
  { 141, 4 },
  { 141, 5 },
  { 134, 1 },
  { 134, 1 },
  { 134, 0 },
  { 144, 2 },
  { 144, 0 },
  { 135, 3 },
  { 135, 2 },
  { 135, 4 },
  { 145, 2 },
  { 145, 1 },
  { 145, 0 },
  { 136, 0 },
  { 136, 2 },
  { 147, 2 },
  { 147, 0 },
  { 146, 7 },
  { 146, 7 },
  { 146, 7 },
  { 149, 0 },
  { 149, 2 },
  { 148, 1 },
  { 148, 2 },
  { 148, 3 },
  { 148, 4 },
  { 151, 2 },
  { 151, 0 },
  { 150, 0 },
  { 150, 3 },
  { 150, 2 },
  { 152, 0 },
  { 139, 0 },
  { 139, 3 },
  { 154, 4 },
  { 154, 2 },
  { 155, 1 },
  { 155, 1 },
  { 155, 0 },
  { 137, 0 },
  { 137, 3 },
  { 138, 0 },
  { 138, 2 },
  { 140, 0 },
  { 140, 2 },
  { 140, 4 },
  { 140, 4 },
  { 128, 1 },
  { 128, 3 },
  { 156, 1 },
  { 128, 1 },
  { 128, 1 },
  { 128, 3 },
  { 128, 5 },
  { 156, 1 },
  { 156, 1 },
  { 128, 1 },
  { 128, 3 },
  { 128, 5 },
  { 128, 4 },
  { 156, 1 },
  { 128, 3 },
  { 128, 3 },
  { 128, 3 },
  { 128, 3 },
  { 128, 3 },
  { 128, 3 },
  { 128, 3 },
  { 128, 3 },
  { 157, 1 },
  { 157, 2 },
  { 128, 3 },
  { 128, 5 },
  { 128, 2 },
  { 128, 3 },
  { 128, 3 },
  { 128, 4 },
  { 128, 2 },
  { 128, 2 },
  { 128, 2 },
  { 128, 2 },
  { 158, 1 },
  { 158, 2 },
  { 128, 5 },
  { 159, 1 },
  { 159, 2 },
  { 128, 5 },
  { 128, 3 },
  { 128, 5 },
  { 128, 4 },
  { 128, 4 },
  { 128, 5 },
  { 161, 5 },
  { 161, 4 },
  { 162, 2 },
  { 162, 0 },
  { 160, 1 },
  { 160, 0 },
  { 143, 1 },
  { 143, 0 },
  { 142, 3 },
  { 142, 1 },
};

static void yy_accept(yyParser*);  /* Forward Declaration */

/*
** Perform a reduce action and the shift that must immediately
** follow the reduce.
*/
static void yy_reduce(
  yyParser *yypParser,         /* The parser */
  int yyruleno                 /* Number of the rule by which to reduce */
){
  int yygoto;                     /* The next state */
  int yyact;                      /* The next action */
  YYMINORTYPE yygotominor;        /* The LHS of the rule reduced */
  yyStackEntry *yymsp;            /* The top of the parser's stack */
  int yysize;                     /* Amount to pop the stack */
  thorParserARG_FETCH;
  yymsp = &yypParser->yystack[yypParser->yyidx];
#ifndef NDEBUG
  if( yyTraceFILE && yyruleno>=0 
        && yyruleno<(int)(sizeof(yyRuleName)/sizeof(yyRuleName[0])) ){
    fprintf(yyTraceFILE, "%sReduce [%s].\n", yyTracePrompt,
      yyRuleName[yyruleno]);
  }
#endif /* NDEBUG */

  /* Silence complaints from purify about yygotominor being uninitialized
  ** in some cases when it is copied into the stack after the following
  ** switch.  yygotominor is uninitialized when a rule reduces that does
  ** not set the value of its left-hand side nonterminal.  Leaving the
  ** value of the nonterminal uninitialized is utterly harmless as long
  ** as the value is never used.  So really the only thing this code
  ** accomplishes is to quieten purify.  
  **
  ** 2007-01-16:  The wireshark project (www.wireshark.org) reports that
  ** without this code, their parser segfaults.  I'm not sure what there
  ** parser is doing to make this happen.  This is the second bug report
  ** from wireshark this week.  Clearly they are stressing Lemon in ways
  ** that it has not been previously stressed...  (SQLite ticket #2172)
  */
  /*memset(&yygotominor, 0, sizeof(yygotominor));*/
  yygotominor = yyzerominor;


  switch( yyruleno ){
  /* Beginning here are the reduction cases.  A typical example
  ** follows:
  **   case 0:
  **  #line <lineno> <grammarfile>
  **     { ... }           // User supplied code
  **  #line <lineno> <thisfile>
  **     break;
  */
      case 5: /* explain ::= */
#line 76 ".\\thor\\sql\\new_parse.y"
{ sqlite3BeginParse(pParse, 0); }
#line 1398 ".\\thor\\sql\\new_parse.c"
        break;
      case 6: /* explain ::= EXPLAIN */
#line 77 ".\\thor\\sql\\new_parse.y"
{ sqlite3BeginParse(pParse, 1); }
#line 1403 ".\\thor\\sql\\new_parse.c"
        break;
      case 7: /* explain ::= EXPLAIN QUERY PLAN */
#line 78 ".\\thor\\sql\\new_parse.y"
{ sqlite3BeginParse(pParse, 2); }
#line 1408 ".\\thor\\sql\\new_parse.c"
        break;
      case 8: /* cmdx ::= cmd */
#line 79 ".\\thor\\sql\\new_parse.y"
{ sqlite3FinishCoding(pParse); }
#line 1413 ".\\thor\\sql\\new_parse.c"
        break;
      case 9: /* nm ::= ID|INDEXED */
      case 10: /* nm ::= STRING */ yytestcase(yyruleno==10);
      case 11: /* nm ::= JOIN_KW */ yytestcase(yyruleno==11);
      case 34: /* as ::= AS nm */ yytestcase(yyruleno==34);
      case 35: /* as ::= ID|STRING */ yytestcase(yyruleno==35);
      case 45: /* dbnm ::= DOT nm */ yytestcase(yyruleno==45);
      case 53: /* indexed_opt ::= INDEXED BY nm */ yytestcase(yyruleno==53);
#line 137 ".\\thor\\sql\\new_parse.y"
{yygotominor.yy0 = yymsp[0].minor.yy0;}
#line 1424 ".\\thor\\sql\\new_parse.c"
        break;
      case 12: /* where_opt ::= */
      case 51: /* on_opt ::= */ yytestcase(yyruleno==51);
      case 65: /* having_opt ::= */ yytestcase(yyruleno==65);
      case 119: /* case_else ::= */ yytestcase(yyruleno==119);
      case 121: /* case_operand ::= */ yytestcase(yyruleno==121);
#line 144 ".\\thor\\sql\\new_parse.y"
{yygotominor.yy66 = 0;}
#line 1433 ".\\thor\\sql\\new_parse.c"
        break;
      case 13: /* where_opt ::= WHERE expr */
      case 50: /* on_opt ::= ON expr */ yytestcase(yyruleno==50);
      case 66: /* having_opt ::= HAVING expr */ yytestcase(yyruleno==66);
      case 118: /* case_else ::= ELSE expr */ yytestcase(yyruleno==118);
      case 120: /* case_operand ::= expr */ yytestcase(yyruleno==120);
#line 145 ".\\thor\\sql\\new_parse.y"
{yygotominor.yy66 = yymsp[0].minor.yy266.pExpr;}
#line 1442 ".\\thor\\sql\\new_parse.c"
        break;
      case 14: /* with ::= */
#line 149 ".\\thor\\sql\\new_parse.y"
{yygotominor.yy197 = 0;}
#line 1447 ".\\thor\\sql\\new_parse.c"
        break;
      case 15: /* cmd ::= select */
#line 154 ".\\thor\\sql\\new_parse.y"
{
  SelectDest dest = {SRT_Output, 0, 0, 0, 0, 0};
  sqlite3Select(pParse, yymsp[0].minor.yy273, &dest);
  sqlite3ExplainBegin(pParse->pVdbe);
  sqlite3ExplainSelect(pParse->pVdbe, yymsp[0].minor.yy273);
  sqlite3ExplainFinish(pParse->pVdbe);
  sqlite3SelectDelete(pParse->db, yymsp[0].minor.yy273);
}
#line 1459 ".\\thor\\sql\\new_parse.c"
        break;
      case 16: /* select ::= with selectnowith */
#line 170 ".\\thor\\sql\\new_parse.y"
{
  Select *p = yymsp[0].minor.yy273, *pNext, *pLoop;
  if( p ){
    int cnt = 0, mxSelect;
    p->pWith = yymsp[-1].minor.yy197;
    if( p->pPrior ){
      pNext = 0;
      for(pLoop=p; pLoop; pNext=pLoop, pLoop=pLoop->pPrior, cnt++){
        pLoop->pNext = pNext;
        pLoop->selFlags |= SF_Compound;
      }
      mxSelect = pParse->db->aLimit[SQLITE_LIMIT_COMPOUND_SELECT];
      if( mxSelect && cnt>mxSelect ){
        sqlite3ErrorMsg(pParse, "too many terms in compound SELECT");
      }
    }
  }else{
    sqlite3WithDelete(pParse->db, yymsp[-1].minor.yy197);
  }
  yygotominor.yy273 = p;
}
#line 1484 ".\\thor\\sql\\new_parse.c"
        break;
      case 17: /* selectnowith ::= oneselect */
      case 23: /* oneselect ::= values */ yytestcase(yyruleno==23);
#line 192 ".\\thor\\sql\\new_parse.y"
{yygotominor.yy273 = yymsp[0].minor.yy273;}
#line 1490 ".\\thor\\sql\\new_parse.c"
        break;
      case 18: /* selectnowith ::= selectnowith multiselect_op oneselect */
#line 194 ".\\thor\\sql\\new_parse.y"
{
  Select *pRhs = yymsp[0].minor.yy273;
  if( pRhs && pRhs->pPrior ){
    SrcList *pFrom;
    Token x;
    x.n = 0;
    pFrom = sqlite3SrcListAppendFromTerm(pParse,0,0,0,&x,pRhs,0,0);
    pRhs = sqlite3SelectNew(pParse,0,pFrom,0,0,0,0,0,0,0);
  }
  if( pRhs ){
    pRhs->op = (u8)yymsp[-1].minor.yy320;
    pRhs->pPrior = yymsp[-2].minor.yy273;
    if( yymsp[-1].minor.yy320!=TK_ALL ) pParse->hasCompound = 1;
  }else{
    sqlite3SelectDelete(pParse->db, yymsp[-2].minor.yy273);
  }
  yygotominor.yy273 = pRhs;
}
#line 1512 ".\\thor\\sql\\new_parse.c"
        break;
      case 19: /* multiselect_op ::= UNION */
      case 21: /* multiselect_op ::= EXCEPT|INTERSECT */ yytestcase(yyruleno==21);
#line 213 ".\\thor\\sql\\new_parse.y"
{yygotominor.yy320 = yymsp[0].major;}
#line 1518 ".\\thor\\sql\\new_parse.c"
        break;
      case 20: /* multiselect_op ::= UNION ALL */
#line 214 ".\\thor\\sql\\new_parse.y"
{yygotominor.yy320 = TK_ALL;}
#line 1523 ".\\thor\\sql\\new_parse.c"
        break;
      case 22: /* oneselect ::= SELECT distinct selcollist from where_opt groupby_opt having_opt orderby_opt limit_opt */
#line 218 ".\\thor\\sql\\new_parse.y"
{
  yygotominor.yy273 = sqlite3SelectNew(pParse,yymsp[-6].minor.yy70,yymsp[-5].minor.yy269,yymsp[-4].minor.yy66,yymsp[-3].minor.yy70,yymsp[-2].minor.yy66,yymsp[-1].minor.yy70,yymsp[-7].minor.yy89,yymsp[0].minor.yy267.pLimit,yymsp[0].minor.yy267.pOffset);
}
#line 1530 ".\\thor\\sql\\new_parse.c"
        break;
      case 24: /* values ::= VALUES LP nexprlist RP */
#line 225 ".\\thor\\sql\\new_parse.y"
{
  yygotominor.yy273 = sqlite3SelectNew(pParse,yymsp[-1].minor.yy70,0,0,0,0,0,SF_Values,0,0);
}
#line 1537 ".\\thor\\sql\\new_parse.c"
        break;
      case 25: /* values ::= values COMMA LP exprlist RP */
#line 228 ".\\thor\\sql\\new_parse.y"
{
  Select *pRight = sqlite3SelectNew(pParse,yymsp[-1].minor.yy70,0,0,0,0,0,SF_Values,0,0);
  if( pRight ){
    pRight->op = TK_ALL;
    pRight->pPrior = yymsp[-4].minor.yy273;
    yygotominor.yy273 = pRight;
  }else{
    yygotominor.yy273 = yymsp[-4].minor.yy273;
  }
}
#line 1551 ".\\thor\\sql\\new_parse.c"
        break;
      case 26: /* distinct ::= DISTINCT */
#line 243 ".\\thor\\sql\\new_parse.y"
{yygotominor.yy89 = SF_Distinct;}
#line 1556 ".\\thor\\sql\\new_parse.c"
        break;
      case 27: /* distinct ::= ALL */
      case 28: /* distinct ::= */ yytestcase(yyruleno==28);
#line 244 ".\\thor\\sql\\new_parse.y"
{yygotominor.yy89 = 0;}
#line 1562 ".\\thor\\sql\\new_parse.c"
        break;
      case 29: /* sclp ::= selcollist COMMA */
#line 256 ".\\thor\\sql\\new_parse.y"
{yygotominor.yy70 = yymsp[-1].minor.yy70;}
#line 1567 ".\\thor\\sql\\new_parse.c"
        break;
      case 30: /* sclp ::= */
      case 56: /* orderby_opt ::= */ yytestcase(yyruleno==56);
      case 63: /* groupby_opt ::= */ yytestcase(yyruleno==63);
      case 123: /* exprlist ::= */ yytestcase(yyruleno==123);
#line 257 ".\\thor\\sql\\new_parse.y"
{yygotominor.yy70 = 0;}
#line 1575 ".\\thor\\sql\\new_parse.c"
        break;
      case 31: /* selcollist ::= sclp expr as */
#line 258 ".\\thor\\sql\\new_parse.y"
{
   yygotominor.yy70 = sqlite3ExprListAppend(pParse, yymsp[-2].minor.yy70, yymsp[-1].minor.yy266.pExpr);
   if( yymsp[0].minor.yy0.n>0 ) sqlite3ExprListSetName(pParse, yygotominor.yy70, &yymsp[0].minor.yy0, 1);
   sqlite3ExprListSetSpan(pParse,yygotominor.yy70,&yymsp[-1].minor.yy266);
}
#line 1584 ".\\thor\\sql\\new_parse.c"
        break;
      case 32: /* selcollist ::= sclp STAR */
#line 263 ".\\thor\\sql\\new_parse.y"
{
  Expr *p = sqlite3Expr(pParse->db, TK_ALL, 0);
  yygotominor.yy70 = sqlite3ExprListAppend(pParse, yymsp[-1].minor.yy70, p);
}
#line 1592 ".\\thor\\sql\\new_parse.c"
        break;
      case 33: /* selcollist ::= sclp nm DOT STAR */
#line 267 ".\\thor\\sql\\new_parse.y"
{
  Expr *pRight = sqlite3PExpr(pParse, TK_ALL, 0, 0, &yymsp[0].minor.yy0);
  Expr *pLeft = sqlite3PExpr(pParse, TK_ID, 0, 0, &yymsp[-2].minor.yy0);
  Expr *pDot = sqlite3PExpr(pParse, TK_DOT, pLeft, pRight, 0);
  yygotominor.yy70 = sqlite3ExprListAppend(pParse,yymsp[-3].minor.yy70, pDot);
}
#line 1602 ".\\thor\\sql\\new_parse.c"
        break;
      case 36: /* as ::= */
#line 280 ".\\thor\\sql\\new_parse.y"
{yygotominor.yy0.n = 0;}
#line 1607 ".\\thor\\sql\\new_parse.c"
        break;
      case 37: /* from ::= */
#line 292 ".\\thor\\sql\\new_parse.y"
{yygotominor.yy269 = sqlite3DbMallocZero(pParse->db, sizeof(*yygotominor.yy269));}
#line 1612 ".\\thor\\sql\\new_parse.c"
        break;
      case 38: /* from ::= FROM seltablist */
#line 293 ".\\thor\\sql\\new_parse.y"
{
  yygotominor.yy269 = yymsp[0].minor.yy269;
  sqlite3SrcListShiftJoinType(yygotominor.yy269);
}
#line 1620 ".\\thor\\sql\\new_parse.c"
        break;
      case 39: /* stl_prefix ::= seltablist joinop */
#line 301 ".\\thor\\sql\\new_parse.y"
{
   yygotominor.yy269 = yymsp[-1].minor.yy269;
   if( ALWAYS(yygotominor.yy269 && yygotominor.yy269->nSrc>0) ) yygotominor.yy269->a[yygotominor.yy269->nSrc-1].jointype = (u8)yymsp[0].minor.yy320;
}
#line 1628 ".\\thor\\sql\\new_parse.c"
        break;
      case 40: /* stl_prefix ::= */
#line 305 ".\\thor\\sql\\new_parse.y"
{yygotominor.yy269 = 0;}
#line 1633 ".\\thor\\sql\\new_parse.c"
        break;
      case 41: /* seltablist ::= stl_prefix nm dbnm as indexed_opt on_opt using_opt */
#line 307 ".\\thor\\sql\\new_parse.y"
{
  yygotominor.yy269 = sqlite3SrcListAppendFromTerm(pParse,yymsp[-6].minor.yy269,&yymsp[-5].minor.yy0,&yymsp[-4].minor.yy0,&yymsp[-3].minor.yy0,0,yymsp[-1].minor.yy66,yymsp[0].minor.yy6);
  sqlite3SrcListIndexedBy(pParse, yygotominor.yy269, &yymsp[-2].minor.yy0);
}
#line 1641 ".\\thor\\sql\\new_parse.c"
        break;
      case 42: /* seltablist ::= stl_prefix LP select RP as on_opt using_opt */
#line 313 ".\\thor\\sql\\new_parse.y"
{
    yygotominor.yy269 = sqlite3SrcListAppendFromTerm(pParse,yymsp[-6].minor.yy269,0,0,&yymsp[-2].minor.yy0,yymsp[-4].minor.yy273,yymsp[-1].minor.yy66,yymsp[0].minor.yy6);
  }
#line 1648 ".\\thor\\sql\\new_parse.c"
        break;
      case 43: /* seltablist ::= stl_prefix LP seltablist RP as on_opt using_opt */
#line 317 ".\\thor\\sql\\new_parse.y"
{
    if( yymsp[-6].minor.yy269==0 && yymsp[-2].minor.yy0.n==0 && yymsp[-1].minor.yy66==0 && yymsp[0].minor.yy6==0 ){
      yygotominor.yy269 = yymsp[-4].minor.yy269;
    }else if( yymsp[-4].minor.yy269->nSrc==1 ){
      yygotominor.yy269 = sqlite3SrcListAppendFromTerm(pParse,yymsp[-6].minor.yy269,0,0,&yymsp[-2].minor.yy0,0,yymsp[-1].minor.yy66,yymsp[0].minor.yy6);
      if( yygotominor.yy269 ){
        struct SrcList_item *pNew = &yygotominor.yy269->a[yygotominor.yy269->nSrc-1];
        struct SrcList_item *pOld = yymsp[-4].minor.yy269->a;
        pNew->zName = pOld->zName;
        pNew->zDatabase = pOld->zDatabase;
        pNew->pSelect = pOld->pSelect;
        pOld->zName = pOld->zDatabase = 0;
        pOld->pSelect = 0;
      }
      sqlite3SrcListDelete(pParse->db, yymsp[-4].minor.yy269);
    }else{
      Select *pSubquery;
      sqlite3SrcListShiftJoinType(yymsp[-4].minor.yy269);
      pSubquery = sqlite3SelectNew(pParse,0,yymsp[-4].minor.yy269,0,0,0,0,SF_NestedFrom,0,0);
      yygotominor.yy269 = sqlite3SrcListAppendFromTerm(pParse,yymsp[-6].minor.yy269,0,0,&yymsp[-2].minor.yy0,pSubquery,yymsp[-1].minor.yy66,yymsp[0].minor.yy6);
    }
  }
#line 1674 ".\\thor\\sql\\new_parse.c"
        break;
      case 44: /* dbnm ::= */
      case 52: /* indexed_opt ::= */ yytestcase(yyruleno==52);
#line 342 ".\\thor\\sql\\new_parse.y"
{yygotominor.yy0.z=0; yygotominor.yy0.n=0;}
#line 1680 ".\\thor\\sql\\new_parse.c"
        break;
      case 46: /* joinop ::= COMMA|JOIN */
#line 349 ".\\thor\\sql\\new_parse.y"
{ yygotominor.yy320 = JT_INNER; }
#line 1685 ".\\thor\\sql\\new_parse.c"
        break;
      case 47: /* joinop ::= JOIN_KW JOIN */
#line 350 ".\\thor\\sql\\new_parse.y"
{ yygotominor.yy320 = sqlite3JoinType(pParse,&yymsp[-1].minor.yy0,0,0); }
#line 1690 ".\\thor\\sql\\new_parse.c"
        break;
      case 48: /* joinop ::= JOIN_KW nm JOIN */
#line 351 ".\\thor\\sql\\new_parse.y"
{ yygotominor.yy320 = sqlite3JoinType(pParse,&yymsp[-2].minor.yy0,&yymsp[-1].minor.yy0,0); }
#line 1695 ".\\thor\\sql\\new_parse.c"
        break;
      case 49: /* joinop ::= JOIN_KW nm nm JOIN */
#line 353 ".\\thor\\sql\\new_parse.y"
{ yygotominor.yy320 = sqlite3JoinType(pParse,&yymsp[-3].minor.yy0,&yymsp[-2].minor.yy0,&yymsp[-1].minor.yy0); }
#line 1700 ".\\thor\\sql\\new_parse.c"
        break;
      case 54: /* indexed_opt ::= NOT INDEXED */
#line 373 ".\\thor\\sql\\new_parse.y"
{yygotominor.yy0.z=0; yygotominor.yy0.n=1;}
#line 1705 ".\\thor\\sql\\new_parse.c"
        break;
      case 55: /* using_opt ::= */
#line 377 ".\\thor\\sql\\new_parse.y"
{yygotominor.yy6 = 0;}
#line 1710 ".\\thor\\sql\\new_parse.c"
        break;
      case 57: /* orderby_opt ::= ORDER BY sortlist */
      case 64: /* groupby_opt ::= GROUP BY nexprlist */ yytestcase(yyruleno==64);
      case 122: /* exprlist ::= nexprlist */ yytestcase(yyruleno==122);
#line 386 ".\\thor\\sql\\new_parse.y"
{yygotominor.yy70 = yymsp[0].minor.yy70;}
#line 1717 ".\\thor\\sql\\new_parse.c"
        break;
      case 58: /* sortlist ::= sortlist COMMA expr sortorder */
#line 387 ".\\thor\\sql\\new_parse.y"
{
  yygotominor.yy70 = sqlite3ExprListAppend(pParse,yymsp[-3].minor.yy70,yymsp[-1].minor.yy266.pExpr);
  if( yygotominor.yy70 ) yygotominor.yy70->a[yygotominor.yy70->nExpr-1].sortOrder = (u8)yymsp[0].minor.yy320;
}
#line 1725 ".\\thor\\sql\\new_parse.c"
        break;
      case 59: /* sortlist ::= expr sortorder */
#line 391 ".\\thor\\sql\\new_parse.y"
{
  yygotominor.yy70 = sqlite3ExprListAppend(pParse,0,yymsp[-1].minor.yy266.pExpr);
  if( yygotominor.yy70 && ALWAYS(yygotominor.yy70->a) ) yygotominor.yy70->a[0].sortOrder = (u8)yymsp[0].minor.yy320;
}
#line 1733 ".\\thor\\sql\\new_parse.c"
        break;
      case 60: /* sortorder ::= ASC */
      case 62: /* sortorder ::= */ yytestcase(yyruleno==62);
#line 398 ".\\thor\\sql\\new_parse.y"
{yygotominor.yy320 = SQLITE_SO_ASC;}
#line 1739 ".\\thor\\sql\\new_parse.c"
        break;
      case 61: /* sortorder ::= DESC */
#line 399 ".\\thor\\sql\\new_parse.y"
{yygotominor.yy320 = SQLITE_SO_DESC;}
#line 1744 ".\\thor\\sql\\new_parse.c"
        break;
      case 67: /* limit_opt ::= */
#line 425 ".\\thor\\sql\\new_parse.y"
{yygotominor.yy267.pLimit = 0; yygotominor.yy267.pOffset = 0;}
#line 1749 ".\\thor\\sql\\new_parse.c"
        break;
      case 68: /* limit_opt ::= LIMIT expr */
#line 426 ".\\thor\\sql\\new_parse.y"
{yygotominor.yy267.pLimit = yymsp[0].minor.yy266.pExpr; yygotominor.yy267.pOffset = 0;}
#line 1754 ".\\thor\\sql\\new_parse.c"
        break;
      case 69: /* limit_opt ::= LIMIT expr OFFSET expr */
#line 428 ".\\thor\\sql\\new_parse.y"
{yygotominor.yy267.pLimit = yymsp[-2].minor.yy266.pExpr; yygotominor.yy267.pOffset = yymsp[0].minor.yy266.pExpr;}
#line 1759 ".\\thor\\sql\\new_parse.c"
        break;
      case 70: /* limit_opt ::= LIMIT expr COMMA expr */
#line 430 ".\\thor\\sql\\new_parse.y"
{yygotominor.yy267.pOffset = yymsp[-2].minor.yy266.pExpr; yygotominor.yy267.pLimit = yymsp[0].minor.yy266.pExpr;}
#line 1764 ".\\thor\\sql\\new_parse.c"
        break;
      case 71: /* expr ::= term */
#line 463 ".\\thor\\sql\\new_parse.y"
{yygotominor.yy266 = yymsp[0].minor.yy266;}
#line 1769 ".\\thor\\sql\\new_parse.c"
        break;
      case 72: /* expr ::= LP expr RP */
#line 464 ".\\thor\\sql\\new_parse.y"
{yygotominor.yy266.pExpr = yymsp[-1].minor.yy266.pExpr; spanSet(&yygotominor.yy266,&yymsp[-2].minor.yy0,&yymsp[0].minor.yy0);}
#line 1774 ".\\thor\\sql\\new_parse.c"
        break;
      case 73: /* term ::= NULL */
      case 78: /* term ::= INTEGER|FLOAT|BLOB */ yytestcase(yyruleno==78);
      case 79: /* term ::= STRING */ yytestcase(yyruleno==79);
#line 465 ".\\thor\\sql\\new_parse.y"
{spanExpr(&yygotominor.yy266, pParse, yymsp[0].major, &yymsp[0].minor.yy0);}
#line 1781 ".\\thor\\sql\\new_parse.c"
        break;
      case 74: /* expr ::= ID|INDEXED */
      case 75: /* expr ::= JOIN_KW */ yytestcase(yyruleno==75);
#line 466 ".\\thor\\sql\\new_parse.y"
{spanExpr(&yygotominor.yy266, pParse, TK_ID, &yymsp[0].minor.yy0);}
#line 1787 ".\\thor\\sql\\new_parse.c"
        break;
      case 76: /* expr ::= nm DOT nm */
#line 468 ".\\thor\\sql\\new_parse.y"
{
  Expr *temp1 = sqlite3PExpr(pParse, TK_ID, 0, 0, &yymsp[-2].minor.yy0);
  Expr *temp2 = sqlite3PExpr(pParse, TK_ID, 0, 0, &yymsp[0].minor.yy0);
  yygotominor.yy266.pExpr = sqlite3PExpr(pParse, TK_DOT, temp1, temp2, 0);
  spanSet(&yygotominor.yy266,&yymsp[-2].minor.yy0,&yymsp[0].minor.yy0);
}
#line 1797 ".\\thor\\sql\\new_parse.c"
        break;
      case 77: /* expr ::= nm DOT nm DOT nm */
#line 474 ".\\thor\\sql\\new_parse.y"
{
  Expr *temp1 = sqlite3PExpr(pParse, TK_ID, 0, 0, &yymsp[-4].minor.yy0);
  Expr *temp2 = sqlite3PExpr(pParse, TK_ID, 0, 0, &yymsp[-2].minor.yy0);
  Expr *temp3 = sqlite3PExpr(pParse, TK_ID, 0, 0, &yymsp[0].minor.yy0);
  Expr *temp4 = sqlite3PExpr(pParse, TK_DOT, temp2, temp3, 0);
  yygotominor.yy266.pExpr = sqlite3PExpr(pParse, TK_DOT, temp1, temp4, 0);
  spanSet(&yygotominor.yy266,&yymsp[-4].minor.yy0,&yymsp[0].minor.yy0);
}
#line 1809 ".\\thor\\sql\\new_parse.c"
        break;
      case 80: /* expr ::= VARIABLE */
#line 484 ".\\thor\\sql\\new_parse.y"
{
  if( yymsp[0].minor.yy0.n>=2 && yymsp[0].minor.yy0.z[0]=='#' && sqlite3Isdigit(yymsp[0].minor.yy0.z[1]) ){
    /* When doing a nested parse, one can include terms in an expression
    ** that look like this:   #1 #2 ...  These terms refer to registers
    ** in the virtual machine.  #N is the N-th register. */
    if( pParse->nested==0 ){
      sqlite3ErrorMsg(pParse, "near \"%T\": syntax error", &yymsp[0].minor.yy0);
      yygotominor.yy266.pExpr = 0;
    }else{
      yygotominor.yy266.pExpr = sqlite3PExpr(pParse, TK_REGISTER, 0, 0, &yymsp[0].minor.yy0);
      if( yygotominor.yy266.pExpr ) sqlite3GetInt32(&yymsp[0].minor.yy0.z[1], &yygotominor.yy266.pExpr->iTable);
    }
  }else{
    spanExpr(&yygotominor.yy266, pParse, TK_VARIABLE, &yymsp[0].minor.yy0);
    sqlite3ExprAssignVarNumber(pParse, yygotominor.yy266.pExpr);
  }
  spanSet(&yygotominor.yy266, &yymsp[0].minor.yy0, &yymsp[0].minor.yy0);
}
#line 1831 ".\\thor\\sql\\new_parse.c"
        break;
      case 81: /* expr ::= expr COLLATE ID|STRING */
#line 502 ".\\thor\\sql\\new_parse.y"
{
  yygotominor.yy266.pExpr = sqlite3ExprAddCollateToken(pParse, yymsp[-2].minor.yy266.pExpr, &yymsp[0].minor.yy0);
  yygotominor.yy266.zStart = yymsp[-2].minor.yy266.zStart;
  yygotominor.yy266.zEnd = &yymsp[0].minor.yy0.z[yymsp[0].minor.yy0.n];
}
#line 1840 ".\\thor\\sql\\new_parse.c"
        break;
      case 82: /* expr ::= ID|INDEXED LP distinct exprlist RP */
#line 509 ".\\thor\\sql\\new_parse.y"
{
  if( yymsp[-1].minor.yy70 && yymsp[-1].minor.yy70->nExpr>pParse->db->aLimit[SQLITE_LIMIT_FUNCTION_ARG] ){
    sqlite3ErrorMsg(pParse, "too many arguments on function %T", &yymsp[-4].minor.yy0);
  }
  yygotominor.yy266.pExpr = sqlite3ExprFunction(pParse, yymsp[-1].minor.yy70, &yymsp[-4].minor.yy0);
  spanSet(&yygotominor.yy266,&yymsp[-4].minor.yy0,&yymsp[0].minor.yy0);
  if( yymsp[-2].minor.yy89 && yygotominor.yy266.pExpr ){
    yygotominor.yy266.pExpr->flags |= EP_Distinct;
  }
}
#line 1854 ".\\thor\\sql\\new_parse.c"
        break;
      case 83: /* expr ::= ID|INDEXED LP STAR RP */
#line 519 ".\\thor\\sql\\new_parse.y"
{
  yygotominor.yy266.pExpr = sqlite3ExprFunction(pParse, 0, &yymsp[-3].minor.yy0);
  spanSet(&yygotominor.yy266,&yymsp[-3].minor.yy0,&yymsp[0].minor.yy0);
}
#line 1862 ".\\thor\\sql\\new_parse.c"
        break;
      case 84: /* term ::= CTIME_KW */
#line 523 ".\\thor\\sql\\new_parse.y"
{
  yygotominor.yy266.pExpr = sqlite3ExprFunction(pParse, 0, &yymsp[0].minor.yy0);
  spanSet(&yygotominor.yy266, &yymsp[0].minor.yy0, &yymsp[0].minor.yy0);
}
#line 1870 ".\\thor\\sql\\new_parse.c"
        break;
      case 85: /* expr ::= expr AND expr */
      case 86: /* expr ::= expr OR expr */ yytestcase(yyruleno==86);
      case 87: /* expr ::= expr LT|GT|GE|LE expr */ yytestcase(yyruleno==87);
      case 88: /* expr ::= expr EQ|NE expr */ yytestcase(yyruleno==88);
      case 89: /* expr ::= expr BITAND|BITOR|LSHIFT|RSHIFT expr */ yytestcase(yyruleno==89);
      case 90: /* expr ::= expr PLUS|MINUS expr */ yytestcase(yyruleno==90);
      case 91: /* expr ::= expr STAR|SLASH|REM expr */ yytestcase(yyruleno==91);
      case 92: /* expr ::= expr CONCAT expr */ yytestcase(yyruleno==92);
#line 545 ".\\thor\\sql\\new_parse.y"
{spanBinaryExpr(&yygotominor.yy266,pParse,yymsp[-1].major,&yymsp[-2].minor.yy266,&yymsp[0].minor.yy266);}
#line 1882 ".\\thor\\sql\\new_parse.c"
        break;
      case 93: /* likeop ::= LIKE_KW|MATCH */
#line 558 ".\\thor\\sql\\new_parse.y"
{yygotominor.yy260.eOperator = yymsp[0].minor.yy0; yygotominor.yy260.bNot = 0;}
#line 1887 ".\\thor\\sql\\new_parse.c"
        break;
      case 94: /* likeop ::= NOT LIKE_KW|MATCH */
#line 559 ".\\thor\\sql\\new_parse.y"
{yygotominor.yy260.eOperator = yymsp[0].minor.yy0; yygotominor.yy260.bNot = 1;}
#line 1892 ".\\thor\\sql\\new_parse.c"
        break;
      case 95: /* expr ::= expr likeop expr */
#line 560 ".\\thor\\sql\\new_parse.y"
{
  ExprList *pList;
  pList = sqlite3ExprListAppend(pParse,0, yymsp[0].minor.yy266.pExpr);
  pList = sqlite3ExprListAppend(pParse,pList, yymsp[-2].minor.yy266.pExpr);
  yygotominor.yy266.pExpr = sqlite3ExprFunction(pParse, pList, &yymsp[-1].minor.yy260.eOperator);
  if( yymsp[-1].minor.yy260.bNot ) yygotominor.yy266.pExpr = sqlite3PExpr(pParse, TK_NOT, yygotominor.yy266.pExpr, 0, 0);
  yygotominor.yy266.zStart = yymsp[-2].minor.yy266.zStart;
  yygotominor.yy266.zEnd = yymsp[0].minor.yy266.zEnd;
  if( yygotominor.yy266.pExpr ) yygotominor.yy266.pExpr->flags |= EP_InfixFunc;
}
#line 1906 ".\\thor\\sql\\new_parse.c"
        break;
      case 96: /* expr ::= expr likeop expr ESCAPE expr */
#line 570 ".\\thor\\sql\\new_parse.y"
{
  ExprList *pList;
  pList = sqlite3ExprListAppend(pParse,0, yymsp[-2].minor.yy266.pExpr);
  pList = sqlite3ExprListAppend(pParse,pList, yymsp[-4].minor.yy266.pExpr);
  pList = sqlite3ExprListAppend(pParse,pList, yymsp[0].minor.yy266.pExpr);
  yygotominor.yy266.pExpr = sqlite3ExprFunction(pParse, pList, &yymsp[-3].minor.yy260.eOperator);
  if( yymsp[-3].minor.yy260.bNot ) yygotominor.yy266.pExpr = sqlite3PExpr(pParse, TK_NOT, yygotominor.yy266.pExpr, 0, 0);
  yygotominor.yy266.zStart = yymsp[-4].minor.yy266.zStart;
  yygotominor.yy266.zEnd = yymsp[0].minor.yy266.zEnd;
  if( yygotominor.yy266.pExpr ) yygotominor.yy266.pExpr->flags |= EP_InfixFunc;
}
#line 1921 ".\\thor\\sql\\new_parse.c"
        break;
      case 97: /* expr ::= expr ISNULL|NOTNULL */
#line 598 ".\\thor\\sql\\new_parse.y"
{spanUnaryPostfix(&yygotominor.yy266,pParse,yymsp[0].major,&yymsp[-1].minor.yy266,&yymsp[0].minor.yy0);}
#line 1926 ".\\thor\\sql\\new_parse.c"
        break;
      case 98: /* expr ::= expr NOT NULL */
#line 599 ".\\thor\\sql\\new_parse.y"
{spanUnaryPostfix(&yygotominor.yy266,pParse,TK_NOTNULL,&yymsp[-2].minor.yy266,&yymsp[0].minor.yy0);}
#line 1931 ".\\thor\\sql\\new_parse.c"
        break;
      case 99: /* expr ::= expr IS expr */
#line 620 ".\\thor\\sql\\new_parse.y"
{
  spanBinaryExpr(&yygotominor.yy266,pParse,TK_IS,&yymsp[-2].minor.yy266,&yymsp[0].minor.yy266);
  binaryToUnaryIfNull(pParse, yymsp[0].minor.yy266.pExpr, yygotominor.yy266.pExpr, TK_ISNULL);
}
#line 1939 ".\\thor\\sql\\new_parse.c"
        break;
      case 100: /* expr ::= expr IS NOT expr */
#line 624 ".\\thor\\sql\\new_parse.y"
{
  spanBinaryExpr(&yygotominor.yy266,pParse,TK_ISNOT,&yymsp[-3].minor.yy266,&yymsp[0].minor.yy266);
  binaryToUnaryIfNull(pParse, yymsp[0].minor.yy266.pExpr, yygotominor.yy266.pExpr, TK_NOTNULL);
}
#line 1947 ".\\thor\\sql\\new_parse.c"
        break;
      case 101: /* expr ::= NOT expr */
      case 102: /* expr ::= BITNOT expr */ yytestcase(yyruleno==102);
#line 647 ".\\thor\\sql\\new_parse.y"
{spanUnaryPrefix(&yygotominor.yy266,pParse,yymsp[-1].major,&yymsp[0].minor.yy266,&yymsp[-1].minor.yy0);}
#line 1953 ".\\thor\\sql\\new_parse.c"
        break;
      case 103: /* expr ::= MINUS expr */
#line 650 ".\\thor\\sql\\new_parse.y"
{spanUnaryPrefix(&yygotominor.yy266,pParse,TK_UMINUS,&yymsp[0].minor.yy266,&yymsp[-1].minor.yy0);}
#line 1958 ".\\thor\\sql\\new_parse.c"
        break;
      case 104: /* expr ::= PLUS expr */
#line 652 ".\\thor\\sql\\new_parse.y"
{spanUnaryPrefix(&yygotominor.yy266,pParse,TK_UPLUS,&yymsp[0].minor.yy266,&yymsp[-1].minor.yy0);}
#line 1963 ".\\thor\\sql\\new_parse.c"
        break;
      case 105: /* between_op ::= BETWEEN */
      case 108: /* in_op ::= IN */ yytestcase(yyruleno==108);
#line 655 ".\\thor\\sql\\new_parse.y"
{yygotominor.yy320 = 0;}
#line 1969 ".\\thor\\sql\\new_parse.c"
        break;
      case 106: /* between_op ::= NOT BETWEEN */
      case 109: /* in_op ::= NOT IN */ yytestcase(yyruleno==109);
#line 656 ".\\thor\\sql\\new_parse.y"
{yygotominor.yy320 = 1;}
#line 1975 ".\\thor\\sql\\new_parse.c"
        break;
      case 107: /* expr ::= expr between_op expr AND expr */
#line 657 ".\\thor\\sql\\new_parse.y"
{
  ExprList *pList = sqlite3ExprListAppend(pParse,0, yymsp[-2].minor.yy266.pExpr);
  pList = sqlite3ExprListAppend(pParse,pList, yymsp[0].minor.yy266.pExpr);
  yygotominor.yy266.pExpr = sqlite3PExpr(pParse, TK_BETWEEN, yymsp[-4].minor.yy266.pExpr, 0, 0);
  if( yygotominor.yy266.pExpr ){
    yygotominor.yy266.pExpr->x.pList = pList;
  }else{
    sqlite3ExprListDelete(pParse->db, pList);
  } 
  if( yymsp[-3].minor.yy320 ) yygotominor.yy266.pExpr = sqlite3PExpr(pParse, TK_NOT, yygotominor.yy266.pExpr, 0, 0);
  yygotominor.yy266.zStart = yymsp[-4].minor.yy266.zStart;
  yygotominor.yy266.zEnd = yymsp[0].minor.yy266.zEnd;
}
#line 1992 ".\\thor\\sql\\new_parse.c"
        break;
      case 110: /* expr ::= expr in_op LP exprlist RP */
#line 674 ".\\thor\\sql\\new_parse.y"
{
    if( yymsp[-1].minor.yy70==0 ){
      /* Expressions of the form
      **
      **      expr1 IN ()
      **      expr1 NOT IN ()
      **
      ** simplify to constants 0 (false) and 1 (true), respectively,
      ** regardless of the value of expr1.
      */
      yygotominor.yy266.pExpr = sqlite3PExpr(pParse, TK_INTEGER, 0, 0, &sqlite3IntTokens[yymsp[-3].minor.yy320]);
      sqlite3ExprDelete(pParse->db, yymsp[-4].minor.yy266.pExpr);
    }else if( yymsp[-1].minor.yy70->nExpr==1 ){
      /* Expressions of the form:
      **
      **      expr1 IN (?1)
      **      expr1 NOT IN (?2)
      **
      ** with exactly one value on the RHS can be simplified to something
      ** like this:
      **
      **      expr1 == ?1
      **      expr1 <> ?2
      **
      ** But, the RHS of the == or <> is marked with the EP_Generic flag
      ** so that it may not contribute to the computation of comparison
      ** affinity or the collating sequence to use for comparison.  Otherwise,
      ** the semantics would be subtly different from IN or NOT IN.
      */
      Expr *pRHS = yymsp[-1].minor.yy70->a[0].pExpr;
      yymsp[-1].minor.yy70->a[0].pExpr = 0;
      sqlite3ExprListDelete(pParse->db, yymsp[-1].minor.yy70);
      /* pRHS cannot be NULL because a malloc error would have been detected
      ** before now and control would have never reached this point */
      if( ALWAYS(pRHS) ){
        pRHS->flags &= ~EP_Collate;
        pRHS->flags |= EP_Generic;
      }
      yygotominor.yy266.pExpr = sqlite3PExpr(pParse, yymsp[-3].minor.yy320 ? TK_NE : TK_EQ, yymsp[-4].minor.yy266.pExpr, pRHS, 0);
    }else{
      yygotominor.yy266.pExpr = sqlite3PExpr(pParse, TK_IN, yymsp[-4].minor.yy266.pExpr, 0, 0);
      if( yygotominor.yy266.pExpr ){
        yygotominor.yy266.pExpr->x.pList = yymsp[-1].minor.yy70;
        sqlite3ExprSetHeight(pParse, yygotominor.yy266.pExpr);
      }else{
        sqlite3ExprListDelete(pParse->db, yymsp[-1].minor.yy70);
      }
      if( yymsp[-3].minor.yy320 ) yygotominor.yy266.pExpr = sqlite3PExpr(pParse, TK_NOT, yygotominor.yy266.pExpr, 0, 0);
    }
    yygotominor.yy266.zStart = yymsp[-4].minor.yy266.zStart;
    yygotominor.yy266.zEnd = &yymsp[0].minor.yy0.z[yymsp[0].minor.yy0.n];
  }
#line 2048 ".\\thor\\sql\\new_parse.c"
        break;
      case 111: /* expr ::= LP select RP */
#line 726 ".\\thor\\sql\\new_parse.y"
{
    yygotominor.yy266.pExpr = sqlite3PExpr(pParse, TK_SELECT, 0, 0, 0);
    if( yygotominor.yy266.pExpr ){
      yygotominor.yy266.pExpr->x.pSelect = yymsp[-1].minor.yy273;
      ExprSetProperty(yygotominor.yy266.pExpr, EP_xIsSelect);
      sqlite3ExprSetHeight(pParse, yygotominor.yy266.pExpr);
    }else{
      sqlite3SelectDelete(pParse->db, yymsp[-1].minor.yy273);
    }
    yygotominor.yy266.zStart = yymsp[-2].minor.yy0.z;
    yygotominor.yy266.zEnd = &yymsp[0].minor.yy0.z[yymsp[0].minor.yy0.n];
  }
#line 2064 ".\\thor\\sql\\new_parse.c"
        break;
      case 112: /* expr ::= expr in_op LP select RP */
#line 738 ".\\thor\\sql\\new_parse.y"
{
    yygotominor.yy266.pExpr = sqlite3PExpr(pParse, TK_IN, yymsp[-4].minor.yy266.pExpr, 0, 0);
    if( yygotominor.yy266.pExpr ){
      yygotominor.yy266.pExpr->x.pSelect = yymsp[-1].minor.yy273;
      ExprSetProperty(yygotominor.yy266.pExpr, EP_xIsSelect);
      sqlite3ExprSetHeight(pParse, yygotominor.yy266.pExpr);
    }else{
      sqlite3SelectDelete(pParse->db, yymsp[-1].minor.yy273);
    }
    if( yymsp[-3].minor.yy320 ) yygotominor.yy266.pExpr = sqlite3PExpr(pParse, TK_NOT, yygotominor.yy266.pExpr, 0, 0);
    yygotominor.yy266.zStart = yymsp[-4].minor.yy266.zStart;
    yygotominor.yy266.zEnd = &yymsp[0].minor.yy0.z[yymsp[0].minor.yy0.n];
  }
#line 2081 ".\\thor\\sql\\new_parse.c"
        break;
      case 113: /* expr ::= expr in_op nm dbnm */
#line 751 ".\\thor\\sql\\new_parse.y"
{
    SrcList *pSrc = sqlite3SrcListAppend(pParse->db, 0,&yymsp[-1].minor.yy0,&yymsp[0].minor.yy0);
    yygotominor.yy266.pExpr = sqlite3PExpr(pParse, TK_IN, yymsp[-3].minor.yy266.pExpr, 0, 0);
    if( yygotominor.yy266.pExpr ){
      yygotominor.yy266.pExpr->x.pSelect = sqlite3SelectNew(pParse, 0,pSrc,0,0,0,0,0,0,0);
      ExprSetProperty(yygotominor.yy266.pExpr, EP_xIsSelect);
      sqlite3ExprSetHeight(pParse, yygotominor.yy266.pExpr);
    }else{
      sqlite3SrcListDelete(pParse->db, pSrc);
    }
    if( yymsp[-2].minor.yy320 ) yygotominor.yy266.pExpr = sqlite3PExpr(pParse, TK_NOT, yygotominor.yy266.pExpr, 0, 0);
    yygotominor.yy266.zStart = yymsp[-3].minor.yy266.zStart;
    yygotominor.yy266.zEnd = yymsp[0].minor.yy0.z ? &yymsp[0].minor.yy0.z[yymsp[0].minor.yy0.n] : &yymsp[-1].minor.yy0.z[yymsp[-1].minor.yy0.n];
  }
#line 2099 ".\\thor\\sql\\new_parse.c"
        break;
      case 114: /* expr ::= EXISTS LP select RP */
#line 765 ".\\thor\\sql\\new_parse.y"
{
    Expr *p = yygotominor.yy266.pExpr = sqlite3PExpr(pParse, TK_EXISTS, 0, 0, 0);
    if( p ){
      p->x.pSelect = yymsp[-1].minor.yy273;
      ExprSetProperty(p, EP_xIsSelect);
      sqlite3ExprSetHeight(pParse, p);
    }else{
      sqlite3SelectDelete(pParse->db, yymsp[-1].minor.yy273);
    }
    yygotominor.yy266.zStart = yymsp[-3].minor.yy0.z;
    yygotominor.yy266.zEnd = &yymsp[0].minor.yy0.z[yymsp[0].minor.yy0.n];
  }
#line 2115 ".\\thor\\sql\\new_parse.c"
        break;
      case 115: /* expr ::= CASE case_operand case_exprlist case_else END */
#line 780 ".\\thor\\sql\\new_parse.y"
{
  yygotominor.yy266.pExpr = sqlite3PExpr(pParse, TK_CASE, yymsp[-3].minor.yy66, 0, 0);
  if( yygotominor.yy266.pExpr ){
    yygotominor.yy266.pExpr->x.pList = yymsp[-1].minor.yy66 ? sqlite3ExprListAppend(pParse,yymsp[-2].minor.yy70,yymsp[-1].minor.yy66) : yymsp[-2].minor.yy70;
    sqlite3ExprSetHeight(pParse, yygotominor.yy266.pExpr);
  }else{
    sqlite3ExprListDelete(pParse->db, yymsp[-2].minor.yy70);
    sqlite3ExprDelete(pParse->db, yymsp[-1].minor.yy66);
  }
  yygotominor.yy266.zStart = yymsp[-4].minor.yy0.z;
  yygotominor.yy266.zEnd = &yymsp[0].minor.yy0.z[yymsp[0].minor.yy0.n];
}
#line 2131 ".\\thor\\sql\\new_parse.c"
        break;
      case 116: /* case_exprlist ::= case_exprlist WHEN expr THEN expr */
#line 794 ".\\thor\\sql\\new_parse.y"
{
  yygotominor.yy70 = sqlite3ExprListAppend(pParse,yymsp[-4].minor.yy70, yymsp[-2].minor.yy266.pExpr);
  yygotominor.yy70 = sqlite3ExprListAppend(pParse,yygotominor.yy70, yymsp[0].minor.yy266.pExpr);
}
#line 2139 ".\\thor\\sql\\new_parse.c"
        break;
      case 117: /* case_exprlist ::= WHEN expr THEN expr */
#line 798 ".\\thor\\sql\\new_parse.y"
{
  yygotominor.yy70 = sqlite3ExprListAppend(pParse,0, yymsp[-2].minor.yy266.pExpr);
  yygotominor.yy70 = sqlite3ExprListAppend(pParse,yygotominor.yy70, yymsp[0].minor.yy266.pExpr);
}
#line 2147 ".\\thor\\sql\\new_parse.c"
        break;
      case 124: /* nexprlist ::= nexprlist COMMA expr */
#line 819 ".\\thor\\sql\\new_parse.y"
{yygotominor.yy70 = sqlite3ExprListAppend(pParse,yymsp[-2].minor.yy70,yymsp[0].minor.yy266.pExpr);}
#line 2152 ".\\thor\\sql\\new_parse.c"
        break;
      case 125: /* nexprlist ::= expr */
#line 821 ".\\thor\\sql\\new_parse.y"
{yygotominor.yy70 = sqlite3ExprListAppend(pParse,0,yymsp[0].minor.yy266.pExpr);}
#line 2157 ".\\thor\\sql\\new_parse.c"
        break;
      default:
      /* (0) input ::= cmdlist */ yytestcase(yyruleno==0);
      /* (1) cmdlist ::= cmdlist ecmd */ yytestcase(yyruleno==1);
      /* (2) cmdlist ::= ecmd */ yytestcase(yyruleno==2);
      /* (3) ecmd ::= SEMI */ yytestcase(yyruleno==3);
      /* (4) ecmd ::= explain cmdx SEMI */ yytestcase(yyruleno==4);
        break;
  };
  yygoto = yyRuleInfo[yyruleno].lhs;
  yysize = yyRuleInfo[yyruleno].nrhs;
  yypParser->yyidx -= yysize;
  yyact = yy_find_reduce_action(yymsp[-yysize].stateno,(YYCODETYPE)yygoto);
  if( yyact < YYNSTATE ){
#ifdef NDEBUG
    /* If we are not debugging and the reduce action popped at least
    ** one element off the stack, then we can push the new element back
    ** onto the stack here, and skip the stack overflow test in yy_shift().
    ** That gives a significant speed improvement. */
    if( yysize ){
      yypParser->yyidx++;
      yymsp -= yysize-1;
      yymsp->stateno = (YYACTIONTYPE)yyact;
      yymsp->major = (YYCODETYPE)yygoto;
      yymsp->minor = yygotominor;
    }else
#endif
    {
      yy_shift(yypParser,yyact,yygoto,&yygotominor);
    }
  }else{
    assert( yyact == YYNSTATE + YYNRULE + 1 );
    yy_accept(yypParser);
  }
}

/*
** The following code executes when the parse fails
*/
#ifndef YYNOERRORRECOVERY
static void yy_parse_failed(
  yyParser *yypParser           /* The parser */
){
  thorParserARG_FETCH;
#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sFail!\n",yyTracePrompt);
  }
#endif
  while( yypParser->yyidx>=0 ) yy_pop_parser_stack(yypParser);
  /* Here code is inserted which will be executed whenever the
  ** parser fails */
  thorParserARG_STORE; /* Suppress warning about unused %extra_argument variable */
}
#endif /* YYNOERRORRECOVERY */

/*
** The following code executes when a syntax error first occurs.
*/
static void yy_syntax_error(
  yyParser *yypParser,           /* The parser */
  int yymajor,                   /* The major type of the error token */
  YYMINORTYPE yyminor            /* The minor type of the error token */
){
  thorParserARG_FETCH;
#define TOKEN (yyminor.yy0)
#line 18 ".\\thor\\sql\\new_parse.y"

    UNUSED_PARAMETER(yymajor);  /* Silence some compiler warnings */
    assert( TOKEN.z[0] );  /* The tokenizer always gives us a token */
    thorSyntaxError(pParse, &TOKEN);
#line 2229 ".\\thor\\sql\\new_parse.c"
  thorParserARG_STORE; /* Suppress warning about unused %extra_argument variable */
}

/*
** The following is executed when the parser accepts
*/
static void yy_accept(
  yyParser *yypParser           /* The parser */
){
  thorParserARG_FETCH;
#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sAccept!\n",yyTracePrompt);
  }
#endif
  while( yypParser->yyidx>=0 ) yy_pop_parser_stack(yypParser);
  /* Here code is inserted which will be executed whenever the
  ** parser accepts */
  thorParserARG_STORE; /* Suppress warning about unused %extra_argument variable */
}

/* The main parser program.
** The first argument is a pointer to a structure obtained from
** "thorParserAlloc" which describes the current state of the parser.
** The second argument is the major token number.  The third is
** the minor token.  The fourth optional argument is whatever the
** user wants (and specified in the grammar) and is available for
** use by the action routines.
**
** Inputs:
** <ul>
** <li> A pointer to the parser (an opaque structure.)
** <li> The major token number.
** <li> The minor token number.
** <li> An option argument of a grammar-specified type.
** </ul>
**
** Outputs:
** None.
*/
void thorParser(
  void *yyp,                   /* The parser */
  int yymajor,                 /* The major token code number */
  thorParserTOKENTYPE yyminor       /* The value for the token */
  thorParserARG_PDECL               /* Optional %extra_argument parameter */
){
  YYMINORTYPE yyminorunion;
  int yyact;            /* The parser action. */
  int yyendofinput;     /* True if we are at the end of input */
#ifdef YYERRORSYMBOL
  int yyerrorhit = 0;   /* True if yymajor has invoked an error */
#endif
  yyParser *yypParser;  /* The parser */

  /* (re)initialize the parser, if necessary */
  yypParser = (yyParser*)yyp;
  if( yypParser->yyidx<0 ){
#if YYSTACKDEPTH<=0
    if( yypParser->yystksz <=0 ){
      /*memset(&yyminorunion, 0, sizeof(yyminorunion));*/
      yyminorunion = yyzerominor;
      yyStackOverflow(yypParser, &yyminorunion);
      return;
    }
#endif
    yypParser->yyidx = 0;
    yypParser->yyerrcnt = -1;
    yypParser->yystack[0].stateno = 0;
    yypParser->yystack[0].major = 0;
  }
  yyminorunion.yy0 = yyminor;
  yyendofinput = (yymajor==0);
  thorParserARG_STORE;

#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sInput %s\n",yyTracePrompt,yyTokenName[yymajor]);
  }
#endif

  do{
    yyact = yy_find_shift_action(yypParser,(YYCODETYPE)yymajor);
    if( yyact<YYNSTATE ){
      assert( !yyendofinput );  /* Impossible to shift the $ token */
      yy_shift(yypParser,yyact,yymajor,&yyminorunion);
      yypParser->yyerrcnt--;
      yymajor = YYNOCODE;
    }else if( yyact < YYNSTATE + YYNRULE ){
      yy_reduce(yypParser,yyact-YYNSTATE);
    }else{
      assert( yyact == YY_ERROR_ACTION );
#ifdef YYERRORSYMBOL
      int yymx;
#endif
#ifndef NDEBUG
      if( yyTraceFILE ){
        fprintf(yyTraceFILE,"%sSyntax Error!\n",yyTracePrompt);
      }
#endif
#ifdef YYERRORSYMBOL
      /* A syntax error has occurred.
      ** The response to an error depends upon whether or not the
      ** grammar defines an error token "ERROR".  
      **
      ** This is what we do if the grammar does define ERROR:
      **
      **  * Call the %syntax_error function.
      **
      **  * Begin popping the stack until we enter a state where
      **    it is legal to shift the error symbol, then shift
      **    the error symbol.
      **
      **  * Set the error count to three.
      **
      **  * Begin accepting and shifting new tokens.  No new error
      **    processing will occur until three tokens have been
      **    shifted successfully.
      **
      */
      if( yypParser->yyerrcnt<0 ){
        yy_syntax_error(yypParser,yymajor,yyminorunion);
      }
      yymx = yypParser->yystack[yypParser->yyidx].major;
      if( yymx==YYERRORSYMBOL || yyerrorhit ){
#ifndef NDEBUG
        if( yyTraceFILE ){
          fprintf(yyTraceFILE,"%sDiscard input token %s\n",
             yyTracePrompt,yyTokenName[yymajor]);
        }
#endif
        yy_destructor(yypParser, (YYCODETYPE)yymajor,&yyminorunion);
        yymajor = YYNOCODE;
      }else{
         while(
          yypParser->yyidx >= 0 &&
          yymx != YYERRORSYMBOL &&
          (yyact = yy_find_reduce_action(
                        yypParser->yystack[yypParser->yyidx].stateno,
                        YYERRORSYMBOL)) >= YYNSTATE
        ){
          yy_pop_parser_stack(yypParser);
        }
        if( yypParser->yyidx < 0 || yymajor==0 ){
          yy_destructor(yypParser,(YYCODETYPE)yymajor,&yyminorunion);
          yy_parse_failed(yypParser);
          yymajor = YYNOCODE;
        }else if( yymx!=YYERRORSYMBOL ){
          YYMINORTYPE u2;
          u2.YYERRSYMDT = 0;
          yy_shift(yypParser,yyact,YYERRORSYMBOL,&u2);
        }
      }
      yypParser->yyerrcnt = 3;
      yyerrorhit = 1;
#elif defined(YYNOERRORRECOVERY)
      /* If the YYNOERRORRECOVERY macro is defined, then do not attempt to
      ** do any kind of error recovery.  Instead, simply invoke the syntax
      ** error routine and continue going as if nothing had happened.
      **
      ** Applications can set this macro (for example inside %include) if
      ** they intend to abandon the parse upon the first syntax error seen.
      */
      yy_syntax_error(yypParser,yymajor,yyminorunion);
      yy_destructor(yypParser,(YYCODETYPE)yymajor,&yyminorunion);
      yymajor = YYNOCODE;
      
#else  /* YYERRORSYMBOL is not defined */
      /* This is what we do if the grammar does not define ERROR:
      **
      **  * Report an error message, and throw away the input token.
      **
      **  * If the input token is $, then fail the parse.
      **
      ** As before, subsequent error messages are suppressed until
      ** three input tokens have been successfully shifted.
      */
      if( yypParser->yyerrcnt<=0 ){
        yy_syntax_error(yypParser,yymajor,yyminorunion);
      }
      yypParser->yyerrcnt = 3;
      yy_destructor(yypParser,(YYCODETYPE)yymajor,&yyminorunion);
      if( yyendofinput ){
        yy_parse_failed(yypParser);
      }
      yymajor = YYNOCODE;
#endif
    }
  }while( yymajor!=YYNOCODE && yypParser->yyidx>=0 );
  return;
}
