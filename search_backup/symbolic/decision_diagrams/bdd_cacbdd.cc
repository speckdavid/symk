#ifdef DDLIBCACBDD

#include "bdd_cacbdd.h"
#include "bdd_tmpl.h"
#include <iostream>
#include <numeric>
#include <limits>

namespace symbolic
{

int Bdd::num_vars = 0;
std::unique_ptr<XBDDManager> Bdd::manager = nullptr;
utils::Timer Bdd::timer;
double Bdd::time_limit = std::numeric_limits<double>::infinity();

Bdd Bdd::BddZero() { return Bdd(manager->BddZero()); }

Bdd Bdd::BddOne() { return Bdd(manager->BddOne()); }

Bdd Bdd::BddVar(int i) { return Bdd(manager->BddVar(i + 1)); }

void Bdd::initalize_manager(unsigned int numVars, unsigned int /*numVarsZ*/,
                            unsigned int /*numSlots*/, unsigned int /*cacheSize*/,
                            unsigned long /*maxMemory*/)
{
    std::cout << "DD-lib: CacBDD" << std::endl;
    manager = std::unique_ptr<XBDDManager>(
        new XBDDManager(numVars));
    num_vars = numVars;
}

void Bdd::unset_time_limit() {
    time_limit = std::numeric_limits<double>::infinity();
}

void Bdd::reset_start_time() {
    timer.reset();
}

void Bdd::set_time_limit(unsigned long tl) {
    time_limit = tl / 1000.0;
}

Bdd::Bdd() {}

Bdd::Bdd(const Bdd &bdd) { this->bdd = bdd.get_lib_bdd(); }

Bdd::Bdd(BDD bdd) { this->bdd = bdd; }

BDD Bdd::get_lib_bdd() const { return bdd; }

Bdd Bdd::operator=(const Bdd &right) { return Bdd(bdd = right.get_lib_bdd()); }

bool Bdd::operator==(const Bdd &other) const
{
    return bdd == other.get_lib_bdd();
}

bool Bdd::operator!=(const Bdd &other) const
{
    return bdd != other.get_lib_bdd();
}

/*bool Bdd::operator<=(const Bdd &other) const
{
  return bdd <= other.get_lib_bdd();
}

bool Bdd::operator>=(const Bdd &other) const
{
  return bdd >= other.get_lib_bdd();
}

bool Bdd::operator<(const Bdd &other) const
{
  return bdd < other.get_lib_bdd();
}

bool Bdd::operator>(const Bdd &other) const
{
  return bdd > other.get_lib_bdd();
}*/

Bdd Bdd::operator!() const { 
    check_time_limit();
    return Bdd(!bdd);
}

Bdd Bdd::operator~() const { 
    check_time_limit();
    return Bdd(!bdd);
}

Bdd Bdd::operator*(const Bdd &other) const
{
    check_time_limit();
    return Bdd(bdd * other.get_lib_bdd());
}

Bdd Bdd::operator*=(const Bdd &other)
{
    check_time_limit();
    bdd = bdd * other.get_lib_bdd();
    return *this;
}

Bdd Bdd::operator&(const Bdd &other) const
{
    check_time_limit();
    return Bdd(bdd * other.get_lib_bdd());
}

Bdd Bdd::operator&=(const Bdd &other)
{
    check_time_limit();
    bdd = bdd * other.get_lib_bdd();
    return *this;
}

Bdd Bdd::operator+(const Bdd &other) const
{
    check_time_limit();
    return Bdd(bdd + other.get_lib_bdd());
}

Bdd Bdd::operator+=(const Bdd &other)
{
    check_time_limit();
    bdd = bdd + other.get_lib_bdd();
    return *this;
}

Bdd Bdd::operator|(const Bdd &other) const
{
    check_time_limit();
    return Bdd(bdd + other.get_lib_bdd());
}

Bdd Bdd::operator|=(const Bdd &other)
{
    check_time_limit();
    bdd = bdd + other.get_lib_bdd();
    return *this;
}

Bdd Bdd::operator^(const Bdd &other) const
{
    check_time_limit();
    return Bdd(bdd ^ other.get_lib_bdd());
}

Bdd Bdd::operator^=(const Bdd &other)
{
    check_time_limit();
    bdd = bdd ^ other.get_lib_bdd();
    return *this;
}

Bdd Bdd::operator-(const Bdd &other) const
{
    check_time_limit();
    return (*this) * (!other);
}

Bdd Bdd::operator-=(const Bdd &other)
{
    check_time_limit();
    bdd = (*this - other).get_lib_bdd();
    return *this;
}

Bdd Bdd::Xnor(const Bdd &other) const
{
    check_time_limit();
    return Bdd(bdd & other.get_lib_bdd());
}

Bdd Bdd::Or(const Bdd &other, int /*maxNodes*/) const
{
    check_time_limit();
    Bdd result = Bdd(bdd + other.get_lib_bdd());
    return result;
}

Bdd Bdd::And(const Bdd &other, int /*maxNodes*/) const
{
    check_time_limit();
    Bdd result = Bdd(bdd * other.get_lib_bdd());
    return result;
}

Bdd Bdd::SwapVariables(const std::vector<Bdd>& x, const std::vector<Bdd>& y) const
{
    check_time_limit();
    std::vector<int> swap_vars(num_vars + 1);
    std::iota(std::begin(swap_vars), std::end(swap_vars), 0);
    // swap_vars[0] = 0;
    for (size_t i = 0; i < x.size(); i++)
    {
        int x_level = x[i].get_lib_bdd().Variable();
        int y_level = y[i].get_lib_bdd().Variable();
        swap_vars[x_level] = y_level;
    }
    return Bdd(bdd.Permute(swap_vars));
}

int Bdd::nodeCount() const { return 0; }

bool Bdd::IsZero() const { return bdd == manager->BddZero(); }

bool Bdd::IsOne() const { return bdd == manager->BddOne(); }

Bdd Bdd::AndAbstract(const Bdd &g, const Bdd &cube, int /*limit*/)
{
    check_time_limit();
    return Bdd(bdd.AndExist(g.get_lib_bdd(), cube.get_lib_bdd()));
}

Bdd Bdd::RelationProductNext(const Bdd &relation, const Bdd &cube,
                             const std::vector<Bdd> &pre_vars,
                             const std::vector<Bdd> &succ_vars,
                             int max_nodes)
{
    check_time_limit();
    Bdd tmp = AndAbstract(relation, cube, max_nodes);
    return tmp.SwapVariables(pre_vars, succ_vars);
}

Bdd Bdd::RelationProductPrev(const Bdd &relation, const Bdd &cube,
                             const std::vector<Bdd> &pre_vars,
                             const std::vector<Bdd> &succ_vars,
                             int max_nodes)
{
    check_time_limit();
    Bdd tmp = SwapVariables(pre_vars, succ_vars);
    return tmp.AndAbstract(relation, cube, max_nodes);
}

void Bdd::check_time_limit() const {
    if (timer() > time_limit) {
        exceptionError();
    }
}

void Bdd::toDot(const std::string & /*file_name*/) const
{
    std::cout << "NOT IMPLEMENTED" << std::endl;
    exit(0);
}

} // namespace symbolic

#endif
