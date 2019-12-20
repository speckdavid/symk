#ifndef SYMBOLIC_SYM_SOLUTION_REGISTRY_H
#define SYMBOLIC_SYM_SOLUTION_REGISTRY_H

#include "../plan_manager.h"
#include "../state_registry.h"
#include "../task_proxy.h"
#include "plan_selection/plan_database.h"
#include "sym_solution_cut.h"
#include "sym_variables.h"

namespace symbolic {
class SymPlanReconstructor;
class UnidirectionalSearch;

class SymSolutionRegistry {
protected:
  std::shared_ptr<PlanDataBase> plan_data_base;
  std::shared_ptr<SymPlanReconstructor> plan_reconstructor;
  std::vector<SymSolutionCut> sym_cuts; // sorted in ascending order!

  std::shared_ptr<SymVariables> sym_vars;
  int plan_cost_bound;

public:
  SymSolutionRegistry();

  void init(std::shared_ptr<SymVariables> sym_vars,
            UnidirectionalSearch *fwd_search, UnidirectionalSearch *bwd_search,
            std::shared_ptr<PlanDataBase> plan_data_base);

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
};
} // namespace symbolic

#endif
