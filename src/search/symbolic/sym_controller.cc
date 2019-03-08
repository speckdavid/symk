#include "sym_controller.h"

#include "opt_order.h"

#include "../option_parser.h"
#include "../task_utils/task_properties.h"
#include "debug_macros.h"
#include "sym_state_space_manager.h"

using namespace std;

namespace symbolic {
SymController::SymController(const Options &opts)
    : vars(make_shared<SymVariables>(opts)), mgrParams(opts),
      searchParams(opts), lower_bound(0),
      solution_registry(mgrParams.num_plans) {
    task_properties::verify_no_axioms(
        TaskProxy(*tasks::g_root_task));
    if (searchParams.top_k)
    {
        task_properties::verify_no_zero_operator_cost(
            TaskProxy(*tasks::g_root_task));
  }

  mgrParams.print_options();
  searchParams.print_options();
  vars->init();
  solution_registry.init(vars);
}

void SymController::add_options_to_parser(OptionParser &parser, int maxStepTime,
                                          int maxStepNodes) {
  SymVariables::add_options_to_parser(parser);
  SymParamsMgr::add_options_to_parser(parser);
  SymParamsSearch::add_options_to_parser(parser, maxStepTime, maxStepNodes);
}
void SymController::new_solution(const SymSolution &sol) {
  if (searchParams.top_k && !solution_registry.found_all_plans()) {
    solution_registry.register_solution(sol);
  }
  if (!solution.solved() || sol.getCost() < solution.getCost()) {
    solution = sol;
    if (!searchParams.top_k) {
      std::cout << "BOUND: " << lower_bound << " < " << getUpperBound()
                << ", total time: " << utils::g_timer << std::endl;
    }
  }
}

void SymController::setLowerBound(int lower, bool hard) {
  // Never set a lower bound greater than the current upper bound
  if (hard) {
    lower = std::numeric_limits<int>::max();
  } else if (!searchParams.top_k && solution.solved()) {
    lower = min(lower, solution.getCost());
  }

  if (lower > lower_bound) {
    lower_bound = lower;
    std::cout << "BOUND: " << lower_bound << " < " << getUpperBound()
              << std::flush;
    if (searchParams.top_k) {
      solution_registry.construct_cheaper_solutions(lower);
    }
    std::cout << " [" << solution_registry.num_found_plans() << "/"
              << mgrParams.num_plans << " plans]" << std::flush;
    std::cout << ", total time: " << utils::g_timer << std::endl;
  }
}

Bdd SymController::get_states_on_goal_paths() const {
  if (searchParams.top_k) {
    return solution_registry.get_states_on_goal_paths();
  }
  return vars->zeroBDD();
}
} // namespace symbolic