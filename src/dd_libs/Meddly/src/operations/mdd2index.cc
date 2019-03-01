
// $Id$

/*
    Meddly: Multi-terminal and Edge-valued Decision Diagram LibrarY.
    Copyright (C) 2009, Iowa State University Research Foundation, Inc.

    This library is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published 
    by the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "../defines.h"
#include "mdd2index.h"

// #define TRACE_ALL_OPS

namespace MEDDLY {
  class mdd2index_operation;
  class mdd2index_opname;
};

// ******************************************************************
// *                                                                *
// *                   mdd2index_operation  class                   *
// *                                                                *
// ******************************************************************

class MEDDLY::mdd2index_operation : public unary_operation {
  public:
    mdd2index_operation(const unary_opname* oc, expert_forest* arg, 
      expert_forest* res);

    virtual bool isStaleEntry(const node_handle* entryData);
    virtual void discardEntry(const node_handle* entryData);
    virtual void showEntry(output &strm, const node_handle* entryData) const;

    virtual void computeDDEdge(const dd_edge &arg, dd_edge &res);

    void compute_r(int k, node_handle a, node_handle &bdn, int &bcard);
};

MEDDLY::mdd2index_operation::mdd2index_operation(const unary_opname* oc, 
  expert_forest* arg, expert_forest* res)
: unary_operation(oc, 1, 2, arg, res)
{
  // answer[0] : pointer
  // answer[1] : cardinality
}

bool 
MEDDLY::mdd2index_operation
::isStaleEntry(const node_handle* entryData)
{
  return 
    argF->isStale(entryData[0]) ||
    resF->isStale(entryData[1]);
}

void 
MEDDLY::mdd2index_operation
::discardEntry(const node_handle* entryData)
{
  argF->uncacheNode(entryData[0]);
  resF->uncacheNode(entryData[1]);
}

void 
MEDDLY::mdd2index_operation
::showEntry(output &strm, const node_handle* entryData) const
{
  strm << "[" << getName() << " " << long(entryData[0]) << " "
       << long(entryData[1]) << " (card " << long(entryData[2]) << ")]";
}

void
MEDDLY::mdd2index_operation
::computeDDEdge(const dd_edge &arg, dd_edge &res)
{
  MEDDLY_DCASSERT(arg.getForest() == argF);
  MEDDLY_DCASSERT(res.getForest() == resF);
  MEDDLY_DCASSERT(resF->handleForValue(false) == 0);
  MEDDLY_DCASSERT(argF->handleForValue(true) < 0);
  MEDDLY_DCASSERT(argF->handleForValue(false) == 0);
  node_handle down;
  int card;
  int nVars = argF->getDomain()->getNumVariables();
  compute_r(nVars, arg.getNode(), down, card);
  res.set(down);
}

void
MEDDLY::mdd2index_operation
::compute_r(int k, node_handle a, node_handle &bdn, int &bcard)
{
  // Deal with terminals
  if (0 == a) {
    bdn = 0;
    bcard = 0;
    return;
  }
  if (0 == k) {
    bdn = expert_forest::bool_Tencoder::value2handle(true);
    bcard = 1;
    return;
  }

  int aLevel = argF->getNodeLevel(a);
  MEDDLY_DCASSERT(aLevel <= k);

  // Check compute table
  compute_table::search_key* CTsrch = 0;
  if (aLevel == k) {
    CTsrch = useCTkey();
    MEDDLY_DCASSERT(CTsrch);
    CTsrch->reset();
    CTsrch->writeNH(a);
    compute_table::search_result &cacheEntry = CT->find(CTsrch);
    if (cacheEntry) {
      bdn = resF->linkNode(cacheEntry.readNH());
      cacheEntry.read(bcard);
      doneCTkey(CTsrch);
      return;
    }
  }

#ifdef TRACE_ALL_OPS
  printf("calling mdd2index::compute(%d, %d)\n", height, a);
#endif

  // Initialize node builder
  const int size = resF->getLevelSize(k);
  unpacked_node* nb = unpacked_node::newFull(resF, k, size);
  
  // Initialize node reader
  unpacked_node *A = unpacked_node::useUnpackedNode();
  if (aLevel < k) {
    A->initRedundant(argF, k, a, true);
  } else {
    A->initFromNode(argF, a, true);
  }

  // recurse
  bcard = 0;
  for (int i=0; i<size; i++) {
    node_handle ddn;
    int dcard;
    compute_r(k-1, A->d(i), ddn, dcard);
    nb->d_ref(i) = ddn;
    if (ddn) {
      nb->setEdge(i, bcard);
      bcard += dcard;
    } else {
      MEDDLY_DCASSERT(0 == dcard);
      nb->setEdge(i, 0);
    }
  }

  // Cleanup
  unpacked_node::recycle(A);

  // Reduce
  memcpy(nb->UHdata(), &bcard, sizeof(bcard));

  int dummy;
  node_handle bl;
  resF->createReducedNode(-1, nb, dummy, bl);
  bdn = bl;
  MEDDLY_DCASSERT(0==dummy);

  // Add to compute table
  if (CTsrch) {
    argF->cacheNode(a);
    compute_table::entry_builder &entry = CT->startNewEntry(CTsrch);
    // entry.writeKeyNH(argF->cacheNode(a));
    entry.writeResultNH(resF->cacheNode(bdn));
    entry.writeResult(bcard);
    CT->addEntry();
  }
}

// ******************************************************************
// *                                                                *
// *                     mdd2index_opname class                     *
// *                                                                *
// ******************************************************************

class MEDDLY::mdd2index_opname : public unary_opname {
  public:
    mdd2index_opname();
    virtual unary_operation* 
      buildOperation(expert_forest* ar, expert_forest* res) const;
};

MEDDLY::mdd2index_opname::mdd2index_opname()
 : unary_opname("ConvertToIndexSet")
{
}

MEDDLY::unary_operation*
MEDDLY::mdd2index_opname
::buildOperation(expert_forest* arg, expert_forest* res) const
{
  if (0==arg || 0==res) return 0;

  if (arg->getDomain() != res->getDomain())
    throw error(error::DOMAIN_MISMATCH);

  if (arg->isForRelations() || 
      arg->getRangeType() != forest::BOOLEAN ||
      arg->getEdgeLabeling() != forest::MULTI_TERMINAL ||
      res->isForRelations() ||
      res->getRangeType() != forest::INTEGER ||
      res->getEdgeLabeling() != forest::INDEX_SET
  ) throw error(error::TYPE_MISMATCH);

  return new mdd2index_operation(this, arg, res);
}

// ******************************************************************
// *                                                                *
// *                           Front  end                           *
// *                                                                *
// ******************************************************************

MEDDLY::unary_opname* MEDDLY::initializeMDD2INDEX()
{
  return new mdd2index_opname;
}

