#ifndef SYMBOLIC_UNIDIRECTIONAL_SEARCH_H
#define SYMBOLIC_UNIDIRECTIONAL_SEARCH_H

#include "sym_search.h"

#include <map>
#include <memory>
#include <vector>
#include "sym_estimate.h"
#include "sym_utils.h"

namespace symbolic {
class SymSolution;
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
      : image_time(0),
        image_time_failed(0),
        time_heuristic_evaluation(0),
        num_steps_succeeded(0),
        step_time(0) {}

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
  virtual SymSolution checkCut(UnidirectionalSearch *search, const Bdd &states,
                               int g, bool fw) const = 0;

  virtual Bdd notClosed() const = 0;

  // Returns true only if all not closed states are guaranteed to be dead ends
  virtual bool exhausted() const = 0;

  virtual int getHNotClosed() const = 0;
};

class OppositeFrontierFixed : public OppositeFrontier {
  Bdd goal;
  int hNotGoal;

 public:
  OppositeFrontierFixed(Bdd g, const SymStateSpaceManager &mgr);
  virtual SymSolution checkCut(UnidirectionalSearch *search, const Bdd &states,
                               int g, bool fw) const override;

  virtual Bdd notClosed() const override { return !goal; }

  virtual bool exhausted() const override { return false; }

  virtual int getHNotClosed() const { return hNotGoal; }
};

class UnidirectionalSearch : public SymSearch {
 protected:
  bool fw;  // Direction of the search. true=forward, false=backward
  std::shared_ptr<ClosedList> closed;  // Closed list is a shared ptr so that we
                                       // can share it with other searches

  SymExpStatistics stats;

  std::shared_ptr<OppositeFrontier> perfectHeuristic;

 public:
  UnidirectionalSearch(SymController *eng, const SymParamsSearch &params);

  inline bool isFW() const { return fw; }

  void statistics() const;

  virtual void getPlan(const Bdd &cut, int g,
                       std::vector<OperatorID> &path) const = 0;

  virtual int getG() const = 0;

  std::shared_ptr<ClosedList> getClosedShared() const;
};
}  // namespace symbolic
#endif
