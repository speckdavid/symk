#ifndef SYMBOLIC_SYM_SOLUTION_H
#define SYMBOLIC_SYM_SOLUTION_H

#include <vector>
#include "../task_proxy.h"
#include "sym_variables.h"

namespace symbolic {
class UnidirectionalSearch;

class SymSolution {
 protected:
  UnidirectionalSearch *exp_fw, *exp_bw;
  std::map<std::pair<int, int>, Bdd> cuts;

  void extract_multiply_paths(const Bdd &c, int h, bool fw,
                              std::vector<OperatorID> path) const;

  // Returns zero Bdd if we dont need zero reconstruction. otherwise it returns
  // the correct cutted bdd!
  // First we need to check if it is contained in any 0 bucket or if the
  // pre/succcessor is contained in any bucket
  Bdd bdd_for_zero_reconstruction(const Bdd &c, int h, bool fw) const;

 public:
  SymSolution() : exp_fw(nullptr), exp_bw(nullptr) {}

  SymSolution(UnidirectionalSearch *e_fw, UnidirectionalSearch *e_bw, int g_val,
              int h_val, Bdd S)
      : exp_fw(e_fw), exp_bw(e_bw) {
    cuts[std::pair<int, int>(g_val, h_val)] = S;
  }

  void merge(const SymSolution &other);

  void getPlan(std::vector<OperatorID> &path) const;

  const std::map<std::pair<int, int>, Bdd> &get_cuts() const { return cuts; }

  inline bool solved() const { return !cuts.empty(); }

  inline int getCost() const {
    if (!solved()) {
      return -1;
    }
    auto any = cuts.begin();
    return any->first.first + any->first.second;
  }
};
}  // namespace symbolic
#endif
