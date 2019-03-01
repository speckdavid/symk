// Copyright 05.07.2018, University of Freiburg,
// Author: David Speck <speckd>.

#include <meddly.h>
#include <meddly_expert.h>
#include <limits>
#include <cmath>
#include "pow.h"
#include "userOperations.h"

const MEDDLY::unary_opname *USER_OPS::POW;

namespace USER_OPS
{
class Pow;
class Pow_Opname;
} // namespace USER_OPS

class USER_OPS::Pow : public MEDDLY::unary_operation
{
public:
  Pow(const MEDDLY::unary_opname *code,
                    MEDDLY::expert_forest *arg, MEDDLY::expert_forest *res)
      : MEDDLY::unary_operation(code, 3, 2, arg, res), exp(2)
  {
  }

  void setExp(float exp)
  {
    this->exp = exp;
  }

  void discardEntry(const MEDDLY::node_handle *data)
  {
    argF->uncacheNode(data[2]);
    resF->uncacheNode(data[4]);
  }

  bool isStaleEntry(const MEDDLY::node_handle *data)
  {
    return argF->isStale(data[2]) || resF->isStale(data[4]);
  }

  void showEntry(MEDDLY::output &strm, const MEDDLY::node_handle* /*data*/) const
  {
    strm << "todo";
  }

  void computeDDEdge(const MEDDLY::dd_edge &a, MEDDLY::dd_edge &b)
  {
    MEDDLY::node_handle result;
    float av, ev;
    a.getEdgeValue(av);
    compute(av, a.getNode(), ev, result);
    b.set(result, ev);
  }

protected:
  float exp;

  MEDDLY::compute_table::search_key *findResult(float aev, MEDDLY::node_handle a, float &cv,
                                                MEDDLY::node_handle &c)
  {
    MEDDLY::compute_table::search_key *CTsrch = useCTkey();
    MEDDLY_DCASSERT(CTsrch);
    CTsrch->reset();
    CTsrch->write(exp);
    CTsrch->write(aev);
    CTsrch->writeNH(a);
    MEDDLY::compute_table::search_result &cacheFind = CT->find(CTsrch);
    if (!cacheFind)
      return CTsrch;
    cacheFind.read(cv);
    c = resF->linkNode(cacheFind.readNH());
    doneCTkey(CTsrch);
    return 0;
  }

  void saveResult(MEDDLY::compute_table::search_key *Key, MEDDLY::node_handle a,
                  float cv, MEDDLY::node_handle c)
  {
    argF->cacheNode(a);
    MEDDLY::compute_table::entry_builder &entry = CT->startNewEntry(Key);
    entry.writeResult(cv);
    entry.writeResultNH(resF->cacheNode(c));
    CT->addEntry();
  }

  void compute(float av, MEDDLY::node_handle ap, float &rv, MEDDLY::node_handle &rp)
  {
    if (argF->isTerminalNode(ap))
    {
      rv = std::pow(av, exp);
      rp = ap;
      return;
    }
    // If we already visited the node we stop here
    MEDDLY::compute_table::search_key *Key = findResult(av, ap, rv, rp);
    if (0 == Key)
    {
      return;
    }

    // Get level information
    int resultLevel = argF->getNodeLevel(ap);
    const int resultSize = resF->getLevelSize(resultLevel);

    MEDDLY::unpacked_node *A = MEDDLY::unpacked_node::newFromNode(argF, ap,
                                                                  true);

    // Initialize result
    MEDDLY::unpacked_node *nb = MEDDLY::unpacked_node::newFull(resF,
                                                               resultLevel, resultSize);

    // Recursive call
    float ev;
    MEDDLY::node_handle ep;
    for (int i = 0; i < resultSize; i++)
    {
      compute(A->ef(i) + av, A->d(i), ev, ep);
      nb->d_ref(i) = ep;
      nb->setEdge(i, ev);
    }

    MEDDLY::unpacked_node::recycle(A);

    // Create a node handle...links cl to nb with cost rv
    MEDDLY::node_handle cl;
    resF->createReducedNode(-1, nb, rv, cl);
    rp = cl;

    // Add to CT
    saveResult(Key, ap, rv, rp);
  }
};

class USER_OPS::Pow_Opname : public MEDDLY::unary_opname
{
public:
  Pow_Opname() : unary_opname("ParitalComplement")
  {
  }

  MEDDLY::unary_operation *buildOperation(MEDDLY::expert_forest *arg1,
                                          MEDDLY::expert_forest *res) const
  {
    return new USER_OPS::Pow(this, arg1, res);
  }
};

void USER_OPS::initializePow()
{
  USER_OPS::POW = new USER_OPS::Pow_Opname;
}

void USER_OPS::setPowExp(const MEDDLY::dd_edge &arg, const MEDDLY::dd_edge &res,
                                 float exp)
{
  Pow *powOP = reinterpret_cast<Pow *>(MEDDLY::getOperation(
      POW, arg, res));
  powOP->setExp(exp);
  powOP = nullptr;
}
