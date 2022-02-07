#ifndef SYMBOLIC_PLAN_RECONSTRUCTION_SIMPLE_SYM_SOLUTION_H
#define SYMBOLIC_PLAN_RECONSTRUCTION_SIMPLE_SYM_SOLUTION_H

#include "sym_solution_cut.h"

using namespace std;

namespace symbolic {
/**
 * A SimpleSymSolutionCut is a SymSolutionCut that keeps track of its visited
 * states.
 */
class SimpleSymSolutionCut : public SymSolutionCut {
protected:
    BDD visited_states;

public:
    SimpleSymSolutionCut(); // dummy for no solution
    SimpleSymSolutionCut(int g, int h, BDD cut, int sol_cost, BDD visited_states);
    SimpleSymSolutionCut(SymSolutionCut cut, BDD visited_states);

    BDD get_visited_states() const;
    void set_visited_states(BDD visited_states);


    friend std::ostream &operator<<(std::ostream &os,
                                    const SimpleSymSolutionCut &sym_cut) {
        return os << "symcut{g=" << sym_cut.get_g() << ", h=" << sym_cut.get_h()
                  << ", f=" << sym_cut.get_f()
                  << ", nodes=" << sym_cut.get_cut().nodeCount()
                  << ", visited=" << sym_cut.get_visited_states().nodeCount()
                  << "}";
    }
};
} // namespace symbolic
#endif
