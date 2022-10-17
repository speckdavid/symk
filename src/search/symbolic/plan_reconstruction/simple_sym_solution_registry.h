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

#include "../../algorithms/priority_queues.h"

using namespace std;

namespace symbolic {

class SimpleSymSolutionRegistry : public SymSolutionRegistry {
protected:

    priority_queues::AdaptiveQueue<ReconstructionNode> queue;

    void add_plan(const Plan &plan) const override;

    virtual void reconstruct_plans(const SymSolutionCut &sym_cut) override;

    void expand_cost_actions(const ReconstructionNode &node);

public:
    SimpleSymSolutionRegistry() : SymSolutionRegistry() {}
    ~SimpleSymSolutionRegistry() = default;
};
} // namespace symbolic

#endif
