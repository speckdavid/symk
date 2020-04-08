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

namespace symbolic_search {

bool only_zero_cost_actions() {
  return task_properties::get_max_operator_cost(
             TaskProxy(*tasks::g_root_task)) == 0;
}

SymbolicSearch::SymbolicSearch(const options::Options &opts)
    : SearchEngine(opts), SymController(opts) {}

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

  SymController::init(fw_search.get(), bw_search.get());

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
    SymController::init(uni_search.get(), nullptr);
  } else {
    uni_search->init(mgr, false, nullptr);
    SymController::init(nullptr, uni_search.get());
  }

  search.reset(uni_search.release());
}

SearchStatus SymbolicSearch::step() {
  search->step();

  if (getLowerBound() < getUpperBound()) {
    return IN_PROGRESS;
  } else if (!SymController::get_states_on_goal_paths().IsZero()) {
    solution_found = true;
    return SOLVED;
  } else {
    return FAILED;
  }
}

void SymbolicSearch::new_solution(const SymSolutionCut &sol) {
  SymController::new_solution(sol);
}
} // namespace symbolic_search

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
  shared_ptr<symbolic_search::SymbolicSearch> engine = nullptr;
  if (!parser.dry_run()) {
    engine =
        make_shared<symbolic_search::SymbolicBidirectionalUniformCostSearch>(
            opts);
  }

  return engine;
}

static shared_ptr<SearchEngine> _parse_forward_ucs(OptionParser &parser,
                                                   Options &opts) {
  parser.document_synopsis("Symbolic Forward Uniform Cost Search", "");
  shared_ptr<symbolic_search::SymbolicSearch> engine = nullptr;
  if (!parser.dry_run()) {
    engine =
        make_shared<symbolic_search::SymbolicUniformCostSearch>(opts, true);
  }

  return engine;
}

static shared_ptr<SearchEngine> _parse_backward_ucs(OptionParser &parser,
                                                    Options &opts) {
  parser.document_synopsis("Symbolic Backward Uniform Cost Search", "");
  shared_ptr<symbolic_search::SymbolicSearch> engine = nullptr;
  if (!parser.dry_run()) {
    engine =
        make_shared<symbolic_search::SymbolicUniformCostSearch>(opts, false);
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
