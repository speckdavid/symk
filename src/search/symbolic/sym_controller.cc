#include "sym_controller.h"

#include "../option_parser.h"
#include "../task_utils/task_properties.h"
#include "opt_order.h"
#include "plan_selection/plan_database.h"
#include "sym_state_space_manager.h"

using namespace std;

namespace symbolic {

SymController::SymController(const Options &opts)
    : vars(make_shared<SymVariables>(opts)), mgrParams(opts),
      searchParams(opts), lower_bound(0),
      upper_bound(std::numeric_limits<int>::max()), min_g(0),
      plan_data_base(opts.get<std::shared_ptr<PlanDataBase>>("plan_selection")),
      solution_registry() {
  // task_properties::verify_no_axioms(TaskProxy(*tasks::g_root_task));
  mgrParams.print_options();
  searchParams.print_options();
  plan_data_base->print_options();
  vars->init();
}

void SymController::init(UniformCostSearch *fwd_search,
                         UniformCostSearch *bwd_search) {
  plan_data_base->init(vars);
  solution_registry.init(vars, fwd_search, bwd_search, plan_data_base);
}

void SymController::add_options_to_parser(OptionParser &parser, int maxStepTime,
                                          int maxStepNodes) {
  SymVariables::add_options_to_parser(parser);
  SymParamsMgr::add_options_to_parser(parser);
  SymParamsSearch::add_options_to_parser(parser, maxStepTime, maxStepNodes);
  parser.add_option<std::shared_ptr<PlanDataBase>>(
      "plan_selection", "plan selection strategy", "top_k(1)");
  // PlanDataBase::add_options_to_parser(parser);
}

void SymController::new_solution(const SymSolutionCut &sol) {
  if (!solution_registry.found_all_plans()) {
    solution_registry.register_solution(sol);
    if (!searchParams.top_k) {
      setUpperBound(std::min(getUpperBound(), sol.get_f()));
    }
  } else {
    lower_bound = std::numeric_limits<int>::max();
  }
}

void SymController::setLowerBound(int lower) {
  if (solution_registry.found_all_plans()) {
    lower_bound = std::numeric_limits<int>::max();
  } else {
    if (lower > lower_bound) {
      lower_bound = lower;
      std::cout << "BOUND: " << lower_bound << " < " << getUpperBound()
                << std::flush;
      if (!searchParams.top_k) {
        if (lower_bound >= upper_bound) {
          solution_registry.construct_cheaper_solutions(
              std::numeric_limits<int>::max());
        }
      } else {
        solution_registry.construct_cheaper_solutions(lower);
      }
      std::cout << " [" << solution_registry.get_num_found_plans() << "/"
                << plan_data_base->get_num_desired_plans() << " plans]"
                << std::flush;
      std::cout << ", total time: " << utils::g_timer << std::endl;
    }
  }
}

BDD SymController::get_states_on_goal_paths() const {
  return solution_registry.get_states_on_goal_paths();
}

} // namespace symbolic
