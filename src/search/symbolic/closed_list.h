#ifndef SYMBOLIC_CLOSED_LIST_H
#define SYMBOLIC_CLOSED_LIST_H

#include "sym_variables.h"
#include "unidirectional_search.h"

#include <map>
#include <set>
#include <vector>

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

 public:
  ClosedList();
  void init(SymStateSpaceManager *manager, UnidirectionalSearch *search);
  void init(SymStateSpaceManager *manager, UnidirectionalSearch *search,
            const ClosedList &other);

  void insert(int h, const Bdd &S);
  void setHNotClosed(int h);
  void setFNotClosed(int f);

  Bdd get_closed_at(int h) const {
    if (!closed.count(h)) {
      return mgr->zeroBDD();
    }
    return closed.at(h);
  }
  Bdd get_zero_closed_at(int h, int layer) const {
    return zeroCostClosed.at(h).at(layer);
  }
  size_t get_num_zero_closed_layers(int h) const {
    return zeroCostClosed.count(h);
  }

  size_t get_zero_cut(int h, const Bdd &bdd) const;

  const std::set<int> &getHValues();

  // Check if any of the states is closed.
  // In case positive, return a solution pair <f_value, S>
  virtual SymSolution checkCut(UnidirectionalSearch *search, const Bdd &states,
                               int g, bool fw) const override;

  void extract_path(const Bdd &cut, int h, bool fw,
                    std::vector<OperatorID> &path) const;

  inline Bdd getClosed() const { return closedTotal; }

  virtual Bdd notClosed() const override { return !closedTotal; }

  inline std::map<int, Bdd> getClosedList() const { return closed; }

  inline int getHNotClosed() const { return hNotClosed; }

  inline int getFNotClosed() const { return fNotClosed; }

  void statistics() const;

  virtual bool exhausted() const override {
    return fNotClosed == std::numeric_limits<int>::max();
  }
};
}  // namespace symbolic

#endif
