#include "symbolic_search.h"

#include "../option_parser.h"
#include "../plugin.h"

#include "../original_state_space.h"
#include "../plan_selection/plan_database.h"
#include "../searches/bidirectional_search.h"
#include "../searches/top_k_uniform_cost_search.h"
#include "../searches/uniform_cost_search.h"

#include "../sym_params_search.h"
#include "../sym_state_space_manager.h"
#include "../sym_variables.h"

#include "../task_utils/task_properties.h"

using namespace std;
using namespace symbolic;
using namespace options;

namespace symbolic {

SymbolicSearch::SymbolicSearch(const options::Options &opts)
    : SearchEngine(opts), vars(make_shared<SymVariables>(opts)),
      mgrParams(opts), searchParams(opts), step_num(-1),
      lower_bound_increased(true), lower_bound(0),
      upper_bound(std::numeric_limits<int>::max()), min_g(0),
      plan_data_base(opts.get<std::shared_ptr<PlanDataBase>>("plan_selection")),
      solution_registry() {
  mgrParams.print_options();
  searchParams.print_options();
  vars->init();
}

void SymbolicSearch::initialize() {
  plan_data_base->set_plan_manager(get_plan_manager());
  plan_data_base->print_options();
}

SearchStatus SymbolicSearch::step() {
  step_num++;
  // Handling empty plan
  if (step_num == 0) {
    BDD cut = mgr->getInitialState() * mgr->getGoal();
    if (!cut.IsZero()) {
      new_solution(SymSolutionCut(0, 0, cut));
    }
  }

  SearchStatus cur_status;

  // Search finished!
  if (lower_bound >= upper_bound) {
    solution_registry.construct_cheaper_solutions(
        std::numeric_limits<int>::max());
    solution_found = plan_data_base->get_num_reported_plan() > 0;
    cur_status = solution_found ? SOLVED : FAILED;
  } else {
    // Bound increade => construct plans
    if (lower_bound_increased) {
      solution_registry.construct_cheaper_solutions(lower_bound);
    }

    // All plans found
    if (solution_registry.found_all_plans()) {
      solution_found = true;
      cur_status = SOLVED;
    } else {
      cur_status = IN_PROGRESS;
    }
  }

  if (lower_bound_increased) {
    std::cout << "BOUND: " << lower_bound << " < " << upper_bound << std::flush;

    std::cout << " [" << solution_registry.get_num_found_plans() << "/"
              << plan_data_base->get_num_desired_plans() << " plans]"
              << std::flush;
    std::cout << ", total time: " << utils::g_timer << std::endl;
  }
  lower_bound_increased = false;

  if (cur_status == SOLVED) {
    std::cout << "Best plan:" << std::endl;
    plan_data_base->dump_first_accepted_plan();
    return cur_status;
  }
  if (cur_status == FAILED) {
    return cur_status;
  }

  // Actuall step
  search->step();

  return cur_status;
}

void SymbolicSearch::setLowerBound(int lower) {
  if (lower > lower_bound) {
    lower_bound_increased = true;
  }
  lower_bound = std::max(lower_bound, lower);
}

void SymbolicSearch::new_solution(const SymSolutionCut &sol) {
  if (!solution_registry.found_all_plans()) {
    solution_registry.register_solution(sol);
    upper_bound = std::min(upper_bound, sol.get_f());
  }
}

void SymbolicSearch::add_options_to_parser(OptionParser &parser) {
  SearchEngine::add_options_to_parser(parser);
  SymVariables::add_options_to_parser(parser);
  SymParamsSearch::add_options_to_parser(parser, 30e3, 10e7);
  SymParamsMgr::add_options_to_parser(parser);
  PlanDataBase::add_options_to_parser(parser);
}
} // namespace symbolic
