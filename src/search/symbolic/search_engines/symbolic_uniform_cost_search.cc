
#include "symbolic_uniform_cost_search.h"
#include "../../option_parser.h"
#include "../original_state_space.h"
#include "../plugin.h"
#include "../searches/uniform_cost_search.h"

#include <memory>

namespace symbolic {

void SymbolicUniformCostSearch::initialize() {
  mgr = std::make_shared<OriginalStateSpace>(vars.get(), mgrParams);
  auto uni_search = std::unique_ptr<UniformCostSearch>(
      new UniformCostSearch(this, searchParams));
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

SymbolicUniformCostSearch::SymbolicUniformCostSearch(
    const options::Options &opts, bool fw)
    : SymbolicSearch(opts), fw(fw) {}

void SymbolicUniformCostSearch::add_options_to_parser(OptionParser &parser) {
  SymbolicSearch::add_options_to_parser(parser);
}

} // namespace symbolic

static std::shared_ptr<SearchEngine> _parse_forward_ucs(OptionParser &parser) {
  parser.document_synopsis("Symbolic Forward Uniform Cost Search", "");
  symbolic::SymbolicUniformCostSearch::add_options_to_parser(parser);
  Options opts = parser.parse();

  std::shared_ptr<symbolic::SymbolicSearch> engine = nullptr;
  if (!parser.dry_run()) {
    engine = std::make_shared<symbolic::SymbolicUniformCostSearch>(opts, true);
  }

  return engine;
}

static std::shared_ptr<SearchEngine> _parse_backward_ucs(OptionParser &parser) {
  parser.document_synopsis("Symbolic Backward Uniform Cost Search", "");
  symbolic::SymbolicUniformCostSearch::add_options_to_parser(parser);
  Options opts = parser.parse();

  std::shared_ptr<symbolic::SymbolicSearch> engine = nullptr;
  if (!parser.dry_run()) {
    engine = std::make_shared<symbolic::SymbolicUniformCostSearch>(opts, false);
  }

  return engine;
}

static Plugin<SearchEngine> _plugin_sym_fw_ordinary("sym-fw",
                                                    _parse_forward_ucs);
static Plugin<SearchEngine> _plugin_sym_bw_ordinary("sym-bw",
                                                    _parse_backward_ucs);
