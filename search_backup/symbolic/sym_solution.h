#ifndef SYMBOLIC_SYM_SOLUTION_H
#define SYMBOLIC_SYM_SOLUTION_H

#include "sym_variables.h"
#include "../task_proxy.h"
#include <vector>

namespace symbolic
{
class UnidirectionalSearch;

class SymSolution
{
    UnidirectionalSearch *exp_fw, *exp_bw;
    int g, h;
    Bdd cut;

  public:
    SymSolution() : g(-1), h(-1) {} //No solution yet

    SymSolution(UnidirectionalSearch *e_fw, UnidirectionalSearch *e_bw, int g_val, int h_val, Bdd S) : exp_fw(e_fw), exp_bw(e_bw), g(g_val), h(h_val), cut(S) {}

    void getPlan(std::vector<OperatorID> &path) const;

    inline bool solved() const
    {
        return g + h >= 0;
    }

    inline int getCost() const
    {
        return g + h;
    }
};
} // namespace symbolic
#endif