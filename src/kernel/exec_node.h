// exec_node.h by scott.zgeng@gmail.com 2014.08.08

 
#ifndef  __EXEC_NODE_H__
#define  __EXEC_NODE_H__

extern "C" {
#include "sqliteInt.h"
#include "vdbeInt.h"
}


#include "define.h"
#include "expression.h"
#include "column_table.h"


class node_base_t
{
public:
    enum xchg_type_t {
        XCHG_NONE = 0,      // don't use xchange
        XCHG_SPSC = 1,      // single product single consumer
        XCHG_SPMC = 2,      // single product multi consumer
        XCHG_MPSC = 3,      // multi product single consumer
        XCHG_MPMC = 4,      // multi product multi consumer
        
    };

    friend class node_generator_t;
    virtual ~node_base_t() {}
public:
    virtual result_t init(Parse* parse, Select* select) = 0;
    virtual void uninit() = 0;
    virtual result_t next(rowset_t* rs, mem_stack_t* mem) = 0;
  
    virtual rowset_mode_t rowset_mode() const = 0;
    virtual db_uint32 table_count() const = 0;

    
    virtual xchg_type_t xchange_type() = 0;
    virtual result_t transform(node_base_t* parent) = 0;


    // for parallel execute plan generation
    

    //virtual result_t split() = 0;
    
    // 0 就是不需要创建额外的交换节点， 如果大于0 则根据并行节点个数创建xchg节点
    //virtual result_t parallel_num() = 0;

    // NOTES(scott.zgeng): 
    // 告诉上层节点是否需要 xchange 节点，如果需要，是什么类型的xchange，以及具体参数

    // XCHANGE_NONE     表示不需要交换节点，可随上层节点分裂而分裂
    // XCHANGE_1_TO_1   表示一对一传输，可随上层节点分裂而分裂
    // XCHANGE_N_TO_?   表示多对多传输，节点本身可分裂，上层节点不确定，并且不随上层节点分裂而分裂
    // XCHANGE_1_TO_N   表示一个生产者，一到多个消费者，不随上层节点分裂而分裂


    // 关于节点reset，在单线程的场景下，RESET很简单，但是在多线程执行的场景下，有很多限制，
    // 首先，必须是基于xchange节点为单位来重置，不能直接支持xchange下的单个节点的RESET，
    // 其次, 最好是全部子节点完成后RESET, 并发控制需要非常注意
    // 因此当前阶段，如果需要使用reset功能的节点，就明确不支持多线程
    
    // 典型场景，自底向上分析
    // CASE1: select xxx, yyy from table1 where xxx < nnn 
    //  先看SCAN节点，SCAN节点支持以上四种模式，以应对不同的场景下的需求，具体支持那种模式需要根据上层节点类型和模式来判断
    //    初期就只支持XCHANGE_NONE即可。如果过滤比较耗时，则可以使用XCHANGE_N_TO_?
    //  最后看PROJECT节点，目前只支持XCHANGE_NONE即可

    // CASE2: select xxx, count(xx) from table1 where xxx < nnn group by xxx
    //  先看SCAN节点，使用XCHANGE_NONE或者XCHANGE_N_TO_?，需要根据节点规模来判断
    //  再看GROUP节点，如果扫描节点规模很大，则使用XCHANGE_N_TO_？，这个时候需要看子节点，如果是NONE, 对子节点调用SPLIT
    //  如果是XCHANGE_N_TO_?，则对接xchange；
    //  最后看PROJECT节点，目前只支持XCHANGE_NONE即可

    // CASE3： select table1.xxx, table2.yyy from table1, table2 where table1.xxx < nnn and table1.xxx = table2.xxx;
    //   先看SCAN节点，当如果是JOIN节点时，则最好是直接选择XCHANGE_NONE
    //   在看JOIN节点，如果是两表，则分几种场景，大小表走HASH方式，小表建HASH，大表拆分
    //   大表对大表，就使用MC，一个表全量扫描，一个拆分
    //   其他全部采用NEST LOOP，前期就不考虑多线程，后续可考虑类似MC;
    //  多表暂时不考虑多线程，但可在顶层JOIN节点，增加一个XCHANGE_1_TO_1
    //  多表连接如果是STAR JOIN, 主表大表，其他小标，类似于HASH JOIN，
    //         如果多个大表，则 sort merge; SORT_MERGE，可增加一个XCHANGE_1_TO_1

    // CASE4: select xxx, yyy from table1 where xxx < nnn order by xxx limit NNN;
    //   ORDER BY的场景，目前限定在TOP N的范围，因为分析应用 TOP N有意义，而全数据的排序主要还是用在ETL上，
    //   和定位有一定的差异，所以全数据（特别是分布式情况下）的排序暂不做支持；


};


class expr_base_t;
class node_generator_t
{
public:
    node_generator_t(statement_t* stmt, Parse* parse, Select* select);
    ~node_generator_t();

public:
    result_t create_tree(node_base_t** root);

private:
    result_t build(node_base_t** root);
    result_t build_join(node_base_t** scan_nodes, db_int32 tab_num, node_base_t** root);
    result_t transform(node_base_t* root);    

private:
    Parse* m_parse;
    Select* m_select;
    statement_t* m_stmt;
};






#endif //__EXEC_NODE_H__

