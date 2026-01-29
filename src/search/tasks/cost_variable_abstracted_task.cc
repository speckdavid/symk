#include "cost_variable_abstracted_task.h"

#include "../plugins/plugin.h"
#include "../tasks/root_task.h"
#include "../utils/system.h"

using namespace std;

namespace extra_tasks {

CostVariableAbstractedTask::CostVariableAbstractedTask(
    const shared_ptr<AbstractTask> &parent)
    : DelegatingTask(parent) {
    if (!parent->has_cost_variable()) {
        cerr << "Error: Tried to create CostVariableAbstractedTask from a "
             << "task without cost variable." << endl;
        utils::exit_with(utils::ExitCode::SEARCH_INPUT_ERROR);
    }
    if (parent->get_cost_variable_id() != parent->get_num_variables() - 1) {
        cerr << "Error: Cost variable is not the last variable." << endl;
        utils::exit_with(utils::ExitCode::SEARCH_INPUT_ERROR);
    }
}

int CostVariableAbstractedTask::get_num_variables() const {
    return parent->get_num_variables() - 1;
}

vector<int> CostVariableAbstractedTask::get_initial_state_values() const {
    vector<int> parent_values = parent->get_initial_state_values();
    return vector<int>(parent_values.begin(), parent_values.end() - 1);
}

void CostVariableAbstractedTask::convert_state_values_from_parent(
    vector<int> &values) const {
    values.pop_back();
}

int CostVariableAbstractedTask::get_cost_variable_id() const {
    return -1;
}

class CostVariableAbstractedTaskFeature
    : public plugins::TypedFeature<AbstractTask, CostVariableAbstractedTask> {
public:
    CostVariableAbstractedTaskFeature()
        : TypedFeature("abstract_cost_variable") {
        document_title("Cost-variable-abstracted task");
        document_synopsis(
            "A task transformation that removes the cost variable.");
    }

    virtual shared_ptr<CostVariableAbstractedTask> create_component(
        const plugins::Options & /*opts*/) const override {
        return plugins::make_shared_from_arg_tuples<CostVariableAbstractedTask>(
            tasks::g_root_task);
    }
};

static plugins::FeaturePlugin<CostVariableAbstractedTaskFeature> _plugin;
}
