#include "effect_aggregated_task.h"

#include "../task_proxy.h"

#include "../task_utils/task_properties.h"
#include "../utils/system.h"

#include "../utils/logging.h"

using namespace std;

namespace extra_tasks {
struct EffectInfo {
    map<pair<int, int>, vector<int>> op_var_to_effect_index;

    void insert(int op_index, int var, int eff_index) {
        auto key = make_pair(op_index, var);
        op_var_to_effect_index[key].push_back(eff_index);
    }

    void dump() const {
        for (const auto &entry : op_var_to_effect_index) {
            const auto &key = entry.first;
            const auto &effect_indices = entry.second;

            cout << "Key: (op=" << key.first << ", var=" << key.second << "), Effect Indices: ";
            for (int index : effect_indices) {
                cout << index << " ";
            }
            cout << endl;
        }
    }
};

EffectAggregatedTask::EffectAggregatedTask(const shared_ptr<AbstractTask> &parent)
    : DelegatingTask(parent) {
    TaskProxy task(*parent);
    parent_to_local_op_ids.resize(task.get_operators().size());

    EffectInfo effects;
    for (const auto &op : task.get_operators()) {
        if (op.is_axiom() || !task_properties::has_conditional_effects(task, OperatorID(op.get_id()))) {
            continue;
        }

        for (size_t eff_index = 0; eff_index < op.get_effects().size(); ++eff_index) {
            const EffectProxy &effect = op.get_effects()[eff_index];
            int var = effect.get_fact().get_variable().get_id();
            effects.insert(op.get_id(), var, eff_index);
        }
    }

    for (const auto &pair : effects.op_var_to_effect_index) {
        int op_index = pair.first.first;
        const auto &effect = pair.second;
        parent_to_local_op_ids[op_index].push_back(local_to_parent_op_id.size());
        local_to_parent_op_id.push_back(op_index);
        local_to_parent_eff_ids.push_back(effect);
    }
    assert(local_to_parent_op_id.size() == local_to_parent_op_id.size());
}

string EffectAggregatedTask::get_operator_cost_function(int index, bool is_axiom) const {
    if (is_axiom) {
        return parent->get_operator_cost_function(index, is_axiom);
    }
    int id = convert_operator_index_to_parent(index);
    return parent->get_operator_cost_function(id, is_axiom);
}

int EffectAggregatedTask::get_operator_cost(int index, bool is_axiom) const {
    if (is_axiom) {
        return parent->get_operator_cost(index, is_axiom);
    }
    int id = convert_operator_index_to_parent(index);
    return parent->get_operator_cost(id, is_axiom);
}

int EffectAggregatedTask::get_num_operators() const {
    return local_to_parent_op_id.size();
}

int EffectAggregatedTask::get_num_operator_preconditions(int index, bool is_axiom) const {
    if (is_axiom) {
        return parent->get_num_operator_preconditions(index, is_axiom);
    }
    int id = convert_operator_index_to_parent(index);
    return parent->get_num_operator_preconditions(id, is_axiom);
}

FactPair EffectAggregatedTask::get_operator_precondition(int op_index, int fact_index, bool is_axiom) const {
    if (is_axiom) {
        return parent->get_operator_precondition(op_index, fact_index, is_axiom);
    }
    int id = convert_operator_index_to_parent(op_index);
    return parent->get_operator_precondition(id, fact_index, is_axiom);
}

int EffectAggregatedTask::get_num_operator_effects(int op_index, bool is_axiom) const {
    if (is_axiom) {
        return parent->get_num_operator_effects(op_index, is_axiom);
    }
    return local_to_parent_eff_ids[op_index].size();
}

int EffectAggregatedTask::get_num_operator_effect_conditions(int op_index, int eff_index, bool is_axiom) const {
    if (is_axiom) {
        return parent->get_num_operator_effect_conditions(op_index, eff_index, is_axiom);
    }
    int parent_op_id = convert_operator_index_to_parent(op_index);
    int parent_eff_id = local_to_parent_eff_ids[op_index][eff_index];
    return parent->get_num_operator_effect_conditions(parent_op_id, parent_eff_id, is_axiom);
}

FactPair EffectAggregatedTask::get_operator_effect_condition(int op_index, int eff_index, int cond_index, bool is_axiom) const {
    if (is_axiom) {
        return parent->get_operator_effect_condition(op_index, eff_index, cond_index, is_axiom);
    }
    int parent_op_id = convert_operator_index_to_parent(op_index);
    int parent_eff_id = local_to_parent_eff_ids[op_index][eff_index];
    return parent->get_operator_effect_condition(parent_op_id, parent_eff_id, cond_index, is_axiom);
}

FactPair EffectAggregatedTask::get_operator_effect(int op_index, int eff_index, bool is_axiom) const {
    if (is_axiom) {
        return parent->get_operator_effect(op_index, eff_index, is_axiom);
    }
    int parent_op_id = convert_operator_index_to_parent(op_index);
    int parent_eff_id = local_to_parent_eff_ids[op_index][eff_index];
    return parent->get_operator_effect(parent_op_id, parent_eff_id, is_axiom);
}

string EffectAggregatedTask::get_operator_name(int index, bool is_axiom) const {
    if (is_axiom) {
        return parent->get_operator_name(index, is_axiom);
    }
    int id = convert_operator_index_to_parent(index);
    return parent->get_operator_name(id, is_axiom);
}

int EffectAggregatedTask::convert_operator_index_to_parent(int index) const {
    return local_to_parent_op_id[index];
}

const vector<int> &EffectAggregatedTask::get_operators_beloning_to_parent(int index) const {
    return parent_to_local_op_ids[index];
}
}
