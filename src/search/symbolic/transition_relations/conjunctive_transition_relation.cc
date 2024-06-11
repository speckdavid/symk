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
static BDD get_exclusive_support(BDD f, BDD g) {
    BDD common, only_f, only_g;
    f.ClassifySupport(g, &common, &only_f, &only_g);
    return only_f;
}

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
}

void ConjunctiveTransitionRelation::sort_transition_relations() {
    auto support = all_exists_vars.SupportIndices();

    vector<vector<unsigned int>> trs_supports;
    for (const auto &tr : transitions) {
        auto tr_support = tr.get_tr_BDD().SupportIndices();
        vector<unsigned int> tr_exist_support;
        set_intersection(tr_support.begin(), tr_support.end(), support.begin(), support.end(), back_inserter(tr_exist_support));
        trs_supports.push_back(tr_exist_support);
    }
    assert(trs_supports.size() == transitions.size());

    vector<pair<int, vector<unsigned int>>> enumerated_trs_supports;
    transform(trs_supports.begin(), trs_supports.end(), back_inserter(enumerated_trs_supports),
              [i = size_t(0)](const auto &elem) mutable {return make_pair(i++, elem);});

    vector<int> new_tr_order;

    while (!enumerated_trs_supports.empty()) {
        size_t best_id = 0;
        size_t max_exclusive_vars = 0;

        // unioned support without tr i
        vector<vector<unsigned int>> trs_others_supports(enumerated_trs_supports.size());
        for (size_t i = 0; i < enumerated_trs_supports.size(); ++i) {
            for (size_t j = 0; j < enumerated_trs_supports.size(); ++j) {
                if (i != j) {
                    vector<unsigned int> new_set;
                    set_union(trs_others_supports[j].begin(), trs_others_supports[j].end(),
                              trs_supports[j].begin(), trs_supports[j].end(), back_inserter(new_set));
                    trs_others_supports[j] = new_set;
                }
            }
        }

        for (size_t i = 0; i < enumerated_trs_supports.size(); ++i) {
            const auto &cur_support = enumerated_trs_supports[i].second;
            const auto &others_support = trs_others_supports[i];
            vector<unsigned int> difference;
            set_difference(cur_support.begin(), cur_support.end(),
                           others_support.begin(), others_support.end(), back_inserter(difference));
            if (difference.size() > max_exclusive_vars) {
                max_exclusive_vars = difference.size();
                best_id = i;
            }
        }

        new_tr_order.push_back(enumerated_trs_supports[best_id].first);
        enumerated_trs_supports.erase(enumerated_trs_supports.begin() + best_id);
    }

    vector<DisjunctiveTransitionRelation> new_transitions;
    for (int index : new_tr_order) {
        new_transitions.push_back(transitions[index]);
    }
    assert(transitions.size() == new_transitions.size());
    transitions = new_transitions;
}

void ConjunctiveTransitionRelation::set_early_exists_and_swap_vars() {
    // utils::g_log << "Support sizes: [ ";
    BDD remaining_exits_vars = all_exists_vars;
    BDD remaining_exits_bw_vars = all_exists_bw_vars;

    for (size_t tr1_id = 0; tr1_id < transitions.size(); ++tr1_id) {
        BDD tr1_exist_vars = remaining_exits_vars;
        BDD tr1_exist_bw_vars = remaining_exits_bw_vars;

        for (size_t tr2_id = tr1_id + 1; tr2_id < transitions.size(); ++tr2_id) {
            tr1_exist_vars = get_exclusive_support(tr1_exist_vars, transitions[tr2_id].get_tr_BDD());
            tr1_exist_bw_vars = get_exclusive_support(tr1_exist_bw_vars, transitions[tr2_id].get_tr_BDD());
        }
        assert(tr1_exist_vars.IsCube());
        assert(tr1_exist_bw_vars.IsCube());

        remaining_exits_vars = get_exclusive_support(remaining_exits_vars, tr1_exist_vars);
        remaining_exits_bw_vars = get_exclusive_support(remaining_exits_bw_vars, tr1_exist_bw_vars);

        exists_vars[tr1_id] = tr1_exist_vars;
        exists_bw_vars[tr1_id] = tr1_exist_bw_vars;

        // utils::g_log << tr1_exist_vars.nodeCount() << ":" << tr1_exist_bw_vars.nodeCount() << " ";
    }
    // utils::g_log << "]" << endl;
}

BDD ConjunctiveTransitionRelation::image(const BDD &from) const {
    return image(from, 0U);
}

BDD ConjunctiveTransitionRelation::preimage(const BDD &from) const {
    return preimage(from, 0U);
}

BDD ConjunctiveTransitionRelation::image(const BDD &from, int max_nodes) const {
    BDD res = from;
    for (size_t tr_id = 0; tr_id < transitions.size(); ++tr_id) {
        const auto &tr = transitions[tr_id];
        res = res.AndAbstract(tr.get_tr_BDD(), exists_vars[tr_id], max_nodes);
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
    }
    // res = res.ExistAbstract(all_exists_bw_vars);
    return res;
}

void ConjunctiveTransitionRelation::merge_transitions(int max_nodes, int max_time) {
    merge(sym_vars, transitions, conjunctive_tr_merge, max_nodes, max_time);
    init_exist_and_swap_vars();
    if (early_quantification) {
        set_early_exists_and_swap_vars();
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
