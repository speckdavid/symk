#ifndef SYMBOLIC_PLAN_RECONSTRUCTION_SYM_SOLUTION_REGISTRY_H
#define SYMBOLIC_PLAN_RECONSTRUCTION_SYM_SOLUTION_REGISTRY_H

#include "sym_solution_cut.h"
#include "../plan_selection/plan_database.h"
#include "../sym_variables.h"
#include "../transition_relation.h"
#include "../../plan_manager.h"
#include "../../state_registry.h"
#include "../../task_proxy.h"

namespace symbolic {
class UniformCostSearch;
class ClosedList;

class SymSolutionRegistry {
protected:
    bool single_solution;

    std::vector<SymSolutionCut> sym_cuts; // sorted in ascending order!

    std::shared_ptr<SymVariables> sym_vars;
    UniformCostSearch *fw_search;
    UniformCostSearch *bw_search;
    std::shared_ptr<PlanDataBase> plan_data_base;
    std::map<int, std::vector<TransitionRelation>> trs;
    int plan_cost_bound;

    bool task_has_zero_costs() const {return trs.count(0) > 0;}

    virtual void add_plan(const Plan &plan) const;

    virtual void reconstruct_plans(const SymSolutionCut &cut);

    // Extracts all plans by a DFS, we copy the current plan suffix by every
    // recusive call which is why we don't use any reference for plan
    // BID: After reconstruction of the forward part we reverse the plan and
    // call extract_all_plans in bw direction which completes the plan
    // After completing a plan we store it in found plans!
    void extract_all_plans(SymSolutionCut &sym_cut, bool fw, Plan plan);

    void extract_all_cost_plans(SymSolutionCut &sym_cut, bool fw, Plan &plan);
    void extract_all_zero_plans(SymSolutionCut &sym_cut, bool fw, Plan &plan);

    void reconstruct_zero_action(SymSolutionCut &sym_cut, bool fw,
                                 std::shared_ptr<ClosedList> closed,
                                 const Plan &plan);
    void reconstruct_cost_action(SymSolutionCut &sym_cut, bool fw,
                                 std::shared_ptr<ClosedList> closed,
                                 const Plan &plan);

public:
    SymSolutionRegistry();
    virtual ~SymSolutionRegistry() = default;

    void init(std::shared_ptr<SymVariables> sym_vars,
              UniformCostSearch *fwd_search, UniformCostSearch *bwd_search,
              std::shared_ptr<PlanDataBase> plan_data_base, bool single_solution);

    void register_solution(const SymSolutionCut &solution);
    void construct_cheaper_solutions(int bound);

    bool found_all_plans() const {
        return plan_data_base && plan_data_base->found_enough_plans();
    }

    int get_num_found_plans() const {
        if (plan_data_base == nullptr) {
            return 0;
        }
        return plan_data_base->get_num_accepted_plans();
    }

    BDD get_states_on_goal_paths() const {
        return plan_data_base->get_states_accepted_goal_path();
    }

    double cheapest_solution_cost_found() const {
        double cheapest = std::numeric_limits<double>::infinity();
        if (plan_data_base) {
            cheapest = std::min(cheapest, plan_data_base->get_first_plan_cost());
        }
        if (sym_cuts.size() > 0) {
            cheapest = std::min(cheapest, (double)sym_cuts.at(0).get_f());
        }
        return cheapest;
    }
};
}

#endif
