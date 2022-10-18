#ifndef SYMBOLIC_PLAN_RECONSTRUCTION_SIMPLE_SYM_SOLUTION_REGISTRY_H
#define SYMBOLIC_PLAN_RECONSTRUCTION_SIMPLE_SYM_SOLUTION_REGISTRY_H

#include "sym_solution_registry.h"
#include "reconstruction_node.h"

#include "../plan_selection/plan_database.h"
#include "../sym_variables.h"
#include "../transition_relation.h"

#include "../../plan_manager.h"
#include "../../state_registry.h"
#include "../../task_proxy.h"

namespace symbolic {
class SimpleSymSolutionRegistry : public SymSolutionRegistry {
protected:

    // We would like to use the prio queue implemented in FD but it requires 
    // integer values as prio and we have a more complex comparision
    std::priority_queue<ReconstructionNode, std::vector<ReconstructionNode>, CompareReconstructionNodes> queue;

    void add_plan(const Plan &plan) const override;

    virtual void reconstruct_plans(const SymSolutionCut &sym_cut) override;

    void expand_non_zero_cost_actions(const ReconstructionNode &node);

    bool swap_to_bwd_phase(const ReconstructionNode &node) const;

    bool is_solution(const ReconstructionNode &node) const;

public:
    SimpleSymSolutionRegistry() : SymSolutionRegistry() {}
    ~SimpleSymSolutionRegistry() = default;
};
} // namespace symbolic

#endif
