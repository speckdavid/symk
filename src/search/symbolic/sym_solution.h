#ifndef SYMBOLIC_SYM_SOLUTION_H
#define SYMBOLIC_SYM_SOLUTION_H

#include "../task_proxy.h"
#include "sym_variables.h"
#include <vector>

class StateRegistry;

namespace symbolic {
class UnidirectionalSearch;

class SymSolution {
protected:
  static std::unique_ptr<StateRegistry> state_registry;
  UnidirectionalSearch *exp_fw, *exp_bw;
  std::map<std::pair<int, int>, Bdd> cuts;
  std::map<std::pair<int, int>, Bdd> already_stored_cuts;
  int working_h;

  void extract_multiply_paths(const Bdd &c, int h, bool fw,
                              std::vector<OperatorID> path) const;

  // Returns zero Bdd if we dont need zero reconstruction. otherwise it returns
  // the correct cutted bdd!
  // First we need to check if it is contained in any 0 bucket or if the
  // pre/succcessor is contained in any bucket
  Bdd bdd_for_zero_reconstruction(const Bdd &c, int h, bool fw) const;

  void save_plan(std::vector<OperatorID> &path, bool fw) const;

  std::pair<int, Bdd>
  get_resulting_state(const std::vector<OperatorID> &partial_plan) const;

public:
  SymSolution() : exp_fw(nullptr), exp_bw(nullptr) {}

  SymSolution(UnidirectionalSearch *e_fw, UnidirectionalSearch *e_bw, int g_val,
              int h_val, Bdd S)
      : exp_fw(e_fw), exp_bw(e_bw) {
    if (g_val != -1) {
      cuts[std::pair<int, int>(g_val, h_val)] = S;
    }
  }

  void merge(const SymSolution &other);

  void getPlan(std::vector<OperatorID> &path);

  const std::map<std::pair<int, int>, Bdd> &get_cuts() const { return cuts; }

  inline bool solved() const { return !cuts.empty(); }

  bool all_plans_found() const;

  inline double getCost() const {
    if (!solved()) {
      return -1;
    }
    auto any = cuts.begin();
    return any->first.first + any->first.second + 0.1;
  }

  const std::vector<std::vector<OperatorID>> &get_found_plans();
};
} // namespace symbolic
#endif
