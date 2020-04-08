#include "symbolic_search.h"

#include "../option_parser.h"
#include "../plugin.h"

#include "../symbolic/original_state_space.h"
#include "../symbolic/plan_selection/plan_database.h"
#include "../symbolic/searches/bidirectional_search.h"
#include "../symbolic/searches/uniform_cost_search.h"
#include "../symbolic/sym_params_search.h"
#include "../symbolic/sym_state_space_manager.h"
#include "../symbolic/sym_variables.h"

#include "../task_utils/task_properties.h"

using namespace std;
using namespace symbolic;
using namespace options;

namespace symbolic {

bool only_zero_cost_actions() {
  return task_properties::get_max_operator_cost(
             TaskProxy(*tasks::g_root_task)) == 0;
}

SymbolicSearch::SymbolicSearch(const options::Options &opts)
    : SearchEngine(opts), vars(make_shared<SymVariables>(opts)),
      mgrParams(opts), searchParams(opts), lower_bound(0),
      upper_bound(std::numeric_limits<int>::max()), min_g(0),
      plan_data_base(opts.get<std::shared_ptr<PlanDataBase>>("plan_selection")),
      solution_registry() {
  mgrParams.print_options();
  searchParams.print_options();
  plan_data_base->print_options();
  vars->init();
}

SymbolicBidirectionalUniformCostSearch::SymbolicBidirectionalUniformCostSearch(
    const options::Options &opts)
    : SymbolicSearch(opts) {}

void SymbolicBidirectionalUniformCostSearch::initialize() {
  mgr = shared_ptr<OriginalStateSpace>(
      new OriginalStateSpace(vars.get(), mgrParams, only_zero_cost_actions()));
  auto fw_search =
      unique_ptr<UniformCostSearch>(new UniformCostSearch(this, searchParams));
  auto bw_search =
      unique_ptr<UniformCostSearch>(new UniformCostSearch(this, searchParams));
  fw_search->init(mgr, true, bw_search->getClosedShared());
  bw_search->init(mgr, false, fw_search->getClosedShared());

  plan_data_base->init(vars);
  solution_registry.init(vars, fw_search.get(), bw_search.get(),
                         plan_data_base);

  search = unique_ptr<BidirectionalSearch>(new BidirectionalSearch(
      this, searchParams, move(fw_search), move(bw_search)));
}

SymbolicUniformCostSearch::SymbolicUniformCostSearch(
    const options::Options &opts, bool _fw)
    : SymbolicSearch(opts), fw(_fw) {}

void SymbolicUniformCostSearch::initialize() {
  mgr = make_shared<OriginalStateSpace>(vars.get(), mgrParams,
                                        only_zero_cost_actions());
  auto uni_search =
      unique_ptr<UniformCostSearch>(new UniformCostSearch(this, searchParams));
  if (fw) {
    uni_search->init(mgr, true, nullptr);
    plan_data_base->init(vars);
    solution_registry.init(vars, uni_search.get(), nullptr, plan_data_base);
  } else {
    uni_search->init(mgr, false, nullptr);
    plan_data_base->init(vars);
    solution_registry.init(vars, nullptr, uni_search.get(), plan_data_base);
  }

  search.reset(uni_search.release());
}

SearchStatus SymbolicSearch::step() {
  search->step();

  if (lower_bound < upper_bound) {
    return IN_PROGRESS;
  } else if (lower_bound == upper_bound) {
    solution_registry.construct_cheaper_solutions(lower_bound + 1);
    solution_found = true;
    return SOLVED;
  } else if (plan_data_base->get_num_reported_plan() > 0) {
    solution_found = true;
    return SOLVED;
  } else {
    return FAILED;
  }
}

void SymbolicSearch::setLowerBound(int lower) {
  if (solution_registry.found_all_plans()) {
    lower_bound = std::numeric_limits<int>::max();
  } else {
    if (lower > lower_bound) {
      lower_bound = lower;
      std::cout << "BOUND: " << lower_bound << " < " << upper_bound
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

void SymbolicSearch::new_solution(const SymSolutionCut &sol) {
  if (!solution_registry.found_all_plans()) {
    solution_registry.register_solution(sol);
    if (!searchParams.top_k) {
      upper_bound = std::min(upper_bound, sol.get_f());
    }
  } else {
    lower_bound = std::numeric_limits<int>::max();
  }
}
} // namespace symbolic

// Parsing Symbolic Planning
static void add_options(OptionParser &parser) {
  SearchEngine::add_options_to_parser(parser);
  SymVariables::add_options_to_parser(parser);
  SymParamsSearch::add_options_to_parser(parser, 30e3, 10e7);
  SymParamsMgr::add_options_to_parser(parser);
  PlanDataBase::add_options_to_parser(parser);
  parser.add_option<std::shared_ptr<PlanDataBase>>(
      "plan_selection", "plan selection strategy", "top_k(1)");
}

static shared_ptr<SearchEngine> _parse_bidirectional_ucs(OptionParser &parser,
                                                         Options &opts) {
  parser.document_synopsis("Symbolic Bidirectional Uniform Cost Search", "");
  shared_ptr<symbolic::SymbolicSearch> engine = nullptr;
  if (!parser.dry_run()) {
    engine =
        make_shared<symbolic::SymbolicBidirectionalUniformCostSearch>(opts);
  }

  return engine;
}

static shared_ptr<SearchEngine> _parse_forward_ucs(OptionParser &parser,
                                                   Options &opts) {
  parser.document_synopsis("Symbolic Forward Uniform Cost Search", "");
  shared_ptr<symbolic::SymbolicSearch> engine = nullptr;
  if (!parser.dry_run()) {
    engine = make_shared<symbolic::SymbolicUniformCostSearch>(opts, true);
  }

  return engine;
}

static shared_ptr<SearchEngine> _parse_backward_ucs(OptionParser &parser,
                                                    Options &opts) {
  parser.document_synopsis("Symbolic Backward Uniform Cost Search", "");
  shared_ptr<symbolic::SymbolicSearch> engine = nullptr;
  if (!parser.dry_run()) {
    engine = make_shared<symbolic::SymbolicUniformCostSearch>(opts, false);
  }

  return engine;
}

// Symbolic Ordinary planning

static shared_ptr<SearchEngine>
_parse_bidirectional_ucs_ordinary(OptionParser &parser) {
  std::string planner = "Symbolic Bidirectional Uniform Cost Search";
  if (!parser.dry_run()) {
    std::cout << planner << std::endl;
  }
  add_options(parser);
  Options opts = parser.parse();
  opts.set("top_k", false);
  return _parse_bidirectional_ucs(parser, opts);
}
static shared_ptr<SearchEngine>
_parse_forward_ucs_ordinary(OptionParser &parser) {
  std::string planner = "Symbolic Forward Uniform Cost Search";
  if (!parser.dry_run()) {
    std::cout << planner << std::endl;
  }
  add_options(parser);
  Options opts = parser.parse();
  opts.set("top_k", false);
  parser.document_synopsis(planner, "");
  return _parse_forward_ucs(parser, opts);
}

static shared_ptr<SearchEngine>
_parse_backward_ucs_ordinary(OptionParser &parser) {
  std::string planner = "Symbolic Backward Uniform Cost Search";
  if (!parser.dry_run()) {
    std::cout << planner << std::endl;
  }
  add_options(parser);
  Options opts = parser.parse();
  opts.set("top_k", false);
  parser.document_synopsis(planner, "");
  return _parse_backward_ucs(parser, opts);
}

static Plugin<SearchEngine>
    _plugin_sym_bd_ordinary("sym-bd", _parse_bidirectional_ucs_ordinary);
static Plugin<SearchEngine>
    _plugin_sym_fw_ordinary("sym-fw", _parse_forward_ucs_ordinary);
static Plugin<SearchEngine>
    _plugin_sym_bw_ordinary("sym-bw", _parse_backward_ucs_ordinary);

// Symbolic Top-k planning

static shared_ptr<SearchEngine>
_parse_bidirectional_ucs_top_k(OptionParser &parser) {
  std::string planner = "Top-k Symbolic Bdirectional Uniform Cost Search";
  if (!parser.dry_run()) {
    std::cout << planner << std::endl;
  }
  add_options(parser);
  Options opts = parser.parse();
  opts.set("top_k", true);
  parser.document_synopsis(planner, "");
  return _parse_bidirectional_ucs(parser, opts);
}
static shared_ptr<SearchEngine> _parse_forward_ucs_top_k(OptionParser &parser) {
  std::string planner = "Top-k Symbolic Forward Uniform Cost Search";
  if (!parser.dry_run()) {
    std::cout << planner << std::endl;
  }
  add_options(parser);
  Options opts = parser.parse();
  opts.set("top_k", true);
  parser.document_synopsis(planner, "");
  return _parse_forward_ucs(parser, opts);
}

static shared_ptr<SearchEngine>
_parse_backward_ucs_top_k(OptionParser &parser) {
  std::string planner = "Top-k Symbolic Backward Uniform Cost Search";
  if (!parser.dry_run()) {
    std::cout << planner << std::endl;
  }
  add_options(parser);
  Options opts = parser.parse();
  opts.set("top_k", true);
  parser.document_synopsis(planner, "");
  return _parse_backward_ucs(parser, opts);
}

static Plugin<SearchEngine>
    _plugin_sym_bd_top_k("symk-bd", _parse_bidirectional_ucs_top_k);
static Plugin<SearchEngine> _plugin_sym_fw_top_k("symk-fw",
                                                 _parse_forward_ucs_top_k);
static Plugin<SearchEngine> _plugin_sym_bw_top_k("symk-bw",
                                                 _parse_backward_ucs_top_k);
