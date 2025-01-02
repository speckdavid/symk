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
    bool dynamic_splitting; // Indicates if we try variable-based with a backup of effect-based splitting
    bool variable_based_splitting; // Determines if we use auxillary variables and full partioning
    bool early_quantification; // Early quantification of processed variables

    std::vector<DisjunctiveTransitionRelation> transitions;
    std::vector<std::vector<DisjunctiveTransitionRelation>> var_based_effect_transitions;

    BDD all_exists_vars, all_exists_bw_vars; // Cube with variables to existentialize of all trs
    std::vector<BDD> exists_vars, exists_bw_vars; // Cubes per transitons relation for existentialize
    std::vector<BDD> all_swap_vars, all_swap_vars_p; // Swap variables from unprimed to primed

    void reset();

    void init_dynamically();
    void init_variable_based();
    void init_effect_based();

    int next_aux_var_id;
    std::map<std::set<FactPair>, int> eff_condition_to_aux_var_id;
    int get_aux_var_id(const ConditionsProxy &cond);
    bool is_simple_condition(const ConditionsProxy &cond);
    void add_precondition_trs(const PreconditionsProxy &pre, std::vector<DisjunctiveTransitionRelation> &transition);
    void add_aux_condition_trs(const EffectProxy &eff, int aux_var_id, std::vector<DisjunctiveTransitionRelation> &transitions);
    void add_effect_trs(const EffectProxy &eff, std::vector<DisjunctiveTransitionRelation> &transitions);
    void add_effect_trs(const EffectProxy &eff, int aux_var_id, std::vector<DisjunctiveTransitionRelation> &transitions);
    void add_frame_trs(const FactProxy &eff_var,
                       const std::unordered_set<int> &condition_aux_var_ids,
                       const std::unordered_set<FactPair> &single_effect_conditions,
                       std::vector<DisjunctiveTransitionRelation> &transitions);

    void init_exist_and_swap_vars();

    void prune_unused_exist_vars(std::vector<DisjunctiveTransitionRelation> &transitions);
    void set_early_exists_vars();

    void variable_based_transition_merging(int max_time, int max_nodes);
    void effect_based_transition_merging(int max_time, int max_nodes);

public:
    ConjunctiveTransitionRelation(SymVariables *sym_vars,
                                  OperatorID op_id,
                                  const std::shared_ptr<extra_tasks::EffectAggregatedTask> &task,
                                  const ConditionalEffectsTransitionType &type);
    void init();

    BDD image(const BDD &from, int max_nodes = 0U) const override;
    BDD preimage(const BDD &from, int max_nodes = 0U) const override;
    BDD preimage(const BDD &from, const BDD &constraint_to, int max_nodes = 0U) const override;

    virtual int nodeCount() const override;

    const OperatorID &get_unique_operator_id() const override;

    void merge_transitions(int max_time, int max_nodes);

    const std::vector<DisjunctiveTransitionRelation> &get_transitions() const;

    int size() const override;
    bool is_monolithic() const;
};
}
#endif
