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
    
    // 0 ���ǲ���Ҫ��������Ľ����ڵ㣬 �������0 ����ݲ��нڵ��������xchg�ڵ�
    //virtual result_t parallel_num() = 0;

    // NOTES(scott.zgeng): 
    // �����ϲ�ڵ��Ƿ���Ҫ xchange �ڵ㣬�����Ҫ����ʲô���͵�xchange���Լ��������

    // XCHANGE_NONE     ��ʾ����Ҫ�����ڵ㣬�����ϲ�ڵ���Ѷ�����
    // XCHANGE_1_TO_1   ��ʾһ��һ���䣬�����ϲ�ڵ���Ѷ�����
    // XCHANGE_N_TO_?   ��ʾ��Զഫ�䣬�ڵ㱾��ɷ��ѣ��ϲ�ڵ㲻ȷ�������Ҳ����ϲ�ڵ���Ѷ�����
    // XCHANGE_1_TO_N   ��ʾһ�������ߣ�һ����������ߣ������ϲ�ڵ���Ѷ�����


    // ���ڽڵ�reset���ڵ��̵߳ĳ����£�RESET�ܼ򵥣������ڶ��߳�ִ�еĳ����£��кܶ����ƣ�
    // ���ȣ������ǻ���xchange�ڵ�Ϊ��λ�����ã�����ֱ��֧��xchange�µĵ����ڵ��RESET��
    // ���, �����ȫ���ӽڵ���ɺ�RESET, ����������Ҫ�ǳ�ע��
    // ��˵�ǰ�׶Σ������Ҫʹ��reset���ܵĽڵ㣬����ȷ��֧�ֶ��߳�
    
    // ���ͳ������Ե����Ϸ���
    // CASE1: select xxx, yyy from table1 where xxx < nnn 
    //  �ȿ�SCAN�ڵ㣬SCAN�ڵ�֧����������ģʽ����Ӧ�Բ�ͬ�ĳ����µ����󣬾���֧������ģʽ��Ҫ�����ϲ�ڵ����ͺ�ģʽ���ж�
    //    ���ھ�ֻ֧��XCHANGE_NONE���ɡ�������˱ȽϺ�ʱ�������ʹ��XCHANGE_N_TO_?
    //  ���PROJECT�ڵ㣬Ŀǰֻ֧��XCHANGE_NONE����

    // CASE2: select xxx, count(xx) from table1 where xxx < nnn group by xxx
    //  �ȿ�SCAN�ڵ㣬ʹ��XCHANGE_NONE����XCHANGE_N_TO_?����Ҫ���ݽڵ��ģ���ж�
    //  �ٿ�GROUP�ڵ㣬���ɨ��ڵ��ģ�ܴ���ʹ��XCHANGE_N_TO_�������ʱ����Ҫ���ӽڵ㣬�����NONE, ���ӽڵ����SPLIT
    //  �����XCHANGE_N_TO_?����Խ�xchange��
    //  ���PROJECT�ڵ㣬Ŀǰֻ֧��XCHANGE_NONE����

    // CASE3�� select table1.xxx, table2.yyy from table1, table2 where table1.xxx < nnn and table1.xxx = table2.xxx;
    //   �ȿ�SCAN�ڵ㣬�������JOIN�ڵ�ʱ���������ֱ��ѡ��XCHANGE_NONE
    //   �ڿ�JOIN�ڵ㣬�����������ּ��ֳ�������С����HASH��ʽ��С��HASH�������
    //   ���Դ����ʹ��MC��һ����ȫ��ɨ�裬һ�����
    //   ����ȫ������NEST LOOP��ǰ�ھͲ����Ƕ��̣߳������ɿ�������MC;
    //  �����ʱ�����Ƕ��̣߳������ڶ���JOIN�ڵ㣬����һ��XCHANGE_1_TO_1
    //  ������������STAR JOIN, ����������С�꣬������HASH JOIN��
    //         ����������� sort merge; SORT_MERGE��������һ��XCHANGE_1_TO_1

    // CASE4: select xxx, yyy from table1 where xxx < nnn order by xxx limit NNN;
    //   ORDER BY�ĳ�����Ŀǰ�޶���TOP N�ķ�Χ����Ϊ����Ӧ�� TOP N�����壬��ȫ���ݵ�������Ҫ��������ETL�ϣ�
    //   �Ͷ�λ��һ���Ĳ��죬����ȫ���ݣ��ر��Ƿֲ�ʽ����£��������ݲ���֧�֣�


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

