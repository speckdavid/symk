#include "sym_solution_cut.h"

#include "../state_registry.h"
#include <vector>

using namespace std;

namespace symbolic {
SymSolutionCut::SymSolutionCut() : g(-1), h(-1), sol_cost(-1) {}

SymSolutionCut::SymSolutionCut(int g, int h, BDD cut, int sol_cost) :
    g(g),
    h(h),
    cut(cut),
    sol_cost(sol_cost) {}

int SymSolutionCut::get_g() const {return g;}

int SymSolutionCut::get_h() const {return h;}

int SymSolutionCut::get_f() const {return g + h;}

BDD SymSolutionCut::get_cut() const {return cut;}

int SymSolutionCut::get_sol_cost() const {return sol_cost;}

void SymSolutionCut::merge(const SymSolutionCut &other) {
    assert(*this == other);
    cut += other.get_cut();
}

void SymSolutionCut::set_g(int g) {this->g = g;}

void SymSolutionCut::set_h(int h) {this->h = h;}

void SymSolutionCut::set_cut(BDD cut) {this->cut = cut;}

bool SymSolutionCut::operator<(const SymSolutionCut &other) const {
    bool result = get_f() < other.get_f();
    result |= (get_f() == other.get_f() && get_g() < other.get_g());
    return result;
}

bool SymSolutionCut::operator>(const SymSolutionCut &other) const {
    bool result = get_f() > other.get_f();
    result |= (get_f() == other.get_f() && get_g() > other.get_g());
    return result;
}

bool SymSolutionCut::operator==(const SymSolutionCut &other) const {
    return get_g() == other.get_g() && get_h() == other.get_h();
}

bool SymSolutionCut::operator!=(const SymSolutionCut &other) const {
    return !(get_g() == other.get_g() && get_h() == other.get_h());
}
} // namespace symbolic
