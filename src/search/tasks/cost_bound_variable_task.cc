#include "cost_bound_variable_task.h"

#include "../utils/system.h"

#include <iostream>

using namespace std;

namespace tasks {
CostBoundVariableTask::CostBoundVariableTask(
    const shared_ptr<AbstractTask> &parent, int cost_domain_size)
    : DelegatingTask(parent),
      cost_variable_id(parent->get_num_variables()),
      cost_domain_size(cost_domain_size) {
    if (cost_domain_size < 1) {
        cerr << "The cost variable domain size must be positive." << endl;
        utils::exit_with(utils::ExitCode::SEARCH_INPUT_ERROR);
    }
}

int CostBoundVariableTask::get_cost_variable_id() const {
    return cost_variable_id;
}

int CostBoundVariableTask::get_num_variables() const {
    return cost_variable_id + 1;
}

string CostBoundVariableTask::get_variable_name(int var) const {
    return var == cost_variable_id ? "cost_bound_variable"
                                   : parent->get_variable_name(var);
}

int CostBoundVariableTask::get_variable_domain_size(int var) const {
    return var == cost_variable_id ? cost_domain_size
                                   : parent->get_variable_domain_size(var);
}

int CostBoundVariableTask::get_variable_axiom_layer(int var) const {
    return var == cost_variable_id ? -1 : parent->get_variable_axiom_layer(var);
}

int CostBoundVariableTask::get_variable_default_axiom_value(int var) const {
    return var == cost_variable_id
               ? -1
               : parent->get_variable_default_axiom_value(var);
}

string CostBoundVariableTask::get_fact_name(const FactPair &fact) const {
    return fact.var == cost_variable_id ? "cost=" + to_string(fact.value)
                                        : parent->get_fact_name(fact);
}

bool CostBoundVariableTask::are_facts_mutex(
    const FactPair &fact1, const FactPair &fact2) const {
    if (fact1.var == cost_variable_id || fact2.var == cost_variable_id) {
        return fact1.var == fact2.var && fact1.value != fact2.value;
    }
    return parent->are_facts_mutex(fact1, fact2);
}

vector<int> CostBoundVariableTask::get_initial_state_values() const {
    vector<int> values = parent->get_initial_state_values();
    values.push_back(0);
    return values;
}

void CostBoundVariableTask::convert_state_values_from_parent(
    vector<int> &) const {
    cerr << "Cannot infer a path cost from a state of the parent task." << endl;
    utils::exit_with(utils::ExitCode::SEARCH_INPUT_ERROR);
}
}