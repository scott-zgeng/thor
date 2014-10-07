// project_node.h by scott.zgeng@gmail.com 2014.08.29



#ifndef  __PROJECT_NODE_H__
#define  __PROJECT_NODE_H__


#include "exec_node.h"
#include "pod_vector.h"

class project_node_t : public node_base_t
{
public:    
    project_node_t(statement_t* stmt, node_base_t* children);
    virtual ~project_node_t();

public:
    virtual result_t init(Parse* parse, Select* select);
    virtual void uninit();
    virtual result_t next(rowset_t* rs, mem_stack_t* mem);
    
    virtual rowset_mode_t rowset_mode() const {
        return m_children->rowset_mode();
    }

    virtual db_uint32 table_count() const {
        return m_children->table_count();
    }

    result_t next();
    db_int32 count() const {
        return m_sub_rowset->count;
    }

    db_uint32 column_count() const {
        return m_expr_columns.size();
    }

    void* column_data(db_int32 idx) {
        return m_expr_values[idx];
    }

    const db_char* column_name(db_int32 idx) const {        
        return "column"; // TODO(scott.zgeng): for test
    }

    data_type_t column_type(db_int32 idx) {
        return m_expr_columns[idx]->data_type();
    }

    db_uint32 column_size(db_int32 idx) {
        return 4; // TODO(scott.zgeng): for test
    }


    virtual xchg_type_t xchange_type() {
        return XCHG_NONE;
    }

    virtual result_t transform(node_base_t* parent);

private:
    statement_t* m_stmt;
    node_base_t* m_children;
    expr_list_t m_expr_columns;
    mem_stack_t m_mem;
    rowset_t* m_sub_rowset;
    pod_vector<void*, DEFAULT_EXPR_NUM> m_expr_values;


};

#endif //__PROJECT_NODE_H__


