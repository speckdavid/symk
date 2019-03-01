// Copyright 05.07.2018, University of Freiburg,
// Author: David Speck <speckd>.

#include <meddly.h>
#include <meddly_expert.h>
#include <limits>
#include "restrict.h"
#include "userOperations.h"

const MEDDLY::unary_opname *USER_OPS::RESTRICT;

namespace USER_OPS
{
class Restrict;
class Restrict_Opname;
} // namespace USER_OPS

class USER_OPS::Restrict : public MEDDLY::unary_operation
{
public:
  Restrict(const MEDDLY::unary_opname *code,
           MEDDLY::expert_forest *arg, MEDDLY::expert_forest *res)
      : MEDDLY::unary_operation(code, 4, 2, arg, res), var(1), val(0)
  {
  }

  void setRestrict(int var, int val)
  {
    this->var = var;
    this->val = val;
  }

  void discardEntry(const MEDDLY::node_handle *data)
  {
    argF->uncacheNode(data[3]);
    resF->uncacheNode(data[5]);
  }

  bool isStaleEntry(const MEDDLY::node_handle *data)
  {
    return argF->isStale(data[3]) || resF->isStale(data[5]);
  }

  void showEntry(MEDDLY::output &strm, const MEDDLY::node_handle* /*data*/) const
  {
    strm << "to fill";
  }

  void computeDDEdge(const MEDDLY::dd_edge &a, MEDDLY::dd_edge &b)
  {
    MEDDLY::node_handle result;
    float av, ev;
    a.getEdgeValue(av);
    compute(av, a.getNode(), ev, result);
    if (result == 0)
    {
      ev = INFTY;
    }
    b.set(result, ev);
  }

protected:
  int var;
  int val;
  MEDDLY::compute_table::search_key *findResult(float av, MEDDLY::node_handle a, float &cv,
                                                MEDDLY::node_handle &c)
  {
    MEDDLY::compute_table::search_key *CTsrch = useCTkey();
    MEDDLY_DCASSERT(CTsrch);
    CTsrch->reset();
    // Also write var and val -> another restrict on the same edges with different
    // var and val look up wrong values!
    CTsrch->write(var);
    CTsrch->write(val);
    CTsrch->write(av);
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
    // is terminal node -> nothing to do
    if (argF->isTerminalNode(ap))
    {
      rv = av;
      rp = ap;
      if (ap == 0)
        rv = INFTY;
      return;
    }

    // If we already visited the node we stop here
    MEDDLY::compute_table::search_key *Key = findResult(av, ap, rv, rp);
    if (0 == Key)
    {
      return;
    }
    
    // Get level information
    const int resultLevel = argF->getNodeLevel(ap);
    const int resultSize = resF->getLevelSize(resultLevel);
    MEDDLY::unpacked_node *A = MEDDLY::unpacked_node::newFromNode(argF, ap,
                                                                  true);
    
    if (resultLevel < var)
    {
      rp = resF->linkNode(ap);
      rv = av;
      MEDDLY::unpacked_node::recycle(A);
      saveResult(Key, ap, rv, rp);
      return;
    }

    // Is a node which we want to restrict to a value
    if (resultLevel == var)
    {
      if (A->d(val) == -1)
      {
        rp = -1;
      }
      else
      {
        rp = resF->linkNode(A->d(val)); // this link is important
      }
      MEDDLY::unpacked_node::recycle(A);
      saveResult(Key, ap, A->ef(val) + av, rp); // We store the value of the edge
      rv = av + A->ef(val);
      return;
      // TODO(speckd): save the result here???
    }

    // Last If: resultLevel > var
    // Initialize result
    MEDDLY::unpacked_node *nb =
        MEDDLY::unpacked_node::newFull(resF, resultLevel, resultSize);
    float ev;
    MEDDLY::node_handle ep;
    for (int i = 0; i < resultSize; i++)
    {
      compute(A->ef(i) + av, A->d(i), ev, ep);
      nb->d_ref(i) = ep;
      nb->setEdge(i, ev);
    }
    MEDDLY::unpacked_node::recycle(A);
    // Create a node handle...links cl to nb with cost cv
    MEDDLY::node_handle cl;
    resF->createReducedNode(-1, nb, rv, cl);
    rp = cl;

    saveResult(Key, ap, rv, rp);
  }
};

class USER_OPS::Restrict_Opname : public MEDDLY::unary_opname
{
public:
  Restrict_Opname() : unary_opname("Restrict")
  {
  }

  MEDDLY::unary_operation *buildOperation(MEDDLY::expert_forest *arg1,
                                          MEDDLY::expert_forest *res) const
  {
    return new USER_OPS::Restrict(this, arg1, res);
  }
};

void USER_OPS::initializeRestrict()
{
  USER_OPS::RESTRICT = new USER_OPS::Restrict_Opname;
}

void USER_OPS::setRestrictVarVal(const MEDDLY::dd_edge &arg, const MEDDLY::dd_edge &res,
                                 int var, int val)
{
  Restrict *resOP = reinterpret_cast<Restrict *>(MEDDLY::getOperation(
      RESTRICT, arg, res));
  resOP->setRestrict(var, val);
  resOP = nullptr;
}
