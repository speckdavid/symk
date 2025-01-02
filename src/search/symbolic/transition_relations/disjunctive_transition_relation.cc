#include "disjunctive_transition_relation.h"

#include "../sym_state_space_manager.h"

#include "../../utils/logging.h"
#include "../../utils/timer.h"


#include <algorithm>
#include <cassert>

using namespace std;

namespace symbolic {
DisjunctiveTransitionRelation::DisjunctiveTransitionRelation(SymVariables *sym_vars, OperatorID op_id, const shared_ptr<AbstractTask> &task)
    : TransitionRelation(),
      sym_vars(sym_vars),
      task_proxy(*task),
      cost(task_proxy.get_operators()[op_id].get_cost()),
      tr_bdd(sym_vars->oneBDD()),
      exists_vars(sym_vars->oneBDD()),
      exists_bw_vars(sym_vars->oneBDD()) {
    ops_ids.insert(op_id);
}

DisjunctiveTransitionRelation::DisjunctiveTransitionRelation(SymVariables *sym_vars, const shared_ptr<AbstractTask> &task)
    : TransitionRelation(),
      sym_vars(sym_vars),
      task_proxy(*task),
      cost(-1),
      tr_bdd(sym_vars->oneBDD()),
      exists_vars(sym_vars->oneBDD()),
      exists_bw_vars(sym_vars->oneBDD()) {
}

void DisjunctiveTransitionRelation::init() {
    OperatorProxy op = task_proxy.get_operators()[ops_ids.begin()->get_index()];

    add_condition(op.get_preconditions());

    map<int, BDD> effect_conditions;
    map<int, BDD> effects;

    // Get effects and the remaining conditions.
    for (auto const &eff : op.get_effects()) {
        FactPair eff_fact = eff.get_fact().get_pair();
        int var = eff_fact.var;
        eff_vars.insert(var);

        BDD condition = sym_vars->oneBDD();
        BDD ppBDD = sym_vars->effBDD(var, eff_fact.value);
        if (effect_conditions.count(var)) {
            condition = effect_conditions.at(var);
        } else {
            effect_conditions[var] = condition;
            effects[var] = sym_vars->zeroBDD();
        }

        for (const auto &cPrev : eff.get_conditions()) {
            FactPair cPrev_cond = cPrev.get_pair();
            condition *= sym_vars->get_axiom_compiliation()->get_primary_representation(
                cPrev_cond.var, cPrev_cond.value);
        }
        effect_conditions[var] *= !condition;
        effects[var] += (condition * ppBDD);
    }

    // Add effects to the tr_bdd
    for (auto it = effects.rbegin(); it != effects.rend(); ++it) {
        int var = it->first;
        BDD effectBDD = it->second;
        // If some possibility is not covered by the conditions of the
        // conditional effect, then in those cases the value of the value
        // is preserved with a biimplication
        if (!effect_conditions[var].IsZero()) {
            effectBDD += (effect_conditions[var] * sym_vars->biimp(var));
        }
        tr_bdd *= effectBDD;
    }
    if (tr_bdd.IsZero()) {
        utils::g_log << "Operator is empty: " << op.get_name() << endl;
    }

    for (int var : eff_vars) {
        add_exist_var(var);
        add_swap_var(var);
    }
}

void DisjunctiveTransitionRelation::init_from_tr(const DisjunctiveTransitionRelation &other) {
    tr_bdd = other.get_tr_BDD();
    ops_ids = other.get_operator_ids();
    cost = other.get_cost();
    eff_vars = other.get_eff_vars();
    exists_vars = other.get_exists_vars();
    exists_bw_vars = other.get_exists_bw_vars();
    swap_vars = other.get_swap_vars();
    swap_vars_p = other.get_swap_vars_p();
}

void DisjunctiveTransitionRelation::add_condition(const ConditionsProxy &conditions) {
    for (FactProxy cond : conditions) {
        FactPair fact = cond.get_pair();
        tr_bdd *= sym_vars->get_axiom_compiliation()->get_primary_representation(
            fact.var, fact.value);
    }
}

void DisjunctiveTransitionRelation::add_condition(BDD condition) {
    tr_bdd *= condition;
}

void DisjunctiveTransitionRelation::add_effect(BDD effect) {
    tr_bdd *= effect;
}

void DisjunctiveTransitionRelation::add_exist_var(int var) {
    for (int bdd_var : sym_vars->vars_index_pre(var)) {
        exists_vars *= sym_vars->bddVar(bdd_var);
    }
    for (int bdd_var : sym_vars->vars_index_eff(var)) {
        exists_bw_vars *= sym_vars->bddVar(bdd_var);
    }
    assert(exists_vars.IsCube());
    assert(exists_bw_vars.IsCube());
}

void DisjunctiveTransitionRelation::add_exist_var(BDD cube) {
    assert(cube.IsCube());
    exists_vars *= cube;
    exists_bw_vars *= cube;
}

void DisjunctiveTransitionRelation::add_eff_var(int var) {
    assert(var >= 0 && var < (int)task_proxy.get_variables().size());
    eff_vars.insert(var);
}

void DisjunctiveTransitionRelation::remove_exist_var(int var) {
    BDD cube = sym_vars->oneBDD();
    for (int bdd_var : sym_vars->vars_index_pre(var)) {
        cube *= sym_vars->bddVar(bdd_var);
    }
    exists_vars = exists_vars.ExistAbstract(cube);

    cube = sym_vars->oneBDD();
    for (int bdd_var : sym_vars->vars_index_eff(var)) {
        cube *= sym_vars->bddVar(bdd_var);
    }
    exists_bw_vars = exists_bw_vars.ExistAbstract(cube);
    assert(exists_vars.IsCube());
    assert(exists_bw_vars.IsCube());
}

void DisjunctiveTransitionRelation::remove_exist_var(BDD cube) {
    assert(cube.IsCube());
    exists_vars = exists_vars.ExistAbstract(cube);
    exists_bw_vars = exists_bw_vars.ExistAbstract(cube);
}

void DisjunctiveTransitionRelation::add_swap_var(int var) {
    for (int bdd_var : sym_vars->vars_index_pre(var)) {
        swap_vars.push_back(sym_vars->bddVar(bdd_var));
    }
    for (int bdd_var : sym_vars->vars_index_eff(var)) {
        swap_vars_p.push_back(sym_vars->bddVar(bdd_var));
    }
    assert(swap_vars.size() == swap_vars_p.size());
}

void DisjunctiveTransitionRelation::set_tr_BDD(BDD tr_bdd) {
    this->tr_bdd = tr_bdd;
}

BDD DisjunctiveTransitionRelation::image(const BDD &from, int maxNodes) const {
    BDD aux = from;
    BDD tmp = tr_bdd.AndAbstract(aux, exists_vars, maxNodes);
    BDD res = tmp.SwapVariables(swap_vars, swap_vars_p);
    assert(!sym_vars->has_aux_variables_in_support(res));
    return res;
}

BDD DisjunctiveTransitionRelation::preimage(const BDD &from, int maxNodes) const {
    BDD tmp = from.SwapVariables(swap_vars, swap_vars_p);
    BDD res = tr_bdd.AndAbstract(tmp, exists_bw_vars, maxNodes);
    assert(!sym_vars->has_aux_variables_in_support(res));
    return res;
}

BDD DisjunctiveTransitionRelation::preimage(const BDD &from, const BDD &constraint_to, int maxNodes) const {
    BDD tmp = from.SwapVariables(swap_vars, swap_vars_p);
    tmp *= constraint_to;
    BDD res = tr_bdd.AndAbstract(tmp, exists_bw_vars, maxNodes);
    assert(!sym_vars->has_aux_variables_in_support(res));
    return res;
}

int DisjunctiveTransitionRelation::nodeCount() const {
    return tr_bdd.nodeCount();
}

void DisjunctiveTransitionRelation::disjunctive_merge(const DisjunctiveTransitionRelation &t2, int maxNodes) {
    assert(cost == t2.cost);

    BDD new_t_bdd = tr_bdd;
    BDD new_t_bdd2 = t2.tr_bdd;

    unordered_set<int> new_eff_vars;
    new_eff_vars.insert(get_eff_vars().begin(), get_eff_vars().end());
    new_eff_vars.insert(t2.get_eff_vars().begin(), t2.get_eff_vars().end());

    for (int var : new_eff_vars) {
        // Check if var is in eff_vars
        if (get_eff_vars().find(var) == get_eff_vars().end()) {
            // var is not in eff_vars
            new_t_bdd *= sym_vars->biimp(var);
        }

        // Check if var is in t2.eff_vars
        if (t2.get_eff_vars().find(var) == t2.get_eff_vars().end()) {
            // var is not in t2_eff_vars
            new_t_bdd2 *= sym_vars->biimp(var);
        }
    }

    new_t_bdd = new_t_bdd.Or(new_t_bdd2, maxNodes);

    if (new_t_bdd.nodeCount() > maxNodes) {
        throw BDDError(); // We could not sucessfully merge
    }

    tr_bdd = new_t_bdd;

    eff_vars.swap(new_eff_vars);
    exists_vars *= t2.exists_vars;
    exists_bw_vars *= t2.exists_bw_vars;

    for (size_t i = 0; i < t2.swap_vars.size(); ++i) {
        if (find(swap_vars.begin(), swap_vars.end(), t2.swap_vars[i]) == swap_vars.end()) {
            swap_vars.push_back(t2.swap_vars[i]);
            swap_vars_p.push_back(t2.swap_vars_p[i]);
        }
    }

    ops_ids.insert(t2.ops_ids.begin(), t2.ops_ids.end());
}

void DisjunctiveTransitionRelation::conjunctive_merge(const DisjunctiveTransitionRelation &t2, int max_nodes) {
    assert(cost == t2.cost);
    assert(get_unique_operator_id() == t2.get_unique_operator_id());

    // Attempt to generate the new tBDD
    BDD new_t_bdd = tr_bdd;
    BDD new_t_bdd2 = t2.tr_bdd;

    new_t_bdd = new_t_bdd.And(new_t_bdd2, max_nodes);

    if (new_t_bdd.nodeCount() > max_nodes) {
        throw BDDError(); // We could not sucessfully merge
    }

    tr_bdd = new_t_bdd;
    eff_vars.insert(t2.get_eff_vars().begin(), t2.get_eff_vars().end());
    exists_vars *= t2.exists_vars;
    exists_bw_vars *= t2.exists_bw_vars;

    for (size_t i = 0; i < t2.swap_vars.size(); ++i) {
        if (find(swap_vars.begin(), swap_vars.end(), t2.swap_vars[i]) == swap_vars.end()) {
            swap_vars.push_back(t2.swap_vars[i]);
            swap_vars_p.push_back(t2.swap_vars_p[i]);
        }
    }
}

// For each op, include relevant mutexes
void DisjunctiveTransitionRelation::edeletion(
    const vector<vector<BDD>> &notMutexBDDsByFluentFw,
    const vector<vector<BDD>> &notMutexBDDsByFluentBw,
    const vector<vector<BDD>> &exactlyOneBDDsByFluent) {
    assert(ops_ids.size() == 1);
    assert(notMutexBDDsByFluentFw.size() == task_proxy.get_variables().size());
    assert(notMutexBDDsByFluentBw.size() == task_proxy.get_variables().size());
    assert(exactlyOneBDDsByFluent.size() == task_proxy.get_variables().size());

    // For each op, include relevant mutexes
    for (const OperatorID &op_id : ops_ids) {
        OperatorProxy op = task_proxy.get_operators()[op_id.get_index()];
        for (const auto &eff : op.get_effects()) {
            FactPair pp = eff.get_fact().get_pair();
            // TODO: somehow fix this here
            FactPair pre(-1, -1);
            for (const auto &cond : op.get_preconditions()) {
                if (cond.get_pair().var == pp.var) {
                    pre = cond.get_pair();
                    break;
                }
            }

            // edeletion bw
            if (pre.var == -1) {
                // We have a post effect over this variable.
                // That means that every previous value is possible
                // for each value of the variable
                for (int val = 0;
                     val < task_proxy.get_variables()[pp.var].get_domain_size();
                     val++) {
                    tr_bdd *= notMutexBDDsByFluentBw[pp.var][val];
                }
            } else {
                // In regression, we are making true pp.pre
                // So we must negate everything of these.
                tr_bdd *= notMutexBDDsByFluentBw[pp.var][pre.value];
            }
            // TODO(speckd): Here we need to swap in the correct direction!
            // edeletion fw
            tr_bdd *= notMutexBDDsByFluentFw[pp.var][pp.value].SwapVariables(
                swap_vars, swap_vars_p);

            // edeletion invariants
            tr_bdd *= exactlyOneBDDsByFluent[pp.var][pp.value];
        }
    }
}

const OperatorID &DisjunctiveTransitionRelation::get_unique_operator_id() const {
    assert(ops_ids.size() == 1);
    return *(get_operator_ids().begin());
}
}
