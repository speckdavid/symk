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
    : sV(sVars), task_proxy(*task),
      cost(task_proxy.get_operators()[op_id].get_cost()), tBDD(sVars->oneBDD()),
      existsVars(sVars->oneBDD()), existsBwVars(sVars->oneBDD()) {
    ops_ids.insert(op_id);
}

void TransitionRelation::init() {
    OperatorProxy op = task_proxy.get_operators()[ops_ids.begin()->get_index()];

    for (auto const &pre : op.get_preconditions()) { // Put precondition of label
        FactPair fact = pre.get_pair();
        tBDD *= sV->get_axiom_compiliation()->get_primary_representation(
            fact.var, fact.value);
    }

    map<int, BDD> effect_conditions;
    map<int, BDD> effects;

    // Get effects and the remaining conditions.
    for (auto const &eff : op.get_effects()) {
        FactPair eff_fact = eff.get_fact().get_pair();
        int var = eff_fact.var;
        if (find(effVars.begin(), effVars.end(), var) == effVars.end()) {
            effVars.push_back(var);
        }

        BDD condition = sV->oneBDD();
        BDD ppBDD = sV->effBDD(var, eff_fact.value);
        if (effect_conditions.count(var)) {
            condition = effect_conditions.at(var);
        } else {
            effect_conditions[var] = condition;
            effects[var] = sV->zeroBDD();
        }

        for (const auto &cPrev : eff.get_conditions()) {
            FactPair cPrev_cond = cPrev.get_pair();
            condition *= sV->get_axiom_compiliation()->get_primary_representation(
                cPrev_cond.var, cPrev_cond.value);
        }
        effect_conditions[var] *= !condition;
        effects[var] += (condition * ppBDD);
    }

    // Add effects to the tBDD
    int counter = 0;
    for (auto it = effects.rbegin(); it != effects.rend(); ++it) {
        int var = it->first;
        BDD effectBDD = it->second;
        // If some possibility is not covered by the conditions of the
        // conditional effect, then in those cases the value of the value
        // is preserved with a biimplication
        if (!effect_conditions[var].IsZero()) {
            effectBDD += (effect_conditions[var] * sV->biimp(var));
        }
        tBDD *= effectBDD;
        counter++;
    }
    if (tBDD.IsZero()) {
        utils::g_log << "Operator is empty: " << op.get_name() << endl;
    }

    sort(effVars.begin(), effVars.end());
    for (int var : effVars) {
        for (int bdd_var : sV->vars_index_pre(var)) {
            swapVarsS.push_back(sV->bddVar(bdd_var));
        }
        for (int bdd_var : sV->vars_index_eff(var)) {
            swapVarsSp.push_back(sV->bddVar(bdd_var));
        }
    }
    assert(swapVarsS.size() == swapVarsSp.size());
    // existsVars/existsBwVars is just the conjunction of swapVarsS and swapVarsSp
    for (size_t i = 0; i < swapVarsS.size(); ++i) {
        existsVars *= swapVarsS[i];
        existsBwVars *= swapVarsSp[i];
    }
}

void TransitionRelation::init_from_tr(const TransitionRelation &other) {
    tBDD = other.getTrBDD();
    ops_ids = other.getOpsIds();
    cost = other.getCost();
    effVars = other.getEffVars();
    existsVars = other.getExistsVars();
    existsBwVars = other.getExistBwVars();
    swapVarsS = other.getSwapVars();
    swapVarsSp = other.getSwapVarsP();
}

void TransitionRelation::add_condition(BDD cond) {
    tBDD *= cond;
}

BDD TransitionRelation::image(const BDD &from) const {
    BDD aux = from;
    BDD tmp = tBDD.AndAbstract(aux, existsVars);
    BDD res = tmp.SwapVariables(swapVarsS, swapVarsSp);
    return res;
}

BDD TransitionRelation::image(const BDD &from, int maxNodes) const {
    BDD aux = from;
    BDD tmp = tBDD.AndAbstract(aux, existsVars, maxNodes);
    BDD res = tmp.SwapVariables(swapVarsS, swapVarsSp);
    return res;
}

BDD TransitionRelation::preimage(const BDD &from) const {
    BDD tmp = from.SwapVariables(swapVarsS, swapVarsSp);
    BDD res = tBDD.AndAbstract(tmp, existsBwVars);
    return res;
}

BDD TransitionRelation::preimage(const BDD &from, int maxNodes) const {
    BDD tmp = from.SwapVariables(swapVarsS, swapVarsSp);
    BDD res = tBDD.AndAbstract(tmp, existsBwVars, maxNodes);
    return res;
}

void TransitionRelation::merge(const TransitionRelation &t2, int maxNodes) {
    assert(cost == t2.cost);
    if (cost != t2.cost) {
        cerr << "Error: merging transitions with different cost: " << cost << " "
             << t2.cost << endl;
        utils::exit_with(utils::ExitCode::SEARCH_CRITICAL_ERROR);
    }

    //  utils::g_log << "set_union" << endl;
    // Attempt to generate the new tBDD
    vector<int> newEffVars;
    set_union(effVars.begin(), effVars.end(), t2.effVars.begin(),
              t2.effVars.end(), back_inserter(newEffVars));

    BDD newTBDD = tBDD;
    BDD newTBDD2 = t2.tBDD;

    //    utils::g_log << "Eff vars" << endl;
    vector<int>::const_iterator var1 = effVars.begin();
    vector<int>::const_iterator var2 = t2.effVars.begin();
    for (vector<int>::const_iterator var = newEffVars.begin();
         var != newEffVars.end(); ++var) {
        if (var1 == effVars.end() || *var1 != *var) {
            newTBDD *= sV->biimp(*var);
        } else {
            ++var1;
        }

        if (var2 == t2.effVars.end() || *var2 != *var) {
            newTBDD2 *= sV->biimp(*var);
        } else {
            ++var2;
        }
    }
    newTBDD = newTBDD.Or(newTBDD2, maxNodes);

    if (newTBDD.nodeCount() > maxNodes) {
        throw BDDError(); // We could not sucessfully merge
    }

    tBDD = newTBDD;

    effVars.swap(newEffVars);
    existsVars *= t2.existsVars;
    existsBwVars *= t2.existsBwVars;

    for (size_t i = 0; i < t2.swapVarsS.size(); i++) {
        if (find(swapVarsS.begin(), swapVarsS.end(), t2.swapVarsS[i]) ==
            swapVarsS.end()) {
            swapVarsS.push_back(t2.swapVarsS[i]);
            swapVarsSp.push_back(t2.swapVarsSp[i]);
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
                    tBDD *= notMutexBDDsByFluentBw[pp.var][val];
                }
            } else {
                // In regression, we are making true pp.pre
                // So we must negate everything of these.
                tBDD *= notMutexBDDsByFluentBw[pp.var][pre.value];
            }
            // TODO(speckd): Here we need to swap in the correct direction!
            // edeletion fw
            tBDD *= notMutexBDDsByFluentFw[pp.var][pp.value].SwapVariables(
                swapVarsS, swapVarsSp);

            // edeletion invariants
            tBDD *= exactlyOneBDDsByFluent[pp.var][pp.value];
        }
    }
}
}
