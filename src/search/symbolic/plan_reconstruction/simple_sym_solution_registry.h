#ifndef SYMBOLIC_PLAN_RECONSTRUCTION_SIMPLE_SYM_SOLUTION_REGISTRY_H
#define SYMBOLIC_PLAN_RECONSTRUCTION_SIMPLE_SYM_SOLUTION_REGISTRY_H

#include "../../plan_manager.h"
#include "../../state_registry.h"
#include "../../task_proxy.h"
#include "../plan_selection/plan_database.h"
#include "../sym_variables.h"
#include "../transition_relation.h"
#include "simple_sym_solution_cut.h"
#include "sym_solution_registry.h"

using namespace std;

namespace symbolic {
class SimpleSymSolutionRegistry : public SymSolutionRegistry {
protected:
    BDD get_visited_states(const Plan &plan) const;

    virtual void add_plan(const Plan &plan) const override;

    virtual void reconstruct_plans(const SymSolutionCut &cut) override;

    void extract_all_plans(SimpleSymSolutionCut &simple_cut, bool fw, Plan plan);

    void extract_all_cost_plans(SimpleSymSolutionCut &simple_cut, bool fw, Plan &plan);
    void extract_all_zero_plans(SimpleSymSolutionCut &sym_cut, bool fw, Plan &plan);

    void reconstruct_cost_action(SimpleSymSolutionCut &simple_cut, bool fw,
                                 std::shared_ptr<ClosedList> closed,
                                 const Plan &plan);
    void reconstruct_zero_action(SimpleSymSolutionCut &sym_cut, bool fw,
                                 std::shared_ptr<ClosedList> closed,
                                 const Plan &plan);
    void extract_one_by_one(BDD states, BDD &visited,
                            SimpleSymSolutionCut &simple_cut, Plan &plan);

public:
    SimpleSymSolutionRegistry() : SymSolutionRegistry() {}
    ~SimpleSymSolutionRegistry() = default;
};
} // namespace symbolic

#endif
