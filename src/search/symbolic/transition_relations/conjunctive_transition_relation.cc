#include "conjunctive_transition_relation.h"

#include "../../tasks/effect_aggregated_task.h"

#include <numeric>
#include <set>

using namespace std;

namespace symbolic {
ConjunctiveTransitionRelation::ConjunctiveTransitionRelation(SymVariables *sym_vars, OperatorID op_id,
                                                             const shared_ptr<extra_tasks::EffectAggregatedTask> &task)
    : TransitionRelation(),
      sym_vars(sym_vars),
      task(task),
      operator_id(op_id),
      exists_vars(sym_vars->oneBDD()),
      exists_bw_vars(sym_vars->oneBDD()) {}

void ConjunctiveTransitionRelation::init() {
    TaskProxy task_proxy(*task);
    for (int ce_op_id : task->get_operators_beloning_to_parent(operator_id.get_index())) {
        transitions.emplace_back(sym_vars, OperatorID(ce_op_id), task);
        transitions.back().init();
        transitions.back().setOpsIds({operator_id});
        exists_vars *= transitions.back().get_exists_vars();
        exists_bw_vars *= transitions.back().get_exists_bw_vars();
        swap_vars.insert(swap_vars.end(), transitions.back().get_swap_vars().begin(), transitions.back().get_swap_vars().end());
        swap_vars_p.insert(swap_vars_p.end(), transitions.back().get_swap_vars_p().begin(), transitions.back().get_swap_vars_p().end());
    }

    // remove duplicates from swap_vars and swap_vars_p
    sort(swap_vars.begin(), swap_vars.end());
    swap_vars.erase(std::unique(swap_vars.begin(), swap_vars.end()), swap_vars.end());
    sort(swap_vars_p.begin(), swap_vars_p.end());
    swap_vars_p.erase(std::unique(swap_vars_p.begin(), swap_vars_p.end()), swap_vars_p.end());
}

BDD ConjunctiveTransitionRelation::image(const BDD &from) const {
    BDD aux = from;
    BDD res = sym_vars->oneBDD();
    for (auto const &tr : transitions) {
        res = from.And(aux);
    }
    // Optimize!
    for (auto const &tr : transitions) {
        res = from.ExistAbstract(tr.get_exists_vars());
    }
    for (auto const &tr : transitions) {
        res = from.SwapVariables(tr.get_swap_vars(), tr.get_swap_vars_p());
    }
    return res;
}

BDD ConjunctiveTransitionRelation::preimage(const BDD &from) const {
    utils::exit_with(utils::ExitCode::SEARCH_CRITICAL_ERROR);
    return sym_vars->zeroBDD();
}

BDD ConjunctiveTransitionRelation::image(const BDD &from, int maxNodes) const {
    BDD aux = from;
    BDD res = sym_vars->oneBDD();
    for (auto const &tr : transitions) {
        res = from.And(aux, maxNodes);
    }
    // Optimize!
    for (auto const &tr : transitions) {
        res = from.ExistAbstract(tr.get_exists_vars(), maxNodes);
    }
    for (auto const &tr : transitions) {
        res = from.SwapVariables(tr.get_swap_vars(), tr.get_swap_vars_p());
    }
    return res;
}

BDD ConjunctiveTransitionRelation::preimage(const BDD &from, int maxNodes) const {
    utils::exit_with(utils::ExitCode::SEARCH_CRITICAL_ERROR);
    return sym_vars->zeroBDD();
}

int ConjunctiveTransitionRelation::nodeCount() const {
    return std::accumulate(transitions.begin(), transitions.end(), 0, [](int acc, const auto &tr) {return acc + tr.nodeCount();});
}

int ConjunctiveTransitionRelation::size() const {
    return transitions.size();
}
}
