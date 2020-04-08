
#include "top_k_symbolic_uniform_cost_search.h"
#include "../../option_parser.h"
#include "../original_state_space.h"
#include "../plugin.h"
#include "../searches/top_k_uniform_cost_search.h"

#include <memory>

namespace symbolic {

void TopkSymbolicUniformCostSearch::initialize() {
  mgr = std::make_shared<OriginalStateSpace>(vars.get(), mgrParams);
  auto uni_search = std::unique_ptr<UniformCostSearch>(
      new TopkUniformCostSearch(this, searchParams));
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

TopkSymbolicUniformCostSearch::TopkSymbolicUniformCostSearch(
    const options::Options &opts, bool fw)
    : SymbolicUniformCostSearch(opts, fw) {}

void TopkSymbolicUniformCostSearch::add_options_to_parser(
    OptionParser &parser) {
  SymbolicUniformCostSearch::add_options_to_parser(parser);
}

void TopkSymbolicUniformCostSearch::setLowerBound(int lower) {
  if (solution_registry.found_all_plans()) {
    lower_bound = std::numeric_limits<int>::max();
  } else {
    if (lower > lower_bound) {
      lower_bound = lower;
      std::cout << "BOUND: " << lower_bound << " < " << upper_bound
                << std::flush;
      solution_registry.construct_cheaper_solutions(lower);
      std::cout << " [" << solution_registry.get_num_found_plans() << "/"
                << plan_data_base->get_num_desired_plans() << " plans]"
                << std::flush;
      std::cout << ", total time: " << utils::g_timer << std::endl;
    }
  }
}

void TopkSymbolicUniformCostSearch::new_solution(const SymSolutionCut &sol) {
  if (!solution_registry.found_all_plans()) {
    solution_registry.register_solution(sol);
  } else {
    lower_bound = std::numeric_limits<int>::max();
  }
}

} // namespace symbolic

static std::shared_ptr<SearchEngine> _parse_forward_ucs(OptionParser &parser) {
  parser.document_synopsis("Symbolic Forward Uniform Cost Search", "");
  symbolic::SymbolicUniformCostSearch::add_options_to_parser(parser);
  Options opts = parser.parse();
  opts.set("top_k", true);

  std::shared_ptr<symbolic::SymbolicSearch> engine = nullptr;
  if (!parser.dry_run()) {
    engine =
        std::make_shared<symbolic::TopkSymbolicUniformCostSearch>(opts, true);
  }

  return engine;
}

static std::shared_ptr<SearchEngine> _parse_backward_ucs(OptionParser &parser) {
  parser.document_synopsis("Symbolic Backward Uniform Cost Search", "");
  symbolic::SymbolicUniformCostSearch::add_options_to_parser(parser);
  Options opts = parser.parse();
  opts.set("top_k", true);

  std::shared_ptr<symbolic::SymbolicSearch> engine = nullptr;
  if (!parser.dry_run()) {
    engine =
        std::make_shared<symbolic::TopkSymbolicUniformCostSearch>(opts, false);
  }

  return engine;
}

static Plugin<SearchEngine> _plugin_sym_fw_top_k("symk-fw", _parse_forward_ucs);
static Plugin<SearchEngine> _plugin_sym_bw_top_k("symk-bw",
                                                 _parse_backward_ucs);
