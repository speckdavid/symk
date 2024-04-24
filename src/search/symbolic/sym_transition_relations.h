#ifndef SYMBOLIC_SYM_TRANSITION_RELATIONS_H
#define SYMBOLIC_SYM_TRANSITION_RELATIONS_H

#include <algorithm>

#include "sym_mutexes.h"
#include "sym_parameters.h"
#include "sym_variables.h"

#include "transition_relations/conjunctive_transition_relation.h"
#include "transition_relations/disjunctive_transition_relation.h"
#include "transition_relations/transition_relation.h"

namespace extra_tasks {
class SdacTask;
}

namespace symbolic {
class SymTransitionRelations {
    SymVariables *sym_vars;
    const SymParameters &sym_params;

    std::map<int, std::vector<DisjunctiveTransitionRelation>> individual_transitions; // individual TRs (for plan reconstruction)
    std::map<int, std::vector<DisjunctiveTransitionRelation>> transitions; // Merged TRs
    int min_transition_cost; // minimum cost of non-zero cost transitions

    void init_individual_transitions(const std::shared_ptr<AbstractTask> &task, const SymMutexes &sym_mutexes);
    void create_single_trs(const std::shared_ptr<AbstractTask> &task, const SymMutexes &sym_mutexes);
    void create_single_sdac_trs(const std::shared_ptr<extra_tasks::SdacTask> &sdac_task, bool fast_creation);
    void create_merged_transitions();

public:
    SymTransitionRelations(SymVariables *sym_vars, const SymParameters &sym_params);
    void init(const std::shared_ptr<AbstractTask> &task, const SymMutexes &sym_mutexes);

    int get_min_transition_cost() const;
    bool has_zero_cost_transition() const;

    const std::map<int, std::vector<DisjunctiveTransitionRelation>> &get_transition_relations() const;
    const std::map<int, std::vector<DisjunctiveTransitionRelation>> &get_individual_transition_relations() const;
};
}

#endif
