#include "sdac_task.h"
#include "../symbolic/sym_function_creator.h"
#include "../symbolic/sym_variables.h"

using namespace std;

namespace extra_tasks {
static void create_bdds_from_add(symbolic::SymVariables *sym_vars, ADD add,
                                 map<int, BDD> &result) {
    assert(result.empty());
    ADD cur_add = add;
    double min_value = Cudd_V(cur_add.FindMin().getNode());

    while (cur_add != sym_vars->constant(numeric_limits<double>::infinity())) {
        result[min_value] = cur_add.BddInterval(min_value, min_value);
        cur_add = cur_add.Maximum(result[min_value].Add()
                                  * sym_vars->constant(numeric_limits<double>::infinity()));
        min_value = Cudd_V(cur_add.FindMin().getNode());
    }
}

SdacTask::SdacTask(const shared_ptr<AbstractTask> &parent, symbolic::SymVariables *sym_vars)
    : DelegatingTask(parent),
      sym_vars(sym_vars) {
    symbolic::SymbolicFunctionCreator creator(sym_vars, parent);

    for (int op_id = 0; op_id < parent->get_num_operators(); ++op_id) {
        ADD cost_function = creator.create_add(parent->get_operator_cost_function(op_id, false));
        map<int, BDD> cost_cond;
        create_bdds_from_add(sym_vars, cost_function, cost_cond);
        for (auto pair : cost_cond) {
            constant_op_cost.push_back(pair.first);
            cost_condition_for_op.push_back(pair.second);
            parent_id.push_back(op_id);
        }
    }
}

int SdacTask::get_num_operators() const {
    return parent_id.size();
}

int SdacTask::get_operator_cost(int index, bool is_axiom) const {
    if (is_axiom) {
        return parent->get_operator_cost(index, is_axiom);
    }
    return constant_op_cost.at(index);
}

int SdacTask::get_num_operator_preconditions(int index, bool is_axiom) const {
    int id = is_axiom ? index : convert_operator_index_to_parent(index);
    return parent->get_num_operator_preconditions(id, is_axiom);
}

FactPair SdacTask::get_operator_precondition(
    int op_index, int fact_index, bool is_axiom) const {
    int id = is_axiom ? op_index : convert_operator_index_to_parent(op_index);
    return parent->get_operator_precondition(id, fact_index, is_axiom);
}

int SdacTask::get_num_operator_effects(int op_index, bool is_axiom) const {
    int id = is_axiom ? op_index : convert_operator_index_to_parent(op_index);
    return parent->get_num_operator_effects(id, is_axiom);
}

int SdacTask::get_num_operator_effect_conditions(
    int op_index, int eff_index, bool is_axiom) const {
    int id = is_axiom ? op_index : convert_operator_index_to_parent(op_index);
    return parent->get_num_operator_effect_conditions(id, eff_index, is_axiom);
}

FactPair SdacTask::get_operator_effect_condition(
    int op_index, int eff_index, int cond_index, bool is_axiom) const {
    int id = is_axiom ? op_index : convert_operator_index_to_parent(op_index);
    return parent->get_operator_effect_condition(id, eff_index, cond_index, is_axiom);
}

FactPair SdacTask::get_operator_effect(
    int op_index, int eff_index, bool is_axiom) const {
    int id = is_axiom ? op_index : convert_operator_index_to_parent(op_index);
    return parent->get_operator_effect(id, eff_index, is_axiom);
}

string SdacTask::get_operator_name(int index, bool is_axiom) const {
    int id = is_axiom ? index : convert_operator_index_to_parent(index);
    return parent->get_operator_name(id, is_axiom);
}

int SdacTask::convert_operator_index_to_parent(int index) const {
    return parent_id.at(index);
}

BDD SdacTask::get_operator_cost_condition(int index, bool is_axiom) const {
    return is_axiom ? sym_vars->oneBDD() : cost_condition_for_op.at(index);
}
}
