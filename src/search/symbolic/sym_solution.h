#ifndef SYMBOLIC_SYM_SOLUTION_H
#define SYMBOLIC_SYM_SOLUTION_H

#include "../task_proxy.h"
#include "sym_variables.h"
#include <vector>

namespace symbolic {
class UnidirectionalSearch;

class SymSolution {
  UnidirectionalSearch *exp_fw, *exp_bw;
  int g, h;
  BDD cut;

public:
  SymSolution() : g(-1), h(-1) {} // No solution yet

  SymSolution(UnidirectionalSearch *e_fw, UnidirectionalSearch *e_bw, int g_val,
              int h_val, BDD S)
      : exp_fw(e_fw), exp_bw(e_bw), g(g_val), h(h_val), cut(S) {}

  void getPlan(std::vector<OperatorID> &path) const;

  inline bool solved() const { return g + h >= 0; }

  inline int getCost() const { return g + h; }

  inline int get_g() const { return g; }

  inline int get_h() const { return h; }

  inline BDD get_cut() const { return cut; }

  inline UnidirectionalSearch *get_fw_search() const { return exp_fw; }

  inline UnidirectionalSearch *get_bw_search() const { return exp_bw; }
};
} // namespace symbolic
#endif