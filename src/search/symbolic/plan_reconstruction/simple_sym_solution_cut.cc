#include "simple_sym_solution_cut.h"

#include "../state_registry.h"
#include <vector>

using namespace std;

namespace symbolic {
SimpleSymSolutionCut::SimpleSymSolutionCut() : SymSolutionCut() {}

SimpleSymSolutionCut::SimpleSymSolutionCut(int g, int h, BDD cut,
                                           int sol_cost, BDD visited_states)
    : SymSolutionCut(g, h, cut, sol_cost), visited_states(visited_states) {}

SimpleSymSolutionCut::SimpleSymSolutionCut(SymSolutionCut cut, BDD visited_states)
    : SymSolutionCut(cut.get_g(), cut.get_h(), cut.get_cut(), cut.get_sol_cost()), visited_states(visited_states) {}


BDD SimpleSymSolutionCut::get_visited_states() const {
    return visited_states;
}

void SimpleSymSolutionCut::set_visited_states(BDD visited_states) {
    this->visited_states = visited_states;
}
} // namespace symbolic
