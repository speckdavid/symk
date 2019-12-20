#ifndef SYMBOLIC_CLOSED_LIST_H
#define SYMBOLIC_CLOSED_LIST_H

#include "sym_variables.h"
#include "unidirectional_search.h"

#include <map>
#include <set>
#include <vector>

namespace symbolic {

class SymStateSpaceManager;
class SymSolutionCut;
class UnidirectionalSearch;
class SymSearch;

class ClosedList : public OppositeFrontier {
private:
  UnidirectionalSearch *my_search;
  SymStateSpaceManager *mgr; // Symbolic manager to perform bdd operations

  std::map<int, BDD> closed; // Mapping from cost to set of states

  // Auxiliar BDDs for the number of 0-cost action steps
  // ALERT: The information here might be wrong
  // It is just used to extract path more quickly, but the information
  // here is an (admissible) estimation and this should be taken into account
  std::map<int, std::vector<BDD>> zeroCostClosed;
  BDD closedTotal; // All closed states.
  int hNotClosed; // if we use bidirectional search this is used to prune states from the other side

public:
  ClosedList();
  void init(SymStateSpaceManager *manager, UnidirectionalSearch *search);
  void init(SymStateSpaceManager *manager, UnidirectionalSearch *search,
            const ClosedList &other);

  void insert(int h, const BDD &S);
  void setHNotClosed(int h);

  BDD getPartialClosed(int upper_bound) const;

  virtual std::vector<SymSolutionCut>
  getAllCuts(const BDD &states, int g, bool fw, int lower_bound) const override;

  inline BDD getClosed() const { return closedTotal; }

  virtual BDD notClosed() const override { return !closedTotal; }

  inline std::map<int, BDD> getClosedList() const { return closed; }

  int getHNotClosed() const { return hNotClosed; }

  BDD get_start_states() const {
    if (get_num_zero_closed_layers(0) == 0) {
      return get_closed_at(0);
    }
    return get_zero_closed_at(0, 0);
  }

  inline BDD get_closed_at(int h) const {
    if (!closed.count(h)) {
      return mgr->zeroBDD();
    }
    return closed.at(h);
  }

  inline BDD get_zero_closed_at(int h, int layer) const {
    return zeroCostClosed.at(h).at(layer);
  }

  inline size_t get_num_zero_closed_layers(int h) const {
    if (zeroCostClosed.count(h) == 0) {
      return 0;
    }
    return zeroCostClosed.at(h).size();
  }

  inline size_t get_zero_cut(int h, const BDD &bdd) const {
    size_t i = 0;
    if (get_num_zero_closed_layers(h)) {
      for (; i < zeroCostClosed.at(h).size(); i++) {
        BDD intersection = zeroCostClosed.at(h).at(i) * bdd;
        if (!intersection.IsZero()) {
          break;
        }
      }
    }
    return i;
  }
};
} // namespace symbolic

#endif
