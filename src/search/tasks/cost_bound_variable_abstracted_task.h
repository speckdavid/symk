#ifndef TASKS_COST_BOUND_VARIABLE_ABSTRACTED_TASK_H
#define TASKS_COST_BOUND_VARIABLE_ABSTRACTED_TASK_H

#include "delegating_task.h"

#include <vector>

namespace tasks {
class CostBoundVariableTask;

class CostBoundVariableAbstractedTask : public DelegatingTask {
public:
    explicit CostBoundVariableAbstractedTask(
        const std::shared_ptr<CostBoundVariableTask> &parent);

    virtual int get_num_variables() const override;

    virtual std::vector<int> get_initial_state_values() const override;
    virtual void convert_state_values_from_parent(
        std::vector<int> &values) const override;
};
}

#endif