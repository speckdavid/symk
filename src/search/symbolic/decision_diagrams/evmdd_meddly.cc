#ifdef DDLIBMEDDLY

#include "evmdd_meddly.h"
#include "../../../dd_libs/Meddly/operations_extension/userOperations.h"
#include "bdd_tmpl.h"
#include <iostream>
#include <limits>

namespace symbolic
{
int Bdd::num_vars = -1;
MEDDLY::domain *Bdd::domain = nullptr;
MEDDLY::forest *Bdd::forest = nullptr;
MEDDLY::expert_forest *Bdd::exp_forest = nullptr;
utils::Timer Bdd::timer;
double Bdd::time_limit = std::numeric_limits<double>::infinity();

Bdd Bdd::BddZero()
{
    MEDDLY::dd_edge c(forest);
    c.set(-1, 0.0f);
    return Bdd(c);
}

Bdd Bdd::BddOne()
{
    MEDDLY::dd_edge c(forest);
    c.set(-1, 1.0f);
    return Bdd(c);
}

Bdd Bdd::BddVar(int i)
{
    MEDDLY::dd_edge e(forest);
    forest->createEdgeForVar(num_vars - i, false, e);
    return Bdd(e);
}

void Bdd::initalize_manager(unsigned int numVars, unsigned int /*numVarsZ*/,
                            unsigned int /*numSlots*/, unsigned int /*cacheSize*/,
                            unsigned long /*maxMemory*/)
{
    num_vars = numVars;
    try
    {
        USER_OPS::initializeUserOperations();
        MEDDLY::initialize();
        MEDDLY::variable **vars = new MEDDLY::variable *[numVars + 1];
        vars[0] = 0;
        for (size_t lev = 0; lev < numVars; lev++)
        {
            vars[lev + 1] = MEDDLY::createVariable(
                2, strdup(("lev-" + std::to_string(num_vars - lev - 1)).c_str()));
        }
        domain = MEDDLY::createDomain(vars, numVars);
        forest = domain->createForest(false, MEDDLY::forest::REAL,
                                      MEDDLY::forest::EVPLUS);
        forest->getPolicies().setPessimistic();
        exp_forest = static_cast<MEDDLY::expert_forest *>(forest);
        std::cout << "DD-lib: MEDDLY" << std::endl;
    }
    catch (const MEDDLY::error &e)
    {
        std::cout << e.getName() << std::endl;
    }
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

Bdd::Bdd(const Bdd &other) { this->dd_edge = other.get_lib_bdd(); }

Bdd::Bdd(const MEDDLY::dd_edge &dd_edge) { this->dd_edge = dd_edge; }

MEDDLY::dd_edge Bdd::get_lib_bdd() const { return dd_edge; }

bool Bdd::operator==(const Bdd &other) const
{
    return dd_edge == other.get_lib_bdd();
}

bool Bdd::operator!=(const Bdd &other) const
{
    return dd_edge != other.get_lib_bdd();
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

Bdd Bdd::operator!() const
{
    check_time_limit();
    MEDDLY::dd_edge res(forest);
    res.set(-1, 1.0f);
    MEDDLY::apply(USER_OPS::MINUS, res, dd_edge, res);
    return Bdd(res);
}

Bdd Bdd::operator~() const { 
    check_time_limit();
    return !(*this); 
}

Bdd Bdd::operator*(const Bdd &other) const
{
    check_time_limit();
    MEDDLY::dd_edge res(forest);
    MEDDLY::apply(USER_OPS::UNIONMIN, dd_edge, other.get_lib_bdd(), res);
    return Bdd(res);
}

Bdd Bdd::operator*=(const Bdd &other)
{
    check_time_limit();
    MEDDLY::apply(USER_OPS::UNIONMIN, dd_edge, other.get_lib_bdd(), dd_edge);
    return *this;
}

Bdd Bdd::operator&(const Bdd &other) const
{
    check_time_limit();
    MEDDLY::dd_edge res(forest);
    MEDDLY::apply(USER_OPS::UNIONMIN, dd_edge, other.get_lib_bdd(), res);
    return Bdd(res);
}

Bdd Bdd::operator&=(const Bdd &other)
{
    check_time_limit();
    MEDDLY::apply(USER_OPS::UNIONMIN, dd_edge, other.get_lib_bdd(), dd_edge);
    return *this;
}

Bdd Bdd::operator+(const Bdd &other) const
{
    check_time_limit();
    MEDDLY::dd_edge res(forest);
    MEDDLY::apply(USER_OPS::INTERSECTIONMAX, dd_edge, other.get_lib_bdd(), res);
    return Bdd(res);
}

Bdd Bdd::operator+=(const Bdd &other)
{
    check_time_limit();
    MEDDLY::apply(USER_OPS::INTERSECTIONMAX, dd_edge, other.get_lib_bdd(),
                  dd_edge);
    return *this;
}

Bdd Bdd::operator|(const Bdd &other) const
{
    check_time_limit();
    MEDDLY::dd_edge res(forest);
    MEDDLY::apply(USER_OPS::INTERSECTIONMAX, dd_edge, other.get_lib_bdd(), res);
    return Bdd(res);
}

Bdd Bdd::operator|=(const Bdd &other)
{
    check_time_limit();
    MEDDLY::apply(USER_OPS::INTERSECTIONMAX, dd_edge, other.get_lib_bdd(),
                  dd_edge);
    return *this;
}

/*Bdd Bdd::operator^(const Bdd &other) const
{
    return Bdd(buddy_bdd ^ other.get_lib_bdd());
}

Bdd Bdd::operator^=(const Bdd &other)
{
    buddy_bdd ^= other.get_lib_bdd();
    return *this;
}*/

Bdd Bdd::operator-(const Bdd &other) const { 
    check_time_limit();
    return (*this) * (!other); 
}

Bdd Bdd::operator-=(const Bdd &other)
{
    check_time_limit();
    dd_edge = (*this - other).get_lib_bdd();
    return *this;
}

Bdd Bdd::Xnor(const Bdd &other) const
{
    check_time_limit();
    MEDDLY::dd_edge res(forest);
    MEDDLY::apply(USER_OPS::EQUALS, dd_edge, other.get_lib_bdd(), res);
    return Bdd(res);
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

Bdd Bdd::SwapVariables(const std::vector<Bdd> &x,
                       const std::vector<Bdd> &y) const
{
    check_time_limit();
    MEDDLY::dd_edge res = dd_edge;
    for (size_t i = 0; i < x.size(); i++)
    {
        int x_var = exp_forest->getNodeLevel(x.at(i).get_lib_bdd().getNode());
        int y_var = exp_forest->getNodeLevel(y.at(i).get_lib_bdd().getNode());
        USER_OPS::setSwapVar(res, res, x_var, y_var);
        MEDDLY::apply(USER_OPS::SWAPVAR, res, res);
    }
    return Bdd(res);
}

int Bdd::nodeCount() const { 
    return dd_edge.getNodeCount(); 
}

bool Bdd::IsZero() const { return Bdd::BddZero() == dd_edge; }

bool Bdd::IsOne() const { return Bdd::BddOne() == dd_edge; }

Bdd Bdd::ExistAbstract(const std::vector<Bdd> &cube, int maxNodes) const
{
    if (maxNodes > 0 && nodeCount() > maxNodes) {
        exceptionError();
    }
    check_time_limit();
    MEDDLY::dd_edge result = dd_edge;
    for (size_t i = 0; i < cube.size(); i++)
    {
        check_time_limit();
        int level = exp_forest->getNodeLevel(cube.at(i).get_lib_bdd().getNode());
        MEDDLY::dd_edge cur_false(forest);
        USER_OPS::setRestrictVarVal(result, cur_false, level, 0);
        MEDDLY::apply(USER_OPS::RESTRICT, result, cur_false);
        MEDDLY::dd_edge cur_true(forest);
        USER_OPS::setRestrictVarVal(result, cur_true, level, 1);
        MEDDLY::apply(USER_OPS::RESTRICT, result, cur_true);
        MEDDLY::apply(USER_OPS::INTERSECTIONMAX, cur_false, cur_true, result);
        if (maxNodes > 0 && static_cast<int>(result.getNodeCount()) > maxNodes)
        {
            exceptionError();
        }
    }
    return Bdd(result);
}

Bdd Bdd::RelationProductNext(const Bdd &relation, const Bdd & /*cube*/,
                             const std::vector<Bdd> &pre_vars,
                             const std::vector<Bdd> &succ_vars,
                             int max_nodes) const
{
    check_time_limit();
    Bdd tmp = this->And(relation, max_nodes);
    tmp = tmp.ExistAbstract(succ_vars, max_nodes);
    return tmp.SwapVariables(pre_vars, succ_vars);
}

Bdd Bdd::RelationProductPrev(const Bdd &relation, const Bdd & /*cube*/,
                             const std::vector<Bdd> &pre_vars,
                             const std::vector<Bdd> &succ_vars,
                             int max_nodes) const
{
    check_time_limit();
    Bdd tmp = this->SwapVariables(pre_vars, succ_vars);
    tmp = tmp.And(relation, max_nodes);
    return tmp.ExistAbstract(succ_vars, max_nodes);
}

void Bdd::check_time_limit() const {
    if (timer() > time_limit) {
        exceptionError();
    }
}

void Bdd::toDot(const std::string &file_name) const
{
    USER_OPS::to_dot(dd_edge, file_name);
}

} // namespace symbolic

#endif
