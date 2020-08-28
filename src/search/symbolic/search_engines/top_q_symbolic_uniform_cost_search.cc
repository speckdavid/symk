#include "top_q_symbolic_uniform_cost_search.h"
#include "../../option_parser.h"
#include "../original_state_space.h"
#include "../plugin.h"
#include "../searches/bidirectional_search.h"
#include "../searches/top_k_uniform_cost_search.h"

#include <memory>

namespace symbolic {

TopqSymbolicUniformCostSearch::TopqSymbolicUniformCostSearch(
    const options::Options &opts, bool fw, bool bw)
    : TopkSymbolicUniformCostSearch(opts, fw, bw),
      quality_multiplier(opts.get<double>("quality")) {
  std::cout << "Quality: " << quality_multiplier << std::endl;
}

void TopqSymbolicUniformCostSearch::new_solution(const SymSolutionCut &sol) {
  if (!(solution_registry.found_all_plans() ||
        lower_bound > get_quality_bound())) {
    solution_registry.register_solution(sol);
    if (get_quality_bound() < std::numeric_limits<double>::infinity()) {
      // std::cout << "Quality bound: " << get_quality_bound() << std::endl;
      upper_bound = std::min((double)upper_bound, get_quality_bound() + 1);
    }
  } else {
    lower_bound = std::numeric_limits<int>::max();
  }
}

SearchStatus TopqSymbolicUniformCostSearch::step() {
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
    solution_registry.construct_cheaper_solutions(upper_bound);
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

void TopqSymbolicUniformCostSearch::add_options_to_parser(
    OptionParser &parser) {
  parser.add_option<double>("quality", "relative quality multiplier",
                            "infinity", Bounds("1.0", "infinity"));
}

} // namespace symbolic

static std::shared_ptr<SearchEngine> _parse_forward_ucs(OptionParser &parser) {
  parser.document_synopsis("Top-q Symbolic Forward Uniform Cost Search", "");
  symbolic::SymbolicSearch::add_options_to_parser(parser);
  parser.add_option<std::shared_ptr<symbolic::PlanDataBase>>(
      "plan_selection", "plan selection strategy");
  symbolic::TopqSymbolicUniformCostSearch::add_options_to_parser(parser);
  Options opts = parser.parse();

  std::shared_ptr<symbolic::SymbolicSearch> engine = nullptr;
  if (!parser.dry_run()) {
    engine = std::make_shared<symbolic::TopqSymbolicUniformCostSearch>(
        opts, true, false);
    std::cout << "Top-q Symbolic Forward Uniform Cost Search" << std::endl;
  }

  return engine;
}

static std::shared_ptr<SearchEngine> _parse_backward_ucs(OptionParser &parser) {
  parser.document_synopsis("Top-q Symbolic Backward Uniform Cost Search", "");
  symbolic::SymbolicSearch::add_options_to_parser(parser);
  parser.add_option<std::shared_ptr<symbolic::PlanDataBase>>(
      "plan_selection", "plan selection strategy");
  symbolic::TopqSymbolicUniformCostSearch::add_options_to_parser(parser);
  Options opts = parser.parse();

  std::shared_ptr<symbolic::SymbolicSearch> engine = nullptr;
  if (!parser.dry_run()) {
    engine = std::make_shared<symbolic::TopqSymbolicUniformCostSearch>(
        opts, false, true);
    std::cout << "Top-q Symbolic Backward Uniform Cost Search" << std::endl;
  }

  return engine;
}

static std::shared_ptr<SearchEngine>
_parse_bidirectional_ucs(OptionParser &parser) {
  parser.document_synopsis("Top-q Symbolic Bidirectional Uniform Cost Search",
                           "");
  symbolic::SymbolicSearch::add_options_to_parser(parser);
  parser.add_option<std::shared_ptr<symbolic::PlanDataBase>>(
      "plan_selection", "plan selection strategy");
  symbolic::TopqSymbolicUniformCostSearch::add_options_to_parser(parser);
  Options opts = parser.parse();

  std::shared_ptr<symbolic::SymbolicSearch> engine = nullptr;
  if (!parser.dry_run()) {
    engine = std::make_shared<symbolic::TopqSymbolicUniformCostSearch>(
        opts, true, true);
    std::cout << "Top-q Symbolic Bidirectional Uniform Cost Search"
              << std::endl;
  }

  return engine;
}

static Plugin<SearchEngine> _plugin_sym_fw_top_q("symq-fw", _parse_forward_ucs);
static Plugin<SearchEngine> _plugin_sym_bw_top_q("symq-bw",
                                                 _parse_backward_ucs);
static Plugin<SearchEngine> _plugin_sym_bd_top_q("symq-bd",
                                                 _parse_bidirectional_ucs);
