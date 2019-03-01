#ifndef SYMBOLIC_CLOSED_LIST_H
#define SYMBOLIC_CLOSED_LIST_H

#include "sym_variables.h"
#include "unidirectional_search.h"

#include <map>
#include <set>
#include <vector>
#include "../plan_manager.h"
#include "../task_proxy.h"
#include "../tasks/root_task.h"

namespace symbolic {

class SymStateSpaceManager;
class SymSolution;
class UnidirectionalSearch;
class SymSearch;

class ClosedList : public OppositeFrontier {
 private:
  UnidirectionalSearch *my_search;
  SymStateSpaceManager *mgr;  // Symbolic manager to perform bdd operations

  std::map<int, Bdd> closed;  // Mapping from cost to set of states

  // Auxiliar BDDs for the number of 0-cost action steps
  // ALERT: The information here might be wrong
  // It is just used to extract path more quickly, but the information
  // here is an (admissible) estimation and this should be taken into account
  std::map<int, std::vector<Bdd>> zeroCostClosed;
  Bdd closedTotal;  // All closed states.

  int hNotClosed,
      fNotClosed;  // Bounds on h and g for those states not in closed
  std::map<int, Bdd>
      closedUpTo;  // Disjunction of BDDs in closed  (auxiliar useful to take
                   // the maximum between several BDDs)
  std::set<int> h_values;  // Set of h_values of the heuristic

  void newHValue(int h_value);

 protected:
  mutable std::vector<std::vector<OperatorID>> paths;
  mutable int num_found_plans;
  int num_target_plans;
  mutable PlanManager plan_mgr;
  TaskProxy relevant_task;

 public:
  ClosedList(int num_target_plans);
  void init(SymStateSpaceManager *manager, UnidirectionalSearch *search);
  void init(SymStateSpaceManager *manager, UnidirectionalSearch *search,
            const ClosedList &other);

  void insert(int h, const Bdd &S);
  void setHNotClosed(int h);
  void setFNotClosed(int f);

  const std::set<int> &getHValues();

  // Check if any of the states is closed.
  // In case positive, return a solution pair <f_value, S>
  virtual SymSolution checkCut(UnidirectionalSearch *search, const Bdd &states,
                               int g, bool fw) const override;

  void extract_path(const Bdd &cut, int h, bool fw,
                    std::vector<OperatorID> &path) const;

  void extract_multiply_paths(const Bdd &cut, int h, bool fw,
                              std::vector<OperatorID> path) const;

  inline Bdd getClosed() const {
    return mgr->zeroBDD();
    return closedTotal;
  }

  virtual Bdd notClosed() const override {
    return mgr->oneBDD();
    return !closedTotal;
  }

  inline std::map<int, Bdd> getClosedList() const { return closed; }

  inline int getHNotClosed() const {
    return 0;
    return hNotClosed;
  }

  inline int getFNotClosed() const {
    return 0;
    return fNotClosed;
  }

  void statistics() const;

  virtual bool exhausted() const override {
    return fNotClosed == std::numeric_limits<int>::max();
  }
};
}  // namespace symbolic

#endif
