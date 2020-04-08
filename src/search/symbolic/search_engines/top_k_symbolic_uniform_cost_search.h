#ifndef SYMBOLIC_SEARCH_ENGINES_TOP_K_SYMBOLIC_UNIFORM_COST_SEARCH_H
#define SYMBOLIC_SEARCH_ENGINES_TOP_K_SYMBOLIC_UNIFORM_COST_SEARCH_H

#include "symbolic_uniform_cost_search.h"

namespace symbolic {
class TopkSymbolicUniformCostSearch : public SymbolicUniformCostSearch {

protected:
  virtual void initialize() override;

public:
  TopkSymbolicUniformCostSearch(const options::Options &opts, bool fw);
  virtual ~TopkSymbolicUniformCostSearch() = default;

  virtual void setLowerBound(int lower) override;

  virtual void new_solution(const SymSolutionCut &sol) override;

  static void add_options_to_parser(OptionParser &parser);
};

} // namespace symbolic

#endif