// parse_ctx.h by scott.zgeng@gmail.com 2014.07.11

 
#ifndef  __PARSE_CTX__
#define  __PARSE_CTX__

extern "C" {
#include "../sql/sqliteInt.h"
#include "../sql/vdbeInt.h"
}



#include "define.h"


#define MAX_JOIN_TABLE 8



class query_pack_t;
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

private:        
    result_t build_join(node_base_t** scan_nodes, int32 tab_num, node_base_t** root);       

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




#endif //__PARSE_CTX__

