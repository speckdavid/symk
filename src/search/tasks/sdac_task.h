#ifndef TASKS_SDAC_TASK_H
#define TASKS_SDAC_TASK_H

#include "delegating_task.h"

#include "cuddObj.hh"

#include <vector>

namespace symbolic {
class SymVariables;
}

namespace extra_tasks {
class SdacTask : public tasks::DelegatingTask {
    symbolic::SymVariables *sym_vars;
    std::vector<BDD> cost_condition_for_op;
    std::vector<int> constant_op_cost;
    std::vector<int> parent_id;

public:
    SdacTask(const std::shared_ptr<AbstractTask> &parent,
             symbolic::SymVariables *sym_vars);
    ~SdacTask() = default;

    virtual int get_operator_cost(int index, bool is_axiom) const;
    virtual std::string get_operator_name(int index, bool is_axiom) const override;
    virtual int get_num_operators() const override;
    virtual int get_num_operator_preconditions(int index, bool is_axiom) const override;
    virtual FactPair get_operator_precondition(
        int op_index, int fact_index, bool is_axiom) const override;
    virtual int get_num_operator_effects(int op_index, bool is_axiom) const override;
    virtual int get_num_operator_effect_conditions(
        int op_index, int eff_index, bool is_axiom) const override;
    virtual FactPair get_operator_effect_condition(
        int op_index, int eff_index, int cond_index, bool is_axiom) const override;
    virtual FactPair get_operator_effect(
        int op_index, int eff_index, bool is_axiom) const override;

    virtual int convert_operator_index_to_parent(int index) const;


    BDD get_operator_cost_condition(int index, bool is_axiom) const;
};
}

#endif
