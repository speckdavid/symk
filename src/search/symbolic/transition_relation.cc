#include "transition_relation.h"

#include "../task_proxy.h"
#include "../utils/logging.h"
#include "../utils/timer.h"
#include "original_state_space.h"
#include "sym_state_space_manager.h"

#include <algorithm>
#include <cassert>

using namespace std;

namespace symbolic {
TransitionRelation::TransitionRelation(
    SymVariables *sVars, OperatorID op_id,
    const shared_ptr<AbstractTask> &task)
    : sym_vars(sVars), task_proxy(*task),
      cost(task_proxy.get_operators()[op_id].get_cost()), tr_bdd(sVars->oneBDD()),
      exist_vars(sVars->oneBDD()), exists_bw_vars(sVars->oneBDD()) {
    ops_ids.insert(op_id);
}

void TransitionRelation::init() {
    OperatorProxy op = task_proxy.get_operators()[ops_ids.begin()->get_index()];

    for (auto const &pre : op.get_preconditions()) {
        FactPair fact = pre.get_pair();
        tr_bdd *= sym_vars->get_axiom_compiliation()->get_primary_representation(
            fact.var, fact.value);
    }

    map<int, BDD> effect_conditions;
    map<int, BDD> effects;

    // Get effects and the remaining conditions.
    for (auto const &eff : op.get_effects()) {
        FactPair eff_fact = eff.get_fact().get_pair();
        int var = eff_fact.var;
        if (find(eff_vars.begin(), eff_vars.end(), var) == eff_vars.end()) {
            eff_vars.push_back(var);
        }

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

    sort(eff_vars.begin(), eff_vars.end());
    for (int var : eff_vars) {
        for (int bdd_var : sym_vars->vars_index_pre(var)) {
            swap_vars.push_back(sym_vars->bddVar(bdd_var));
        }
        for (int bdd_var : sym_vars->vars_index_eff(var)) {
            swap_vars_p.push_back(sym_vars->bddVar(bdd_var));
        }
    }
    assert(swap_vars.size() == swap_vars_p.size());
    // exist_vars/exists_bw_vars is just the conjunction of swap_vars and swap_vars_p
    for (size_t i = 0; i < swap_vars.size(); ++i) {
        exist_vars *= swap_vars[i];
        exists_bw_vars *= swap_vars_p[i];
    }
}

void TransitionRelation::init_from_tr(const TransitionRelation &other) {
    tr_bdd = other.get_tr_BDD();
    ops_ids = other.get_operator_ids();
    cost = other.get_cost();
    eff_vars = other.get_eff_vars();
    exist_vars = other.get_exists_vars();
    exists_bw_vars = other.get_exist_bw_vars();
    swap_vars = other.get_swap_vars();
    swap_vars_p = other.get_swap_vars_p();
}

void TransitionRelation::add_condition(BDD cond) {
    tr_bdd *= cond;
}

BDD TransitionRelation::image(const BDD &from) const {
    BDD aux = from;
    BDD tmp = tr_bdd.AndAbstract(aux, exist_vars);
    BDD res = tmp.SwapVariables(swap_vars, swap_vars_p);
    return res;
}

BDD TransitionRelation::image(const BDD &from, int maxNodes) const {
    BDD aux = from;
    BDD tmp = tr_bdd.AndAbstract(aux, exist_vars, maxNodes);
    BDD res = tmp.SwapVariables(swap_vars, swap_vars_p);
    return res;
}

BDD TransitionRelation::preimage(const BDD &from) const {
    BDD tmp = from.SwapVariables(swap_vars, swap_vars_p);
    BDD res = tr_bdd.AndAbstract(tmp, exists_bw_vars);
    return res;
}

BDD TransitionRelation::preimage(const BDD &from, int maxNodes) const {
    BDD tmp = from.SwapVariables(swap_vars, swap_vars_p);
    BDD res = tr_bdd.AndAbstract(tmp, exists_bw_vars, maxNodes);
    return res;
}

void TransitionRelation::merge(const TransitionRelation &t2, int maxNodes) {
    assert(cost == t2.cost);
    if (cost != t2.cost) {
        cerr << "Error: merging transitions with different cost: " << cost << " "
             << t2.cost << endl;
        utils::exit_with(utils::ExitCode::SEARCH_CRITICAL_ERROR);
    }

    // Attempt to generate the new tBDD
    vector<int> newEffVars;
    set_union(eff_vars.begin(), eff_vars.end(), t2.eff_vars.begin(),
              t2.eff_vars.end(), back_inserter(newEffVars));

    BDD newTBDD = tr_bdd;
    BDD newTBDD2 = t2.tr_bdd;

    vector<int>::const_iterator var1 = eff_vars.begin();
    vector<int>::const_iterator var2 = t2.eff_vars.begin();
    for (vector<int>::const_iterator var = newEffVars.begin();
         var != newEffVars.end(); ++var) {
        if (var1 == eff_vars.end() || *var1 != *var) {
            newTBDD *= sym_vars->biimp(*var);
        } else {
            ++var1;
        }

        if (var2 == t2.eff_vars.end() || *var2 != *var) {
            newTBDD2 *= sym_vars->biimp(*var);
        } else {
            ++var2;
        }
    }
    newTBDD = newTBDD.Or(newTBDD2, maxNodes);

    if (newTBDD.nodeCount() > maxNodes) {
        throw BDDError(); // We could not sucessfully merge
    }

    tr_bdd = newTBDD;

    eff_vars.swap(newEffVars);
    exist_vars *= t2.exist_vars;
    exists_bw_vars *= t2.exists_bw_vars;

    for (size_t i = 0; i < t2.swap_vars.size(); i++) {
        if (find(swap_vars.begin(), swap_vars.end(), t2.swap_vars[i]) ==
            swap_vars.end()) {
            swap_vars.push_back(t2.swap_vars[i]);
            swap_vars_p.push_back(t2.swap_vars_p[i]);
        }
    }

    ops_ids.insert(t2.ops_ids.begin(), t2.ops_ids.end());
}

// For each op, include relevant mutexes
void TransitionRelation::edeletion(
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

const OperatorID &TransitionRelation::get_unique_operator_id() const {
    assert(ops_ids.size() == 1);
    return *(get_operator_ids().begin());
}
}
