#ifdef DDLIBBUDDY

#include "bdd_buddy.h"
#include "bdd_tmpl.h"
#include <iostream>
#include <limits>

namespace symbolic
{

utils::Timer Bdd::timer;
double Bdd::time_limit = std::numeric_limits<double>::infinity();

Bdd Bdd::BddZero() { return Bdd(bdd_false()); }

Bdd Bdd::BddOne() { return Bdd(bdd_true()); }

Bdd Bdd::BddVar(int i) { return Bdd(bdd_ithvar(i)); }

void Bdd::initalize_manager(unsigned int numVars, unsigned int /*numVarsZ*/,
                            unsigned int /*numSlots*/, unsigned int /*cacheSize*/,
                            unsigned long /*maxMemory*/)
{
    int num_nodes = 180000000; // ~3.8 GB
        bdd_init(num_nodes, num_nodes / 64);
    bdd_setvarnum(numVars);
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

Bdd::Bdd(const Bdd &other) { this->buddy_bdd = other.get_lib_bdd(); }

Bdd::Bdd(bdd buddy_bdd) { this->buddy_bdd = buddy_bdd; }

bdd Bdd::get_lib_bdd() const { return buddy_bdd; }

Bdd Bdd::operator=(const Bdd &right) { return Bdd(buddy_bdd = right.get_lib_bdd()); }

bool Bdd::operator==(const Bdd &other) const
{
    return buddy_bdd == other.get_lib_bdd();
}

bool Bdd::operator!=(const Bdd &other) const
{
    return buddy_bdd != other.get_lib_bdd();
}

/*bool Bdd::operator<=(const Bdd &other) const
{
    return buddy_bdd <= other.get_lib_bdd();
}

bool Bdd::operator>=(const Bdd &other) const
{
    return buddy_bdd >= other.get_lib_bdd();
}

bool Bdd::operator<(const Bdd &other) const
{
    return buddy_bdd < other.get_lib_bdd();
}

bool Bdd::operator>(const Bdd &other) const
{
    return buddy_bdd > other.get_lib_bdd();
}*/

Bdd Bdd::operator!() const { 
    check_time_limit();
    return Bdd(!buddy_bdd); 
}

Bdd Bdd::operator~() const { 
    check_time_limit();
    return Bdd(!buddy_bdd); 
}

Bdd Bdd::operator*(const Bdd &other) const
{
    check_time_limit();
    return Bdd(buddy_bdd & other.get_lib_bdd());
}

Bdd Bdd::operator*=(const Bdd &other)
{
    check_time_limit();
    buddy_bdd &= other.get_lib_bdd();
    return *this;
}

Bdd Bdd::operator&(const Bdd &other) const
{
    check_time_limit();
    return Bdd(buddy_bdd & other.get_lib_bdd());
}

Bdd Bdd::operator&=(const Bdd &other)
{
    check_time_limit();
    buddy_bdd &= other.get_lib_bdd();
    return *this;
}

Bdd Bdd::operator+(const Bdd &other) const
{
    check_time_limit();
    return Bdd(buddy_bdd | other.get_lib_bdd());
}

Bdd Bdd::operator+=(const Bdd &other)
{
    check_time_limit();
    buddy_bdd |= other.get_lib_bdd();
    return *this;
}

Bdd Bdd::operator|(const Bdd &other) const
{
    check_time_limit();
    return Bdd(buddy_bdd | other.get_lib_bdd());
}

Bdd Bdd::operator|=(const Bdd &other)
{
    check_time_limit();
    buddy_bdd |= other.get_lib_bdd();
    return *this;
}

Bdd Bdd::operator^(const Bdd &other) const
{
    check_time_limit();
    return Bdd(buddy_bdd ^ other.get_lib_bdd());
}

Bdd Bdd::operator^=(const Bdd &other)
{
    check_time_limit();
    buddy_bdd ^= other.get_lib_bdd();
    return *this;
}

Bdd Bdd::operator-(const Bdd &other) const
{
    check_time_limit();
    return Bdd(buddy_bdd - other.get_lib_bdd());
}

Bdd Bdd::operator-=(const Bdd &other)
{
    check_time_limit();
    buddy_bdd -= other.get_lib_bdd();
    return *this;
}

Bdd Bdd::Xnor(const Bdd &other) const
{
    check_time_limit();
    return Bdd(bdd_apply(buddy_bdd, other.get_lib_bdd(), bddop_biimp));
}

Bdd Bdd::Or(const Bdd &other, int maxNodes) const
{
    if (maxNodes > 0 && (nodeCount() > maxNodes || other.nodeCount() > maxNodes)) {
        exceptionError();
    }
    check_time_limit();
    Bdd result = *this + other;
    if (maxNodes > 0 && result.nodeCount() > maxNodes)
    {
        exceptionError();
    }
    return result;
}

Bdd Bdd::And(const Bdd &other, int maxNodes) const
{
    if (maxNodes > 0 && (nodeCount() > maxNodes || other.nodeCount() > maxNodes)) {
        exceptionError();
    }
    check_time_limit();
    Bdd result = *this * other;
    if (maxNodes > 0 && result.nodeCount() > maxNodes)
    {
        exceptionError();
    }
    return result;
}

Bdd Bdd::SwapVariables(const std::vector<Bdd>& x, const std::vector<Bdd>& y) const
{
    check_time_limit();
    auto pair = bdd_newpair();
    for (size_t i = 0; i < x.size(); i++)
    {
        int x_var = bdd_var(x[i].get_lib_bdd());
        int y_var = bdd_var(y[i].get_lib_bdd());
        bdd_setpair(pair, x_var, y_var);
    }
    Bdd result = bdd_replace(buddy_bdd, pair);
    bdd_freepair(pair);
    return result;
}

int Bdd::nodeCount() const { return bdd_nodecount(buddy_bdd); }

bool Bdd::IsZero() const { return bdd_false() == buddy_bdd; }

bool Bdd::IsOne() const { return bdd_true() == buddy_bdd; }

Bdd Bdd::AndAbstract(const Bdd &g, const Bdd &cube, int limit) const
{
    if (limit > 0 && (nodeCount() > limit || g.nodeCount() > limit)) {
        exceptionError();
    }
    check_time_limit();
    Bdd result = Bdd(bdd_appex(buddy_bdd, g.get_lib_bdd(), bddop_and, cube.get_lib_bdd()));
    if (limit > 0 && result.nodeCount() > limit)
    {
        exceptionError();
    }
    return result;
}

Bdd Bdd::RelationProductNext(const Bdd &relation, const Bdd &cube,
                             const std::vector<Bdd> &pre_vars,
                             const std::vector<Bdd> &succ_vars,
                             int max_nodes) const
{
    check_time_limit();
    Bdd tmp = AndAbstract(relation, cube, max_nodes);
    return tmp.SwapVariables(pre_vars, succ_vars);
}

Bdd Bdd::RelationProductPrev(const Bdd &relation, const Bdd &cube,
                             const std::vector<Bdd> &pre_vars,
                             const std::vector<Bdd> &succ_vars,
                             int max_nodes) const
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

void Bdd::toDot(const std::string &/*file_name*/) const
{
    std::cout << "NOT IMPLEMENTED" << std::endl;
    exit(0);
}

} // namespace symbolic

#endif
