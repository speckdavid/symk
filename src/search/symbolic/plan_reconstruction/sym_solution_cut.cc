#include "sym_solution_cut.h"

#include "../state_registry.h"
#include <vector>

using namespace std;

namespace symbolic {
SymSolutionCut::SymSolutionCut() :
    g(-1),
    h(-1),
    util(-1) {}

SymSolutionCut::SymSolutionCut(int g, int h, int util, BDD cut) :
    g(g),
    h(h),
    util(util),
    cut(cut) {}


SymSolutionCut::SymSolutionCut(int g, int h, BDD cut) :
    SymSolutionCut(g, h, -1, cut) {}

int SymSolutionCut::get_g() const {return g;}

int SymSolutionCut::get_h() const {return h;}

int SymSolutionCut::get_f() const {return g + h;}

int SymSolutionCut::get_util() const {return util;}

BDD SymSolutionCut::get_cut() const {return cut;}

void SymSolutionCut::merge(const SymSolutionCut &other) {
    assert(*this == other);
    cut += other.get_cut();
}

void SymSolutionCut::set_g(int g) {this->g = g;}

void SymSolutionCut::set_h(int h) {this->h = h;}

void SymSolutionCut::set_util(int util) {this->util = util;}

void SymSolutionCut::set_cut(BDD cut) {this->cut = cut;}

bool SymSolutionCut::operator<(const SymSolutionCut &other) const {
    if (get_util() > other.get_util())
        return true;
    if (get_util() < other.get_util())
        return false;
    if (get_f() < other.get_f())
        return true;
    if (get_f() > other.get_f())
        return false;
    if (get_g() < other.get_g())
        return true;
    return false;
}

bool SymSolutionCut::operator>(const SymSolutionCut &other) const {
    return !(*this < other);
}

bool SymSolutionCut::operator==(const SymSolutionCut &other) const {
    return get_util() == other.get_util() && get_g() == other.get_g() && get_h() == other.get_h();
}

bool SymSolutionCut::operator!=(const SymSolutionCut &other) const {
    return !(*this == other);
}
} // namespace symbolic
