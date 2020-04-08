#ifndef SEARCH_ENGINES_SYMBOLIC_SEARCH_H
#define SEARCH_ENGINES_SYMBOLIC_SEARCH_H

#include <memory>
#include <vector>

#include "../search_engine.h"
#include "../symbolic/sym_enums.h"
#include "../symbolic/sym_params_search.h"
#include "../symbolic/sym_solution_registry.h"
#include "../symbolic/sym_state_space_manager.h"

namespace options {
class Options;
}

namespace symbolic {
class SymStateSpaceManager;
class SymSearch;
class PlanDataBase;
class SymVariables;

class SymbolicSearch : public SearchEngine {
protected:
  // Symbolic manager to perform bdd operations
  std::shared_ptr<SymStateSpaceManager> mgr;

  std::unique_ptr<SymSearch> search;

  std::shared_ptr<SymVariables> vars; // The symbolic variables are declared

  SymParamsMgr mgrParams; // Parameters for SymStateSpaceManager configuration.
  SymParamsSearch searchParams; // Parameters to search the original state space

  int lower_bound; // Lower bound of search (incl. min-action costs)
  int upper_bound; // Upper bound of search (not use by top_k)
  int min_g;       // min g costs of open lists

  std::shared_ptr<PlanDataBase> plan_data_base;
  SymSolutionRegistry solution_registry; // Solution registry

  virtual SearchStatus step() override;

public:
  SymbolicSearch(const options::Options &opts);
  virtual ~SymbolicSearch() = default;

  virtual void setLowerBound(int lower);

  virtual void setMinG(int g) { min_g = std::max(g, min_g); }

  virtual bool solved() const { return lower_bound >= upper_bound; }

  virtual int getLowerBound() const { return lower_bound; }

  virtual void new_solution(const SymSolutionCut &sol);
};

//////// Specialized

class SymbolicBidirectionalUniformCostSearch : public SymbolicSearch {
protected:
  virtual void initialize() override;

public:
  SymbolicBidirectionalUniformCostSearch(const options::Options &opts);
  virtual ~SymbolicBidirectionalUniformCostSearch() = default;
};

class SymbolicUniformCostSearch : public SymbolicSearch {
  bool fw;

protected:
  virtual void initialize() override;

public:
  SymbolicUniformCostSearch(const options::Options &opts, bool _fw);
  virtual ~SymbolicUniformCostSearch() = default;
};

} // namespace symbolic

#endif