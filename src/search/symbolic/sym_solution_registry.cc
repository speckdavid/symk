#include "sym_solution_registry.h"
#include "../tasks/root_task.h"
#include "sym_plan_reconstruction.h"

namespace symbolic {

SymSolutionRegistry::SymSolutionRegistry()
    : plan_data_base(nullptr), plan_reconstructor(nullptr), sym_vars(nullptr),
      plan_cost_bound(-1) {}

void SymSolutionRegistry::init(std::shared_ptr<SymVariables> sym_vars,
                               UniformCostSearch *fwd_search,
                               UniformCostSearch *bwd_search,
                               std::shared_ptr<PlanDataBase> plan_data_base) {
  this->sym_vars = sym_vars;
  this->plan_data_base = plan_data_base;
  this->plan_reconstructor = std::make_shared<SymPlanReconstructor>(
      fwd_search, bwd_search, sym_vars, plan_data_base);
}

void SymSolutionRegistry::register_solution(const SymSolutionCut &solution) {

  // std::cout << "\nregister " << new_cut << std::endl;

  bool merged = false;
  size_t pos = 0;
  for (; pos < sym_cuts.size(); pos++) {
    // a cut with same g and h values exist
    // => we combine the cut to avoid multiple cuts with same solutions
    if (sym_cuts[pos] == solution) {
      sym_cuts[pos].merge(solution);
      merged = true;
      break;
    }
    if (sym_cuts[pos] > solution) {
      break;
    }
  }
  if (!merged) {
    sym_cuts.insert(sym_cuts.begin() + pos, solution);
  }
}

void SymSolutionRegistry::construct_cheaper_solutions(int bound) {
  bool bound_used = false;
  int min_plan_bound = std::numeric_limits<int>::max();

  while (sym_cuts.size() > 0 && sym_cuts.at(0).get_f() < bound &&
         !found_all_plans()) {

    // Ignore cuts with costs smaller than the proven cost bound
    // This occurs only in bidirectional search
    if (sym_cuts.at(0).get_f() < plan_cost_bound) {
      sym_cuts.erase(sym_cuts.begin());
    } else {
      min_plan_bound = std::min(min_plan_bound, sym_cuts.at(0).get_f());
      bound_used = true;
      plan_reconstructor->reconstruct_plans(sym_cuts[0]);
      sym_cuts.erase(sym_cuts.begin());
    }
  }

  // Update the plan bound
  if (bound_used) {
    plan_cost_bound = min_plan_bound;
  }
}
} // namespace symbolic
