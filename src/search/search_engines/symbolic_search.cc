#include "symbolic_search.h"

#include "../option_parser.h"
#include "../plugin.h"

#include "../symbolic/bidirectional_search.h"
#include "../symbolic/original_state_space.h"
#include "../symbolic/sym_params_search.h"
#include "../symbolic/sym_search.h"
#include "../symbolic/sym_state_space_manager.h"
#include "../symbolic/sym_variables.h"
#include "../symbolic/uniform_cost_search.h"
#include "../task_utils/plan_graph.h"
#include "../tasks/plan_forbid_reformulated_task.h"

#include <fstream>
#include <iostream>

using namespace std;
using namespace symbolic;
using namespace options;

namespace symbolic_search {

SymbolicSearch::SymbolicSearch(const options::Options &opts)
    : SearchEngine(opts), SymController(opts) {
  set_save_plan_when_solved(false);
}

SymbolicBidirectionalUniformCostSearch::SymbolicBidirectionalUniformCostSearch(
    const options::Options &opts)
    : SymbolicSearch(opts) {}

void SymbolicBidirectionalUniformCostSearch::initialize() {
  mgr = shared_ptr<OriginalStateSpace>(new OriginalStateSpace(
      vars.get(), mgrParams, std::make_shared<PlanManager>(plan_manager)));
  auto fw_search =
      unique_ptr<UniformCostSearch>(new UniformCostSearch(this, searchParams));
  auto bw_search =
      unique_ptr<UniformCostSearch>(new UniformCostSearch(this, searchParams));
  fw_search->init(mgr, true, bw_search->getClosedShared());
  bw_search->init(mgr, false, fw_search->getClosedShared());

  search = unique_ptr<BidirectionalSearch>(new BidirectionalSearch(
      this, searchParams, move(fw_search), move(bw_search)));
}

SymbolicUniformCostSearch::SymbolicUniformCostSearch(
    const options::Options &opts, bool _fw)
    : SymbolicSearch(opts), fw(_fw) {}

void SymbolicUniformCostSearch::initialize() {
  mgr = make_shared<OriginalStateSpace>(
      vars.get(), mgrParams, std::make_shared<PlanManager>(plan_manager));
  auto uni_search =
      unique_ptr<UniformCostSearch>(new UniformCostSearch(this, searchParams));
  if (fw) {
    uni_search->init(mgr, true, nullptr);
  } else {
    uni_search->init(mgr, false, nullptr);
  }

  search.reset(uni_search.release());
}

SearchStatus SymbolicSearch::step() {
  search->step();

  if (getLowerBound() < getUpperBound()) {
    if (getLowerBound() + 0.2 > getUpperBound()) {
      vector<OperatorID> plan;
      solution.getPlan(plan);
      if (solution.all_plans_found()) {
        return SOLVED;
      }
    }
    return IN_PROGRESS;
  } else if (found_solution()) {
    vector<OperatorID> plan;
    solution.getPlan(plan);
    if (!solution.all_plans_found()) {
      std::cout << "Not enough plans found." << std::endl;
      std::cout << "#PLans: " << solution.get_found_plans().size() << std::endl;
      TaskProxy task_proxy(*tasks::g_root_task);
      std::vector<Plan> one_plan;
      one_plan.push_back(solution.get_found_plans().at(0));
      extra_tasks::PlanForbidReformulatedTask new_task(
          tasks::g_root_task, solution.get_found_plans());

      std::ofstream reforumalted_problem("reformulated_output.sas");
      new_task.dump_to_SAS(reforumalted_problem);
      reforumalted_problem.close();
      // new_task.dump();
      // plan_graph::PlanGraph p_graph(solution.get_found_plans(), task_proxy);
    }
    return SOLVED;
  } else {
    return FAILED;
  }
} // namespace symbolic_search

void SymbolicSearch::new_solution(const SymSolution &sol) {
  if (sol.getCost() < getUpperBound()) {
    vector<OperatorID> plan;
    // sol.getPlan(plan); SET DUMMY EMPTY PLAN!
    set_plan(plan);
  }

  SymController::new_solution(sol);
}
} // namespace symbolic_search

static shared_ptr<SearchEngine> _parse_bidirectional_ucs(OptionParser &parser) {
  parser.document_synopsis("Symbolic Bidirectional Uniform Cost Search", "");

  SearchEngine::add_options_to_parser(parser);
  SymVariables::add_options_to_parser(parser);
  SymParamsSearch::add_options_to_parser(parser, 30e3, 10e7);
  SymParamsMgr::add_options_to_parser(parser);
  Bdd::add_options_to_parser(parser);

  Options opts = parser.parse();

  shared_ptr<symbolic_search::SymbolicSearch> engine = nullptr;
  if (!parser.dry_run()) {
    Bdd::parse_options(opts);
    engine =
        make_shared<symbolic_search::SymbolicBidirectionalUniformCostSearch>(
            opts);
  }

  return engine;
}

static shared_ptr<SearchEngine> _parse_forward_ucs(OptionParser &parser) {
  parser.document_synopsis("Symbolic Bidirectional Uniform Cost Search", "");

  SearchEngine::add_options_to_parser(parser);
  SymVariables::add_options_to_parser(parser);
  SymParamsSearch::add_options_to_parser(parser, 30e3, 10e7);
  SymParamsMgr::add_options_to_parser(parser);
  Bdd::add_options_to_parser(parser);

  Options opts = parser.parse();

  shared_ptr<symbolic_search::SymbolicSearch> engine = nullptr;
  if (!parser.dry_run()) {
    Bdd::parse_options(opts);
    engine =
        make_shared<symbolic_search::SymbolicUniformCostSearch>(opts, true);
  }

  return engine;
}

static shared_ptr<SearchEngine> _parse_backward_ucs(OptionParser &parser) {
  parser.document_synopsis("Symbolic Bidirectional Uniform Cost Search", "");

  SearchEngine::add_options_to_parser(parser);
  SymVariables::add_options_to_parser(parser);
  SymParamsSearch::add_options_to_parser(parser, 30e3, 10e7);
  SymParamsMgr::add_options_to_parser(parser);
  Bdd::add_options_to_parser(parser);

  Options opts = parser.parse();

  shared_ptr<symbolic_search::SymbolicSearch> engine = nullptr;
  if (!parser.dry_run()) {
    Bdd::parse_options(opts);
    engine =
        make_shared<symbolic_search::SymbolicUniformCostSearch>(opts, false);
  }

  return engine;
}

static Plugin<SearchEngine> _plugin_bd("sbd", _parse_bidirectional_ucs);
static Plugin<SearchEngine> _plugin_fw("sfw", _parse_forward_ucs);
static Plugin<SearchEngine> _plugin_bw("sbw", _parse_backward_ucs);
