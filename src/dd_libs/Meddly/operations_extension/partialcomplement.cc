// Copyright 05.07.2018, University of Freiburg,
// Author: David Speck <speckd>.

#include <meddly.h>
#include <meddly_expert.h>
#include <limits>
#include "partialcomplement.h"
#include "userOperations.h"

const MEDDLY::unary_opname *USER_OPS::PARTIALCOMPLEMENT;

namespace USER_OPS
{
class PartialComplement;
class PartialComplement_Opname;
} // namespace USER_OPS

class USER_OPS::PartialComplement : public MEDDLY::unary_operation
{
public:
  PartialComplement(const MEDDLY::unary_opname *code,
                    MEDDLY::expert_forest *arg, MEDDLY::expert_forest *res)
      : MEDDLY::unary_operation(code, 1, 2, arg, res)
  {
  }

  void discardEntry(const MEDDLY::node_handle *data)
  {
    argF->uncacheNode(data[0]);
    resF->uncacheNode(data[2]);
  }

  bool isStaleEntry(const MEDDLY::node_handle *data)
  {
    return argF->isStale(data[0]) || resF->isStale(data[2]);
  }

  void showEntry(MEDDLY::output &strm, const MEDDLY::node_handle *data) const
  {
    strm << "[" << getName() << "(" << long(data[0]) << "): <" << long(data[1])
         << ", " << long(data[2]) << ">]";
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
  MEDDLY::compute_table::search_key *findResult(MEDDLY::node_handle a, float &cv,
                                                MEDDLY::node_handle &c)
  {
    MEDDLY::compute_table::search_key *CTsrch = useCTkey();
    MEDDLY_DCASSERT(CTsrch);
    CTsrch->reset();
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
      // Infinity cost become 0 and vise versa
      rv = av == INFTY ? 0 : INFTY;
      rp = ap;
      return;
    }
    // If we already visited the node we stop here
    MEDDLY::compute_table::search_key *Key = findResult(ap, rv, rp);
    if (0 == Key)
    {
      // If Link to terminal case => flip edge
      if (rp == -1)
      {
        rv = av == INFTY ? 0 : INFTY;
      }
      else
      {
        rv = 0;
      }
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
      compute(A->ef(i), A->d(i), ev, ep);
      nb->d_ref(i) = ep;
      ev = ev != INFTY ? 0 : ev;
      nb->setEdge(i, ev);
    }

    MEDDLY::unpacked_node::recycle(A);

    // Create a node handle...links cl to nb with cost rv
    MEDDLY::node_handle cl;
    resF->createReducedNode(-1, nb, rv, cl);
    rp = cl;

    // Add to CT
    saveResult(Key, ap, rv, rp);
    rv = std::max(rv, av) == INFTY ? INFTY : 0;
  }
};

class USER_OPS::PartialComplement_Opname : public MEDDLY::unary_opname
{
public:
  PartialComplement_Opname() : unary_opname("ParitalComplement")
  {
  }

  MEDDLY::unary_operation *buildOperation(MEDDLY::expert_forest *arg1,
                                          MEDDLY::expert_forest *res) const
  {
    return new USER_OPS::PartialComplement(this, arg1, res);
  }
};

void USER_OPS::initializePartialComplement()
{
  USER_OPS::PARTIALCOMPLEMENT = new USER_OPS::PartialComplement_Opname;
}
