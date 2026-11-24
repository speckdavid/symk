#include "cost_bound_variable_abstracted_task.h"

#include "cost_bound_variable_task.h"

using namespace std;

namespace tasks {
CostBoundVariableAbstractedTask::CostBoundVariableAbstractedTask(
    const shared_ptr<CostBoundVariableTask> &parent)
    : DelegatingTask(parent) {
}

int CostBoundVariableAbstractedTask::get_num_variables() const {
    return parent->get_num_variables() - 1;
}

vector<int> CostBoundVariableAbstractedTask::get_initial_state_values() const {
    vector<int> parent_values = parent->get_initial_state_values();
    return vector<int>(parent_values.begin(), parent_values.end() - 1);
}

void CostBoundVariableAbstractedTask::convert_state_values_from_parent(
    vector<int> &values) const {
    values.pop_back();
}
}