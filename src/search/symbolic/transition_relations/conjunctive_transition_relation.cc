#include "conjunctive_transition_relation.h"

#include "../../tasks/effect_aggregated_task.h"
#include "../../utils/logging.h"

#include "../sym_utils.h"

#include <algorithm>
#include <numeric>
#include <set>
#include <vector>

using namespace std;

namespace symbolic {
ConjunctiveTransitionRelation::ConjunctiveTransitionRelation(SymVariables *sym_vars,
                                                             OperatorID op_id,
                                                             const shared_ptr<extra_tasks::EffectAggregatedTask> &task,
                                                             bool early_quantification)
    : TransitionRelation(),
      sym_vars(sym_vars),
      task(task),
      operator_id(op_id),
      early_quantification(early_quantification),
      all_exists_vars(sym_vars->oneBDD()),
      all_exists_bw_vars(sym_vars->oneBDD()) {}

void ConjunctiveTransitionRelation::init() {
    TaskProxy task_proxy(*task);

    for (int ce_op_id : task->get_operators_beloning_to_parent(operator_id.get_index())) {
        transitions.emplace_back(sym_vars, OperatorID(ce_op_id), task);
        auto &tr = transitions.back();
        tr.init();
        tr.setOpsIds({operator_id});
    }

    init_exist_and_swap_vars();
}

void ConjunctiveTransitionRelation::init_exist_and_swap_vars() {
    all_exists_vars = sym_vars->oneBDD();
    all_exists_bw_vars = sym_vars->oneBDD();

    for (const auto &tr : transitions) {
        all_exists_vars *= tr.get_exists_vars();
        all_exists_bw_vars *= tr.get_exists_bw_vars();
        all_swap_vars.insert(all_swap_vars.end(), tr.get_swap_vars().begin(), tr.get_swap_vars().end());
        all_swap_vars_p.insert(all_swap_vars_p.end(), tr.get_swap_vars_p().begin(), tr.get_swap_vars_p().end());
    }

    // Remove duplicates from all_swap_vars and all_swap_vars_p
    sort(all_swap_vars.begin(), all_swap_vars.end());
    all_swap_vars.erase(unique(all_swap_vars.begin(), all_swap_vars.end()), all_swap_vars.end());
    sort(all_swap_vars_p.begin(), all_swap_vars_p.end());
    all_swap_vars_p.erase(unique(all_swap_vars_p.begin(), all_swap_vars_p.end()), all_swap_vars_p.end());

    // By default we just quantify everything with the last transition relation
    exists_vars = vector<BDD>(transitions.size(), sym_vars->oneBDD());
    exists_bw_vars = vector<BDD>(transitions.size(), sym_vars->oneBDD());

    exists_vars.back() = all_exists_vars;
    exists_bw_vars.back() = all_exists_bw_vars;

    if (early_quantification) {
        set_early_exists_vars();
    }
}

void ConjunctiveTransitionRelation::set_early_exists_vars() {
    unordered_map<int, int> vars_last_occurance;
    unordered_map<int, int> vars_bw_last_occurance;

    for (int bdd_var_level : all_exists_vars.SupportIndices()) {
        vars_last_occurance[bdd_var_level] = -1;
    }
    for (int bdd_var_level : all_exists_bw_vars.SupportIndices()) {
        vars_bw_last_occurance[bdd_var_level] = -1;
    }

    // Track the last occurrence of each variable in the transitions
    for (size_t tr_id = 0; tr_id < transitions.size(); ++tr_id) {
        const auto &tr = transitions[tr_id];
        for (int bdd_var_level : tr.get_tr_BDD().SupportIndices()) {
            if (vars_last_occurance.count(bdd_var_level)) {
                vars_last_occurance[bdd_var_level] = tr_id;
            }
        }
        for (int bdd_var_level : tr.get_tr_BDD().SupportIndices()) {
            if (vars_bw_last_occurance.count(bdd_var_level)) {
                vars_bw_last_occurance[bdd_var_level] = tr_id;
            }
        }
    }

    // Initialize the BDD vectors with the one BDDs
    exists_vars = vector<BDD>(transitions.size(), sym_vars->oneBDD());
    exists_bw_vars = vector<BDD>(transitions.size(), sym_vars->oneBDD());

    // Add the exist variable to the last TR having the variable in the support
    for (auto [bdd_var_level, tr_id] : vars_last_occurance) {
        if (tr_id != -1) {
            exists_vars[tr_id] *= sym_vars->levelBDD(bdd_var_level);
        } else {
            exists_vars[0] *= sym_vars->levelBDD(bdd_var_level);
        }
    }
    for (auto [bdd_var_level, tr_id] : vars_bw_last_occurance) {
        if (tr_id != -1) {
            exists_bw_vars[tr_id] *= sym_vars->levelBDD(bdd_var_level);
        } else {
            exists_bw_vars[0] *= sym_vars->levelBDD(bdd_var_level);
        }
    }
}

BDD ConjunctiveTransitionRelation::image(const BDD &from, int max_nodes) const {
    BDD res = from;
    for (size_t tr_id = 0; tr_id < transitions.size(); ++tr_id) {
        const auto &tr = transitions[tr_id];
        res = res.AndAbstract(tr.get_tr_BDD(), exists_vars[tr_id], max_nodes);
        if (res.IsZero()) {
            return res;
        }
    }
    // res = res.ExistAbstract(all_exists_vars);
    res = res.SwapVariables(all_swap_vars, all_swap_vars_p);
    return res;
}

BDD ConjunctiveTransitionRelation::preimage(const BDD &from, int max_nodes) const {
    BDD res = from;
    res = res.SwapVariables(all_swap_vars, all_swap_vars_p);
    for (size_t tr_id = 0; tr_id < transitions.size(); ++tr_id) {
        const auto &tr = transitions[tr_id];
        res = res.AndAbstract(tr.get_tr_BDD(), exists_bw_vars[tr_id], max_nodes);
        if (res.IsZero()) {
            return res;
        }
    }
    // res = res.ExistAbstract(all_exists_bw_vars);
    return res;
}

BDD ConjunctiveTransitionRelation::preimage(const BDD &from, const BDD &constraint_to, int max_nodes) const {
    BDD res = from;
    res = res.SwapVariables(all_swap_vars, all_swap_vars_p);
    res *= constraint_to;
    for (size_t tr_id = 0; tr_id < transitions.size(); ++tr_id) {
        const auto &tr = transitions[tr_id];
        res = res.AndAbstract(tr.get_tr_BDD(), exists_bw_vars[tr_id], max_nodes);
        if (res.IsZero()) {
            return res;
        }
    }
    return res;
}

void ConjunctiveTransitionRelation::merge_transitions(int max_time, int max_nodes) {
    merge(sym_vars, transitions, conjunctive_tr_merge, max_time, max_nodes);
    init_exist_and_swap_vars();
    if (early_quantification) {
        set_early_exists_vars();
    }
}

const vector<DisjunctiveTransitionRelation> &ConjunctiveTransitionRelation::get_transitions() const {
    return transitions;
}

int ConjunctiveTransitionRelation::nodeCount() const {
    return accumulate(transitions.begin(), transitions.end(), 0, [](int acc, const auto &tr) {return acc + tr.nodeCount();});
}

const OperatorID &ConjunctiveTransitionRelation::get_unique_operator_id() const {
    return operator_id;
}

int ConjunctiveTransitionRelation::size() const {
    return transitions.size();
}

bool ConjunctiveTransitionRelation::is_monolithic() const {
    return size() == 1;
}
}
