#ifndef TASKS_COST_VARIABLE_ABSTRACTED_TASK_H
#define TASKS_COST_VARIABLE_ABSTRACTED_TASK_H

#include "delegating_task.h"

#include "../utils/collections.h"

#include <cassert>
#include <string>
#include <utility>
#include <vector>

namespace extra_tasks {
/*
  Task transformation to abstract the cost variable away.
*/
class CostVariableAbstractedTask : public tasks::DelegatingTask {

public:
    CostVariableAbstractedTask(const std::shared_ptr<AbstractTask> &parent);

    virtual int get_num_variables() const override;

    virtual std::vector<int> get_initial_state_values() const override;
    virtual void convert_state_values_from_parent(
        std::vector<int> &values) const override;

    virtual int get_cost_variable_id() const override;
};
}

#endif
