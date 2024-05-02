#ifndef SYMBOLIC_TRANSITION_RELATIONS_CONJUNCTIVE_TRANSITION_RELATION_H
#define SYMBOLIC_TRANSITION_RELATIONS_CONJUNCTIVE_TRANSITION_RELATION_H

#include "transition_relation.h"

#include "disjunctive_transition_relation.h"

#include "../sym_enums.h"
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
class ConjunctiveTransitionRelation : public TransitionRelation {
protected:
    SymVariables *sym_vars; // To call basic BDD creation methods
    const std::shared_ptr<extra_tasks::EffectAggregatedTask> &task; // Use task_proxy to access task information.
    OperatorID operator_id; // Single operator with multiple conditional effects
    bool early_quantification;

    std::vector<DisjunctiveTransitionRelation> transitions;

    BDD all_exists_vars, all_exists_bw_vars; // Cube with variables to existentialize of all trs
    std::vector<BDD> exists_vars, exists_bw_vars; // Cubes per transitons relation for existentialize
    std::vector<BDD> all_swap_vars, all_swap_vars_p; // Swap variables from unprimed to primed

    void init_exist_and_swap_vars();
    void sort_transition_relations();
    void set_early_exists_and_swap_vars();

public:
    ConjunctiveTransitionRelation(SymVariables *sym_vars,
                                  OperatorID op_id,
                                  const std::shared_ptr<extra_tasks::EffectAggregatedTask> &task,
                                  bool early_quantification);
    void init();

    BDD image(const BDD &from) const override;
    BDD preimage(const BDD &from) const override;
    BDD image(const BDD &from, int max_nodes) const override;
    BDD preimage(const BDD &from, int max_nodes) const override;

    virtual int nodeCount() const override;

    const OperatorID &get_unique_operator_id() const override;

    void merge_transitions(int max_nodes, int max_time);

    const std::vector<DisjunctiveTransitionRelation> &get_transitions() const;

    int size() const;
    bool is_monolithic() const;
};
}
#endif
