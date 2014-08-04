// parse_ctx.h by scott.zgeng@gmail.com 2014.07.11

 
#ifndef  __PARSE_CTX__
#define  __PARSE_CTX__

extern "C" {
#include "../sql/sqliteInt.h"
#include "../sql/vdbeInt.h"
}



typedef char int8;
typedef unsigned uint8;
typedef short int16;
typedef unsigned short uint16;
typedef int int32;
typedef unsigned int uint32;
typedef long long int64;
typedef unsigned long long uint64;

typedef unsigned long long rowid_t;

// 为了防止和内置类型混用
struct result_t
{
    uint32 code;
    bool operator == (result_t other) { return code == other.code; }
    bool operator != (result_t other) { return code != other.code; }
};


const static result_t RT_SUCCEEDED = { 0 };
const static result_t RT_FAILED = { -1 };


#define IF_RETURN(code, condition) \
    do { if (condition) { printf("return from %s:%d\n", __FILE__, __LINE__); return (code); } } while (0)

#define IF_RETURN_FAILED(condition) IF_RETURN(RT_FAILED, condition)

#define MAX_JOIN_TABLE 8



class query_pack_t
{
public:
    static const int SEGMENT_SIZE = 1024;
public:
    query_pack_t();
    ~query_pack_t();

public:
    //uint8 alloc_buffer();

    result_t generate_data(int32 table_id, int32 column_id);

private:

    bool usedd_map[4];
    uint8 nullmap[SEGMENT_SIZE * 4];
    uint8 buffer[SEGMENT_SIZE * 8 * 4];
};


class node_base_t
{
public:
    virtual result_t init(Parse* parse, Select* select) = 0;
    virtual void uninit() = 0;
    virtual result_t next(query_pack_t* pack) = 0;
    virtual const char* name() = 0;

};


class expr_base_t;
class node_generator_t
{
public:
    node_generator_t(Parse* parse, Select* select);
    ~node_generator_t();

public:
    result_t build(node_base_t** root);
    result_t build_expression(Expr* expr, expr_base_t** root);

private:        
    result_t build_join(node_base_t** scan_nodes, int32 tab_num, node_base_t** root);
    
    expr_base_t* create_expression(Expr* expr);

private:
    Parse* m_parse;
    Select* m_select;
};


class scan_node_t: public node_base_t
{
public:
    scan_node_t(int index);
    virtual ~scan_node_t();

public:
    virtual result_t init(Parse* parse, Select* select);
    virtual void uninit();
    virtual result_t next(query_pack_t* pack);
    virtual const char* name();

private:
    int32 m_index;


    expr_base_t* m_condition;  // filter of where 
};

class join_node_t : public node_base_t
{
public:
    join_node_t(node_base_t* left, node_base_t* right);
    virtual ~join_node_t();

public:
    virtual result_t init(Parse* parse, Select* select);
    virtual void uninit();
    virtual result_t next(query_pack_t* pack);
    virtual const char* name();

private:
    node_base_t* m_left;
    node_base_t* m_right;
};


class group_node_t : public node_base_t
{

};


class aggr_node_t : public node_base_t
{

};

class sort_node_t : public node_base_t
{

};

class project_node_t : public node_base_t
{

};


class expr_base_t
{
public:
    expr_base_t() { 
        m_left = NULL; 
        m_right = NULL; 
    }
    virtual ~expr_base_t() {}

    virtual result_t init(Expr* expr) = 0;
    virtual const char* name() = 0;
    virtual result_t calc(query_pack_t* pack) = 0;

    expr_base_t* m_left;
    expr_base_t* m_right;    
};




class expr_plus_t: public expr_base_t
{
public:
    expr_plus_t();
    virtual ~expr_plus_t();

public:
    virtual result_t init(Expr* expr);
    virtual const char* name() { return "EXPR_PLUS"; }
    virtual result_t calc(query_pack_t* pack);
};

class expr_multiple_t: public expr_base_t
{
public:
    expr_multiple_t();
    virtual ~expr_multiple_t();

public:
    virtual result_t init(Expr* expr);
    virtual const char* name() { return "EXPR_MULTIPLE"; }
    virtual result_t calc(query_pack_t* pack);
};



class expr_integer_t: public expr_base_t
{
public:
    expr_integer_t();
    virtual ~expr_integer_t();

public:
    virtual result_t init(Expr* expr);
    virtual const char* name() { return "EXPR_INTEGER"; }
    virtual result_t calc(query_pack_t* pack);

    //virtual int return_type();
};


template<int OP_TYPE>
class expr_logic_op_t : public expr_base_t
{
public:
    expr_logic_op_t(){}
    virtual ~expr_logic_op_t() {}

public:
    virtual result_t init(Expr* expr) { return RT_FAILED; }
    virtual const char* name() { return "EXPR_LOGIC"; }
    virtual result_t calc(query_pack_t* pack) { return RT_FAILED; }
};



class expr_and_t : public expr_base_t
{
public:
    expr_and_t();
    virtual ~expr_and_t();

public:
    virtual result_t init(Expr* expr);
    virtual const char* name() { return "EXPR_AND"; }
    virtual result_t calc(query_pack_t* pack);
};

class expr_column_t: public expr_base_t
{
public:
    expr_column_t();
    virtual ~expr_column_t();

public:
    virtual result_t init(Expr* expr);
    virtual const char* name() { return "EXPR_COLUMN";  }
    virtual result_t calc(query_pack_t* pack);

private:
    int32 m_table_id;
    int32 m_column_id;
};


#endif //__PARSE_CTX__

