#ifndef SYMBOLIC_TRANSITION_RELATIONS_CONJUNCTIVE_TRANSITION_RELATION_H
#define SYMBOLIC_TRANSITION_RELATIONS_CONJUNCTIVE_TRANSITION_RELATION_H

#include "transition_relation.h"

#include "disjunctive_transition_relation.h"

#include "../sym_variables.h"

#include "../../task_proxy.h"

#include <vector>

namespace extra_tasks {
class EffectAggregatedTask;
}

namespace symbolic {
/*
 * Represents a conjunctive transition relation (used for conditional effects) with BDDs.
 */
class ConjunctiveTransitionRelation : TransitionRelation {
protected:
    SymVariables *sym_vars; // To call basic BDD creation methods
    const std::shared_ptr<extra_tasks::EffectAggregatedTask> &task; // Use task_proxy to access task information.
    OperatorID operator_id; // Single operator with multiple conditional effects

    std::vector<DisjunctiveTransitionRelation> transitions;

    BDD exists_vars, exists_bw_vars; // Cube with variables to existentialize
    std::vector<BDD> swap_vars, swap_vars_p; // Swap variables from unprimed to primed

public:
    ConjunctiveTransitionRelation(SymVariables *sym_vars, OperatorID op_id, const std::shared_ptr<extra_tasks::EffectAggregatedTask> &task);
    void init();

    BDD image(const BDD &from) const override;
    BDD preimage(const BDD &from) const override;
    BDD image(const BDD &from, int maxNodes) const override;
    BDD preimage(const BDD &from, int maxNodes) const override;

    virtual int nodeCount() const override;

    int size() const;
};
}
#endif
