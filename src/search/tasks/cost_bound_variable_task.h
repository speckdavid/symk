#ifndef TASKS_COST_BOUND_VARIABLE_TASK_H
#define TASKS_COST_BOUND_VARIABLE_TASK_H

#include "delegating_task.h"

namespace tasks {
class CostBoundVariableTask : public DelegatingTask {
    int cost_variable_id;
    int cost_domain_size;

public:
    CostBoundVariableTask(
        const std::shared_ptr<AbstractTask> &parent, int cost_domain_size);

    int get_cost_variable_id() const;
    int get_num_variables() const override;
    std::string get_variable_name(int var) const override;
    int get_variable_domain_size(int var) const override;
    int get_variable_axiom_layer(int var) const override;
    int get_variable_default_axiom_value(int var) const override;
    std::string get_fact_name(const FactPair &fact) const override;
    bool are_facts_mutex(
        const FactPair &fact1, const FactPair &fact2) const override;
    std::vector<int> get_initial_state_values() const override;
    void convert_state_values_from_parent(
        std::vector<int> &values) const override;
};
}

#endif