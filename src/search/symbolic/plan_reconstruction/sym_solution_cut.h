#ifndef SYMBOLIC_PLAN_RECONSTRUCTION_SYM_SOLUTION_H
#define SYMBOLIC_PLAN_RECONSTRUCTION_SYM_SOLUTION_H

#include "../../task_proxy.h"
#include "../sym_variables.h"

#include <vector>

namespace symbolic {
class SymSolutionCut {
protected:
    int g;
    int h;
    BDD cut;
    int sol_cost; // orginial solution cost

public:
    SymSolutionCut(); // dummy for no solution
    SymSolutionCut(int g, int h, BDD cut, int sol_cost);

    int get_g() const;
    int get_h() const;
    int get_f() const;
    int get_sol_cost() const;
    BDD get_cut() const;
    void merge(const SymSolutionCut &other);

    void set_g(int g);
    void set_h(int h);
    void set_cut(BDD cut);

    // Here we only compare g and h values!!!
    bool operator<(const SymSolutionCut &other) const;
    bool operator>(const SymSolutionCut &other) const;
    bool operator==(const SymSolutionCut &other) const;
    bool operator!=(const SymSolutionCut &other) const;

    friend std::ostream &operator<<(std::ostream &os,
                                    const SymSolutionCut &sym_cut) {
        return os << "symcut{g=" << sym_cut.get_g() << ", h=" << sym_cut.get_h()
                  << ", f=" << sym_cut.get_f()
                  << ", nodes=" << sym_cut.get_cut().nodeCount() << "}";
    }
};
} // namespace symbolic
#endif
