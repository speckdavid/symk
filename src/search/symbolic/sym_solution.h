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

    inline int get_g() const 
    {
        return g;
    }

    inline int get_h() const 
    {
        return h;
    }

    inline Bdd get_cut() const 
    {
        return cut;
    }

    inline UnidirectionalSearch* get_fw_search() const {
        return exp_fw;
    }

    inline UnidirectionalSearch* get_bw_search() const {
        return exp_bw;
    }

};
} // namespace symbolic
#endif