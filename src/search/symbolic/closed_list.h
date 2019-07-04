#ifndef SYMBOLIC_CLOSED_LIST_H
#define SYMBOLIC_CLOSED_LIST_H

#include "sym_variables.h"
#include "unidirectional_search.h"

#include <map>
#include <set>
#include <vector>

namespace symbolic
{

class SymStateSpaceManager;
class SymSolution;
class UnidirectionalSearch;
class SymSearch;

class ClosedList : public OppositeFrontier
{
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

  std::map<int, BDD>
      closedUpTo;         // Disjunction of BDDs in closed  (auxiliar useful to take the
                          // maximum between several BDDs)
  std::set<int> h_values; // Set of h_values of the heuristic

  void newHValue(int h_value);

public:
  ClosedList();
  void init(SymStateSpaceManager *manager, UnidirectionalSearch *search);
  void init(SymStateSpaceManager *manager, UnidirectionalSearch *search,
            const ClosedList &other);

  void insert(int h, const BDD &S);

  const std::set<int> &getHValues();

  BDD getFullyCostClosed() const
  {
    BDD res = mgr->zeroBDD();
    size_t i = 0;
    for (const auto& pair : closed)
    {
      if (i == closed.size() - 1) {
        break;
      }
      res += pair.second;
      i++;
      // std::cout << "Closed entry: " << i << std::endl;
    }
    return res;
  }

  // Check if any of the states is closed.
  // In case positive, return a solution pair <f_value, S>
  virtual SymSolution checkCut(UnidirectionalSearch *search, const BDD &states,
                               int g, bool fw) const override;

  virtual std::vector<SymSolution> getAllCuts(UnidirectionalSearch *search,
                                              const BDD &states, int g, bool fw,
                                              int lower_bound) const override;

  void extract_path(const BDD &cut, int h, bool fw,
                    std::vector<OperatorID> &path) const;

  inline BDD getClosed() const { return closedTotal; }

  virtual BDD notClosed() const override { return !closedTotal; }

  inline std::map<int, BDD> getClosedList() const { return closed; }

  BDD get_start_states() const
  {
    if (get_num_zero_closed_layers(0) == 0)
    {
      return get_closed_at(0);
    }
    return get_zero_closed_at(0, 0);
  }

  inline BDD get_closed_at(int h) const
  {
    if (!closed.count(h))
    {
      return mgr->zeroBDD();
    }
    return closed.at(h);
  }

  inline BDD get_zero_closed_at(int h, int layer) const
  {
    return zeroCostClosed.at(h).at(layer);
  }

  inline size_t get_num_zero_closed_layers(int h) const
  {
    if (zeroCostClosed.count(h) == 0)
    {
      return 0;
    }
    return zeroCostClosed.at(h).size();
  }

  inline size_t get_zero_cut(int h, const BDD &bdd) const
  {
    size_t i = 0;
    if (get_num_zero_closed_layers(h))
    {
      for (; i < zeroCostClosed.at(h).size(); i++)
      {
        BDD intersection = zeroCostClosed.at(h).at(i) * bdd;
        if (!intersection.IsZero())
        {
          break;
        }
      }
    }
    return i;
  }

  void statistics() const;

  void dump() const
  {
    for (auto pair : zeroCostClosed)
    {
      std::cout << "Closed at cost " << pair.first << ":" << std::endl;
      int i = 0;
      for (auto bdd : pair.second)
      {
        std::cout << "   Layer " << i++ << ": " << bdd.nodeCount() << "Node" << std::endl;
      }
    }
  }
};
} // namespace symbolic

#endif
