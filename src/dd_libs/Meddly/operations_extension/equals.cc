#include "equals.h"
#include "userOperations.h"
#include <limits>
#include <meddly.h>
#include <meddly_expert.h>
#include <math.h>
#include <cmath>

const MEDDLY::binary_opname *USER_OPS::EQUALS;

namespace USER_OPS
{
class Equals;
class Equals_Opname;
} // namespace USER_OPS

class USER_OPS::Equals : public MEDDLY::binary_operation
{
public:
  Equals(const MEDDLY::binary_opname *code, MEDDLY::expert_forest *arg1,
         MEDDLY::expert_forest *arg2, MEDDLY::expert_forest *res)
      : binary_operation(code, 4, 2, arg1, arg2, res)
  {
    can_commute = true;
  }

  bool isStaleEntry(const MEDDLY::node_handle *entry)
  {
    return arg1F->isStale(entry[1]) || arg2F->isStale(entry[3]) ||
           resF->isStale(entry[5]);
  }

  void discardEntry(const MEDDLY::node_handle *entryData)
  {
    arg1F->uncacheNode(entryData[1]);
    arg2F->uncacheNode(entryData[3]);
    resF->uncacheNode(entryData[5]);
  }

  void showEntry(MEDDLY::output &strm, const MEDDLY::node_handle *data) const
  {
    strm << "[" << getName() << "(<" << long(data[0]) << ":" << long(data[1])
         << ">, <" << long(data[2]) << ":" << long(data[3]) << ">): <"
         << long(data[4]) << ":" << long(data[5]) << ">]";
  }

  void computeDDEdge(const MEDDLY::dd_edge &ar1, const MEDDLY::dd_edge &ar2,
                     MEDDLY::dd_edge &res)
  {
    MEDDLY::node_handle result;
    float ev, aev, bev;
    ar1.getEdgeValue(aev);
    ar2.getEdgeValue(bev);
    compute(aev, ar1.getNode(), bev, ar2.getNode(), ev, result);
    res.set(result, ev);
  }

protected:
  void compute(float aev, MEDDLY::node_handle a, float bev,
               MEDDLY::node_handle b, float &cev, MEDDLY::node_handle &c)
  {
    // std::cout << "<" << aev << ", " << a << ">, " << "<" << bev << ", " << b << "> " << std::endl;
    if (checkTerminals(aev, a, bev, b, cev, c))
      return;

    MEDDLY::compute_table::search_key *Key = findResult(aev, a, bev, b, cev, c);
    if (0 == Key)
    {
      // std::cout << "Cache hit" << std::endl;
      return;
    }
    // Get level information
    const int aLevel = arg1F->getNodeLevel(a);
    const int bLevel = arg2F->getNodeLevel(b);

    const int resultLevel = aLevel > bLevel ? aLevel : bLevel;
    const int resultSize = resF->getLevelSize(resultLevel);

    // Initialize result
    MEDDLY::unpacked_node *nb =
        MEDDLY::unpacked_node::newFull(resF, resultLevel, resultSize);

    // Initialize readers
    MEDDLY::unpacked_node *A =
        (aLevel < resultLevel)
            ? MEDDLY::unpacked_node::newRedundant(arg1F, resultLevel, 0, a,
                                                  true)
            : MEDDLY::unpacked_node::newFromNode(arg1F, a, true);

    MEDDLY::unpacked_node *B =
        (bLevel < resultLevel)
            ? MEDDLY::unpacked_node::newRedundant(arg2F, resultLevel, 0, b,
                                                  true)
            : MEDDLY::unpacked_node::newFromNode(arg2F, b, true);

    // do computation
    for (int i = 0; i < resultSize; i++)
    {
      float ev;
      MEDDLY::node_handle ed;
      compute(aev + A->ef(i), A->d(i), bev + B->ef(i), B->d(i), ev, ed);
      nb->d_ref(i) = ed;
      nb->setEdge(i, ev);
    }

    // cleanup
    MEDDLY::unpacked_node::recycle(B);
    MEDDLY::unpacked_node::recycle(A);

    // Reduce
    MEDDLY::node_handle cl;
    resF->createReducedNode(-1, nb, cev, cl);
    c = cl;

    // std::cout << a << "," << aev << " vs. " << b << "," << bev << " = " << c
    // << "," << cev << std::endl;

    // Add to CT
    saveResult(Key, a, b, cev, c);
  }

  // Check if something is a terminal
  bool checkTerminals(float aev, MEDDLY::node_handle a, float bev,
                      MEDDLY::node_handle b, float &cev,
                      MEDDLY::node_handle &c)
  {
    if (a == -1 && b == -1)
    {
      bool equal = std::fabs(aev - bev) < std::numeric_limits<float>::epsilon();
      c = -1;
      cev = equal ? 1 : 0;
      return true;
    }

    if (0 == a || 0 == b)
    {
      std::cout << "ERROR => Zero edge pointer in equals" << std::endl;
      return true;
    }
    return false;
  }

  MEDDLY::compute_table::search_key *
  findResult(float aev, MEDDLY::node_handle a, float bev, MEDDLY::node_handle b,
             float &cev, MEDDLY::node_handle &c)
  {
    MEDDLY::compute_table::search_key *CTsrch = useCTkey();
    MEDDLY_DCASSERT(CTsrch);
    CTsrch->reset();
    if (can_commute && a > b)
    {
      CTsrch->write(bev);
      CTsrch->writeNH(b);
      CTsrch->write(aev);
      CTsrch->writeNH(a);
    }
    else
    {
      CTsrch->write(aev);
      CTsrch->writeNH(a);
      CTsrch->write(bev);
      CTsrch->writeNH(b);
    }
    MEDDLY::compute_table::search_result &cacheFind = CT->find(CTsrch);
    if (!cacheFind)
      return CTsrch;
    cacheFind.read(cev);
    c = resF->linkNode(cacheFind.readNH());
    doneCTkey(CTsrch);
    return 0;
  }

  void saveResult(MEDDLY::compute_table::search_key *Key, MEDDLY::node_handle a,
                  MEDDLY::node_handle b, float cev, MEDDLY::node_handle c)
  {
    arg1F->cacheNode(a);
    arg2F->cacheNode(b);
    MEDDLY::compute_table::entry_builder &entry = CT->startNewEntry(Key);
    entry.writeResult(cev);
    entry.writeResultNH(resF->cacheNode(c));
    CT->addEntry();
  }
};

class USER_OPS::Equals_Opname : public MEDDLY::binary_opname
{
public:
  Equals_Opname() : binary_opname("EVEQUALS") {}

  MEDDLY::binary_operation *buildOperation(MEDDLY::expert_forest *arg1,
                                           MEDDLY::expert_forest *arg2,
                                           MEDDLY::expert_forest *res) const
  {
    return new USER_OPS::Equals(this, arg1, arg2, res);
  }
};

void USER_OPS::initializeEquals()
{
  USER_OPS::EQUALS = new USER_OPS::Equals_Opname();
}
