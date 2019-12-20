#ifndef SYMBOLIC_UNIDIRECTIONAL_SEARCH_H
#define SYMBOLIC_UNIDIRECTIONAL_SEARCH_H

#include "sym_search.h"

#include "sym_estimate.h"
#include "sym_utils.h"
#include <map>
#include <memory>
#include <vector>

namespace symbolic {
class SymSolutionCut;
class UnidirectionalSearch;
class SymController;
class ClosedList;

class SymExpStatistics {
public:
  double image_time, image_time_failed;
  double time_heuristic_evaluation;
  int num_steps_succeeded;
  double step_time;

  SymExpStatistics()
      : image_time(0), image_time_failed(0), time_heuristic_evaluation(0),
        num_steps_succeeded(0), step_time(0) {}

  void add_image_time(double t) {
    image_time += t;
    num_steps_succeeded += 1;
  }

  void add_image_time_failed(double t) {
    image_time += t;
    image_time_failed += t;
    num_steps_succeeded += 1;
  }
};

class OppositeFrontier {
public:
  virtual ~OppositeFrontier() {}

  virtual std::vector<SymSolutionCut>
  getAllCuts(const BDD &states, int g, bool fw, int lower_bound) const = 0;

  virtual BDD notClosed() const = 0;
};

class OppositeFrontierFixed : public OppositeFrontier {
  BDD goal;
  int hNotGoal;

public:
  OppositeFrontierFixed(BDD g, const SymStateSpaceManager &mgr);

  virtual std::vector<SymSolutionCut>
  getAllCuts(const BDD &states, int g, bool fw, int lower_bound) const override;

  virtual BDD notClosed() const override { return !goal; }
};

class UnidirectionalSearch : public SymSearch {
protected:
  bool fw; // Direction of the search. true=forward, false=backward
  std::shared_ptr<ClosedList> closed; // Closed list is a shared ptr so that we
  // can share it with other searches

  SymExpStatistics stats;

  std::shared_ptr<OppositeFrontier> perfectHeuristic;

public:
  UnidirectionalSearch(SymController *eng, const SymParamsSearch &params);

  inline bool isFW() const { return fw; }

  virtual int getG() const = 0;

  // Pointer to the closed list Used to set as heuristic of other explorations.

  inline ClosedList *getClosed() const { return closed.get(); }

  inline std::shared_ptr<ClosedList> getClosedShared() const { return closed; }
};
} // namespace symbolic
#endif