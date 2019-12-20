#include "unidirectional_search.h"
#include "closed_list.h"
#include "sym_solution_registry.h"

using namespace std;

namespace symbolic {

OppositeFrontierFixed::OppositeFrontierFixed(BDD bdd,
                                             const SymStateSpaceManager &mgr)
    : goal(bdd), hNotGoal(mgr.getAbsoluteMinTransitionCost()) {}

std::vector<SymSolutionCut>
OppositeFrontierFixed::getAllCuts(const BDD &states, int g, bool fw,
                                  int /*lower_bound*/) const {
  std::vector<SymSolutionCut> result;
  BDD cut = states * goal;
  if (!cut.IsZero()) {
    if (fw) // Solution reconstruction will fail
      result.emplace_back(g, 0, cut);
    else
      result.emplace_back(0, g, cut);
  }
  return result;
}

UnidirectionalSearch::UnidirectionalSearch(SymController *eng,
                                           const SymParamsSearch &params)
    : SymSearch(eng, params), fw(true), closed(std::make_shared<ClosedList>()) {
}

} // namespace symbolic
