#ifndef TASKS_EFFECT_AGGREGATED_TASK_H
#define TASKS_EFFECT_AGGREGATED_TASK_H

#include "delegating_task.h"

#include <map>
#include <set>


namespace extra_tasks {
class EffectAggregatedTask : public tasks::DelegatingTask {
    // Mapping the operator ids
    std::vector<int> local_to_parent_op_id;
    std::vector<std::vector<int>> local_to_parent_eff_ids;
    std::vector<std::vector<int>> parent_to_local_op_ids;

public:
    EffectAggregatedTask(const std::shared_ptr<AbstractTask> &parent);
    ~EffectAggregatedTask() = default;

    virtual int get_operator_cost(int index, bool is_axiom) const override;
    virtual std::string get_operator_cost_function(int index, bool is_axiom) const override;
    virtual std::string get_operator_name(int index, bool is_axiom) const override;
    virtual int get_num_operators() const override;
    virtual int get_num_operator_preconditions(int index, bool is_axiom) const override;
    virtual FactPair get_operator_precondition(int op_index, int fact_index, bool is_axiom) const override;
    virtual int get_num_operator_effects(int op_index, bool is_axiom) const override;
    virtual int get_num_operator_effect_conditions(int op_index, int eff_index, bool is_axiom) const override;
    virtual FactPair get_operator_effect_condition(int op_index, int eff_index, int cond_index, bool is_axiom) const override;
    virtual FactPair get_operator_effect(int op_index, int eff_index, bool is_axiom) const override;

    virtual int convert_operator_index_to_parent(int index) const override;

    const std::vector<int> &get_operators_beloning_to_parent(int index) const;
};
}

#endif
