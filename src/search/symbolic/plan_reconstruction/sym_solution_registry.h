#ifndef SYMBOLIC_PLAN_RECONSTRUCTION_SYM_SOLUTION_REGISTRY_H
#define SYMBOLIC_PLAN_RECONSTRUCTION_SYM_SOLUTION_REGISTRY_H

#include "sym_solution_cut.h"
#include "../plan_selection/plan_selector.h"
#include "../sym_variables.h"
#include "../transition_relation.h"
#include "../../plan_manager.h"
#include "../../state_registry.h"
#include "../../task_proxy.h"

#include "reconstruction_node.h"

namespace symbolic {
// We would like to use the prio queue implemented in FD but it requires
// integer values as prio and we have a more complex comparision
typedef std::priority_queue<ReconstructionNode, std::vector<ReconstructionNode>, CompareReconstructionNodes> ReconstructionQueue;

enum class PlanPruning {
    /* Multiple combinable options to specify which plans are allowed to be part of
    the solution set. The Plan reconstruction will optimize to prune partial plans that
    can not fullfill the contraints. */
    SINGLE_SOLUTION = 0,
    SIMPLE_SOLUTIONS = 1,
    JUSTIFIED = 2
};


class UniformCostSearch;
class ClosedList;

class SymSolutionRegistry {
protected:
    // Pruning techniques
    bool justified_solutions_pruning;
    bool single_solution_pruning;
    bool simple_solutions_pruning;


    std::map<int, std::vector<SymSolutionCut>> sym_cuts;

    std::shared_ptr<SymVariables> sym_vars;
    std::shared_ptr<ClosedList> fw_closed;
    std::shared_ptr<ClosedList> bw_closed;
    std::shared_ptr<PlanSelector> plan_data_base;
    std::map<int, std::vector<TransitionRelation>> trs;

    // We would like to use the prio queue implemented in FD but it requires
    // integer values as prio and we have a more complex comparision
    ReconstructionQueue queue;

    void add_plan(const Plan &plan) const;

    void reconstruct_plans(const std::vector<SymSolutionCut> &sym_cuts);

    void expand_actions(const ReconstructionNode &node);

    bool swap_to_bwd_phase(const ReconstructionNode &node) const;

    bool is_solution(const ReconstructionNode &node) const;

    bool task_has_zero_costs() const {return trs.count(0) > 0;}

    bool justified_solutions() const {return justified_solutions_pruning;}

    bool simple_solutions() const {return simple_solutions_pruning;}

    bool single_solution() const {return single_solution_pruning;}

    bool no_pruning() const {
        return !single_solution() && !justified_solutions() && !simple_solutions();
    }

public:
    SymSolutionRegistry();

    void init(std::shared_ptr<SymVariables> sym_vars,
              std::shared_ptr<symbolic::ClosedList> fw_closed,
              std::shared_ptr<symbolic::ClosedList> bw_closed,
              std::map<int, std::vector<TransitionRelation>> &trs,
              std::shared_ptr<PlanSelector> plan_data_base,
              bool single_solution,
              bool simple_solutions);

    virtual ~SymSolutionRegistry() = default;

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
            cheapest = std::min(cheapest, (double)sym_cuts.begin()->first);
        }
        return cheapest;
    }
};
}

#endif
