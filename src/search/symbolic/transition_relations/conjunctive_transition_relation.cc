#include "conjunctive_transition_relation.h"

#include "../../tasks/effect_aggregated_task.h"
#include "../../utils/logging.h"

#include "../sym_enums.h"
#include "../sym_utils.h"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <set>
#include <vector>

using namespace std;

// TODO: Disable Axiom Support for the effect based compilation. In general it should work but we need to test it.

namespace symbolic {
#if !NDEBUG
static bool is_simple_disjunction_or_conjunction(const BDD &bdd) {
    constexpr double epsilon = 1e-9;
    constexpr double one = 1.0;

    BDD support = bdd.Support();
    int nodeCount = bdd.nodeCount();  // Store node count to avoid recomputation

    // Check if the support is the same size as the BDD itself
    if (support.nodeCount() != nodeCount)
        return false;

    int nodes = nodeCount - 1;
    double disjunction_states = pow(2, nodes) - 1;
    double minterms = numeric_limits<double>::infinity();
    try {
        minterms = bdd.CountMinterm(nodes);
    } catch (const BDDError &e) {
    }

    // Check for disjunction or conjunction
    return minterms == disjunction_states  // Exact match for large values (inf)
           || fabs(minterms - one) < epsilon // Conjunction: Only one minterm
           || fabs(minterms - disjunction_states) < epsilon;   // Disjunction: All but one minterm
}
#endif

ConjunctiveTransitionRelation::ConjunctiveTransitionRelation(SymVariables *sym_vars,
                                                             OperatorID op_id,
                                                             const shared_ptr<extra_tasks::EffectAggregatedTask> &task,
                                                             const ConditionalEffectsTransitionType &type)
    : TransitionRelation(),
      sym_vars(sym_vars),
      task(task),
      operator_id(op_id),
      dynamic_splitting(type == ConditionalEffectsTransitionType::DYNAMIC),
      variable_based_splitting(is_ce_transition_type_variable_based(type)),
      early_quantification(is_ce_transition_type_early_quantification(type)),
      all_exists_vars(sym_vars->oneBDD()),
      all_exists_bw_vars(sym_vars->oneBDD()),
      next_aux_var_id(0) {}

// Resets the necessary variables for effect-based fallback.
void ConjunctiveTransitionRelation::reset() {
    variable_based_splitting = false;
    all_exists_vars = sym_vars->oneBDD();
    all_exists_bw_vars = sym_vars->oneBDD();
    all_swap_vars.clear();
    all_swap_vars_p.clear();
    exists_vars.clear();
    exists_bw_vars.clear();
    transitions.clear();
}

void ConjunctiveTransitionRelation::init() {
    if (dynamic_splitting) {
        init_dynamically();
    } else if (variable_based_splitting) {
        init_variable_based();
    } else {
        init_effect_based();
    }
    init_exist_and_swap_vars();
}


// We try for 5 seconds to create the TR with variable-based splitting.
// Afterwards, we proceed with the fallback of effect-based splitting.
void ConjunctiveTransitionRelation::init_dynamically() {
    sym_vars->set_time_limit(5000); // 5 seconds
    try {
        variable_based_splitting = true;
        init_variable_based();
    } catch (const BDDError &e) {
        sym_vars->unset_time_limit();
        reset();
        variable_based_splitting = false;
        init_effect_based();
    }
    sym_vars->unset_time_limit();
}



// We create one disjunctive TR for each effect variable of the global operator
void ConjunctiveTransitionRelation::init_variable_based() {
    // Variable based operators
    for (int ce_op_id : task->get_operators_beloning_to_parent(operator_id.get_index())) {
        transitions.emplace_back(sym_vars, OperatorID(ce_op_id), task);
        auto &tr = transitions.back();
        tr.init();
        tr.setOpsIds({operator_id});
    }
}

void ConjunctiveTransitionRelation::init_effect_based() {
    TaskProxy task_proxy(*task);

    // Get relevant operators based on the parent operator
    const auto &relevant_op_ids = task->get_operators_beloning_to_parent(operator_id.get_index());
    var_based_effect_transitions.resize(relevant_op_ids.size());

    // Iterate over the relevant operator IDs
    for (size_t eff_op_id = 0; eff_op_id < relevant_op_ids.size(); ++eff_op_id) {
        int ce_op_id = relevant_op_ids[eff_op_id];
        auto &var_transitions = var_based_effect_transitions[eff_op_id];
        OperatorProxy effect_op = task_proxy.get_operators()[ce_op_id];

        // Skip if the operator has no effects
        if (effect_op.get_effects().size() == 0) {
            continue;
        }

        // Add preconditions to transitions
        add_precondition_trs(effect_op.get_preconditions(), var_transitions);

        unordered_set<int> condition_aux_vars;
        unordered_set<FactPair> single_effect_conditions;

        // Process each effect in the operator
        for (const EffectProxy &effect : effect_op.get_effects()) {
            const auto &conditions = effect.get_conditions();
            size_t num_conditions = conditions.size();

            if (is_simple_condition(conditions)) {
                // Simple condition: no auxiliary variables
                add_effect_trs(effect, var_transitions);
                if (num_conditions == 1) {
                    single_effect_conditions.insert(conditions[0].get_pair());
                } else {
                    // No condition: represent it with a special value
                    single_effect_conditions.insert(FactPair::no_fact);
                }
            } else {
                //Complex condition: use auxiliary variables
                int aux_var_id = get_aux_var_id(conditions);
                add_aux_condition_trs(effect, aux_var_id, var_transitions);
                add_effect_trs(effect, aux_var_id, var_transitions);
                condition_aux_vars.insert(aux_var_id);
            }
        }

        // Add frame transitions for the primary effect variable
        FactProxy effect_var = effect_op.get_effects()[0].get_fact();
        add_frame_trs(effect_var, condition_aux_vars, single_effect_conditions, var_transitions);

        // Finalize transitions by setting operator ID and cost
        for (auto &transition : var_transitions) {
            assert(is_simple_disjunction_or_conjunction(transition.get_tr_BDD()));
            transition.setOpsIds({operator_id});
            transition.set_cost(effect_op.get_cost());
            transitions.push_back(transition);
        }
    }
}

int ConjunctiveTransitionRelation::get_aux_var_id(const ConditionsProxy &cond) {
    set<FactPair> set_cond;
    for (FactProxy fact_proxy : cond) {
        set_cond.insert(fact_proxy.get_pair());
    }
    if (!eff_condition_to_aux_var_id.contains(set_cond)) {
        eff_condition_to_aux_var_id[set_cond] = next_aux_var_id;
        ++next_aux_var_id;
    }
    return eff_condition_to_aux_var_id[set_cond];
}

bool ConjunctiveTransitionRelation::is_simple_condition(const ConditionsProxy &cond) {
    if (cond.size() > 1)
        return false;

    if (cond.size() == 0)
        return true;

    return cond[0].get_variable().get_domain_size() <= 2;
}

void ConjunctiveTransitionRelation::add_precondition_trs(const PreconditionsProxy &pre, vector<DisjunctiveTransitionRelation> &transitions) {
    if (!pre.empty()) {
        transitions.emplace_back(sym_vars, task);
        auto &tr = transitions.back();
        tr.add_condition(pre);
    }
}

void ConjunctiveTransitionRelation::add_aux_condition_trs(const EffectProxy &eff, int aux_var_id, vector<DisjunctiveTransitionRelation> &transitions) {
    // cond => aux == !cond OR aux (disjunction)
    transitions.emplace_back(sym_vars, task);
    auto &cond_aux_tr = transitions.back();
    cond_aux_tr.add_condition(eff.get_conditions());
    cond_aux_tr.set_tr_BDD(!cond_aux_tr.get_tr_BDD() + sym_vars->auxBDD(aux_var_id, 1));

    // aux => cond (multiple ones for each binary variable of cond)
    for (const auto &cond : eff.get_conditions()) {
        int cond_var = cond.get_pair().var;
        int cond_value = cond.get_pair().value;
        const vector<int> &pre_bdd_levels = sym_vars->vars_index_pre(cond_var);
        vector<BDD> cond_value_bdds;
        sym_vars->get_variable_value_bdds(pre_bdd_levels, cond_value, cond_value_bdds);
        for (BDD pre_bdd : cond_value_bdds) {
            transitions.emplace_back(sym_vars, task);
            auto &tr = transitions.back();
            tr.set_tr_BDD(!sym_vars->auxBDD(aux_var_id, 1) + pre_bdd);
        }
    }
}

void ConjunctiveTransitionRelation::add_effect_trs(const EffectProxy &eff, vector<DisjunctiveTransitionRelation> &transitions) {
    assert(eff.get_conditions().size() <= 1);
    int eff_var = eff.get_fact().get_pair().var;
    int eff_value = eff.get_fact().get_pair().value;

    // We create one TR for each binary variable of the effect to obtain a pure CNF
    const vector<int> &eff_bdd_levels = sym_vars->vars_index_eff(eff_var);
    vector<BDD> eff_value_bdds;
    sym_vars->get_variable_value_bdds(eff_bdd_levels, eff_value, eff_value_bdds);
    for (BDD eff_bdd : eff_value_bdds) {
        transitions.emplace_back(sym_vars, task);
        auto &tr = transitions.back();
        tr.add_condition(eff.get_conditions());
        tr.set_tr_BDD(!tr.get_tr_BDD() + eff_bdd);
        tr.add_exist_var(eff_var);
        tr.add_eff_var(eff_var);
        tr.add_swap_var(eff_var);
    }
}

void ConjunctiveTransitionRelation::add_effect_trs(const EffectProxy &eff, int aux_var_id, vector<DisjunctiveTransitionRelation> &transitions) {
    int eff_var = eff.get_fact().get_pair().var;
    int eff_value = eff.get_fact().get_pair().value;
    BDD aux_bdd = sym_vars->auxBDD(aux_var_id, 1);

    // We create one TR for each binary variable of the effect to obtain a pure CNF
    const vector<int> &eff_bdd_levels = sym_vars->vars_index_eff(eff_var);
    vector<BDD> eff_value_bdds;
    sym_vars->get_variable_value_bdds(eff_bdd_levels, eff_value, eff_value_bdds);
    for (BDD eff_bdd : eff_value_bdds) {
        transitions.emplace_back(sym_vars, task);
        auto &tr = transitions.back();
        tr.set_tr_BDD(!aux_bdd + eff_bdd);
        tr.add_exist_var(eff_var);
        tr.add_exist_var(aux_bdd);
        tr.add_eff_var(eff_var);
        tr.add_swap_var(eff_var);
    }
}

void ConjunctiveTransitionRelation::add_frame_trs(const FactProxy &eff_variable,
                                                  const unordered_set<int> &condition_aux_var_ids,
                                                  const unordered_set<FactPair> &single_effect_conditions,
                                                  vector<DisjunctiveTransitionRelation> &transitions) {
    if (condition_aux_var_ids.empty() && single_effect_conditions.empty()) {
        return;
    }
    int eff_var = eff_variable.get_pair().var;
    BDD condition_bdd = sym_vars->zeroBDD();
    BDD aux_cube_bdd = sym_vars->oneBDD();

    for (int cond_aux_var_id : condition_aux_var_ids) {
        condition_bdd += sym_vars->auxBDD(cond_aux_var_id, 1);
        aux_cube_bdd *= sym_vars->auxBDD(cond_aux_var_id, 1);
    }

    for (const FactPair &cond : single_effect_conditions) {
        if (cond == FactPair::no_fact) {
            condition_bdd += sym_vars->oneBDD();
            break;
        } else {
            condition_bdd += sym_vars->preBDD(cond.var, cond.value);
        }
    }

    // For each binary variable, we create frame axioms.
    // If the condition does not hold (!cond), the effect variable (eff_var) remains unchanged.
    // This is encoded as two separate transitions:
    // 1. !cond => (eff_var OR !eff_var')
    // 2. !cond => (!eff_var OR eff_var')
    const vector<int> &pre_bdd_levels = sym_vars->vars_index_pre(eff_var);
    const vector<int> &eff_bdd_levels = sym_vars->vars_index_eff(eff_var);
    assert(pre_bdd_levels.size() == eff_bdd_levels.size());

    for (size_t i = 0; i < pre_bdd_levels.size(); ++i) {
        const auto pre_level_bdd = sym_vars->levelBDD(pre_bdd_levels.at(i));
        const auto eff_level_bdd = sym_vars->levelBDD(eff_bdd_levels.at(i));

        // First transition: !cond => (eff_var OR !eff_var')
        transitions.emplace_back(sym_vars, task);
        auto &tr1 = transitions.back();
        tr1.set_tr_BDD(condition_bdd + pre_level_bdd + !eff_level_bdd);
        tr1.add_exist_var(aux_cube_bdd);
        tr1.add_exist_var(eff_var);
        tr1.add_eff_var(eff_var);
        tr1.add_swap_var(eff_var);

        // Second transition: !cond => (!eff_var OR eff_var')
        transitions.emplace_back(sym_vars, task);
        auto &tr2 = transitions.back();
        tr2.set_tr_BDD(condition_bdd + !pre_level_bdd + eff_level_bdd);
        tr2.add_exist_var(aux_cube_bdd);
        tr2.add_exist_var(eff_var);
        tr2.add_eff_var(eff_var);
        tr2.add_swap_var(eff_var);
    }
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

    if (!variable_based_splitting) {
        prune_unused_exist_vars(transitions);
    }

    if (early_quantification) {
        set_early_exists_vars();
    }
}

void ConjunctiveTransitionRelation::prune_unused_exist_vars(vector<DisjunctiveTransitionRelation> &transitions) {
    unordered_map<int, int> aux_var_count;

    for (int bdd_var_level : sym_vars->get_aux_cube().SupportIndices()) {
        aux_var_count[bdd_var_level] = 0;
    }

    // Track the last occurrence of each variable in the transitions
    for (size_t tr_id = 0; tr_id < transitions.size(); ++tr_id) {
        const auto &tr = transitions[tr_id];
        for (int bdd_var_level : tr.get_tr_BDD().SupportIndices()) {
            if (aux_var_count.count(bdd_var_level)) {
                aux_var_count[bdd_var_level]++;
            }
        }
    }

    BDD remove_aux = sym_vars->oneBDD();
    for (auto [bdd_level, count] : aux_var_count) {
        if (count == 1) {
            remove_aux *= sym_vars->levelBDD(bdd_level);
        }
    }

    for (auto &tr : transitions) {
        BDD new_tr_bdd = tr.get_tr_BDD().ExistAbstract(remove_aux);
        tr.set_tr_BDD(new_tr_bdd);
        tr.remove_exist_var(remove_aux);
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
    assert(!sym_vars->has_aux_variables_in_support(res));
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
    assert(!sym_vars->has_aux_variables_in_support(res));
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
    assert(!sym_vars->has_aux_variables_in_support(res));
    return res;
}

void ConjunctiveTransitionRelation::variable_based_transition_merging(int max_time, int max_nodes) {
    merge(sym_vars, transitions, conjunctive_tr_merge, max_time, max_nodes);
    init_exist_and_swap_vars();
}

// We try to merge per effect variable to faster get rid of aux variables and simulate the good
// working variable based partioning if feasible
void ConjunctiveTransitionRelation::effect_based_transition_merging(int max_time, int max_nodes) {
    for (auto &var_based_trs : var_based_effect_transitions) {
        merge(sym_vars, var_based_trs, conjunctive_tr_merge, max_time, max_nodes);
        prune_unused_exist_vars(var_based_trs);
    }
    transitions.clear();
    for (auto &var_based_trs : var_based_effect_transitions) {
        for (DisjunctiveTransitionRelation &tr : var_based_trs) {
            transitions.push_back(tr);
        }
    }
    prune_unused_exist_vars(transitions);
    variable_based_transition_merging(max_time, max_nodes);

    assert(!is_monolithic() || sym_vars->get_aux_variables_in_support(transitions[0].get_tr_BDD()).IsOne());
    assert(!is_monolithic() || sym_vars->get_aux_variables_in_support(transitions[0].get_exists_vars()).IsOne());
    assert(!is_monolithic() || sym_vars->get_aux_variables_in_support(transitions[0].get_exists_bw_vars()).IsOne());
}

void ConjunctiveTransitionRelation::merge_transitions(int max_time, int max_nodes) {
    if (variable_based_splitting) {
        variable_based_transition_merging(max_time, max_nodes);
    } else {
        effect_based_transition_merging(max_time, max_nodes);
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
