
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
#include "../forests/mt.h"
#include "complement.h"

namespace MEDDLY {
  class compl_mdd;
  class compl_mxd;
  class compl_opname;
};

// #define DEBUG_MXD_COMPL

// ******************************************************************
// *                                                                *
// *                        compl_mdd  class                        *
// *                                                                *
// ******************************************************************

class MEDDLY::compl_mdd : public unary_operation {
  public:
    compl_mdd(const unary_opname* oc, expert_forest* arg, expert_forest* res);

    virtual bool isStaleEntry(const node_handle* entryData);
    virtual void discardEntry(const node_handle* entryData);
    virtual void showEntry(output &strm, const node_handle* entryData) const;
    virtual void computeDDEdge(const dd_edge& a, dd_edge& b);

  protected:
    node_handle compute_r(node_handle a);

    inline compute_table::search_key* 
    findResult(node_handle a, node_handle &b) 
    {
      compute_table::search_key* CTsrch = useCTkey();
      MEDDLY_DCASSERT(CTsrch);
      CTsrch->reset();
      CTsrch->writeNH(a);
      compute_table::search_result &cacheFind = CT->find(CTsrch);
      if (!cacheFind) return CTsrch;
      b = resF->linkNode(cacheFind.readNH());
      doneCTkey(CTsrch);
      return 0;
    }
    inline node_handle saveResult(compute_table::search_key* Key, 
      node_handle a, node_handle b) 
    {
      argF->cacheNode(a);
      compute_table::entry_builder &entry = CT->startNewEntry(Key);
      entry.writeResultNH(resF->cacheNode(b));
      CT->addEntry();
      return b;
    }
};

MEDDLY::compl_mdd
::compl_mdd(const unary_opname* oc, expert_forest* arg, expert_forest* res)
 : unary_operation(oc, 1, 1, arg, res)
{
  // ct entry 0: input node
  // ct entry 1: output node
}

bool MEDDLY::compl_mdd::isStaleEntry(const node_handle* data)
{
  return argF->isStale(data[0]) || resF->isStale(data[1]);
}

void MEDDLY::compl_mdd::discardEntry(const node_handle* data)
{
  argF->uncacheNode(data[0]);
  resF->uncacheNode(data[1]);
}

void MEDDLY::compl_mdd::showEntry(output &strm, const node_handle* data) const
{
  strm  << "[" << getName() << "(" << long(data[0]) 
        << "): " << long(data[1]) << "]";
}

void MEDDLY::compl_mdd::computeDDEdge(const dd_edge& a, dd_edge& b) 
{
  int result = compute_r(a.getNode());
  b.set(result);
}

MEDDLY::node_handle MEDDLY::compl_mdd::compute_r(node_handle a)
{
  // Check terminals
  if (argF->isTerminalNode(a)) {
    return expert_forest::bool_Tencoder::value2handle(
      !expert_forest::bool_Tencoder::handle2value(a)
    );
  }

  // Check compute table
  node_handle b;
  compute_table::search_key* Key = findResult(a, b);
  if (0==Key) return b;

  const int level = argF->getNodeLevel(a);
  const int size = resF->getLevelSize(level);
  bool addRedundentNode=(resF->isQuasiReduced() && level>1);

  // Initialize unpacked nodes
  unpacked_node* A = unpacked_node::newFromNode(argF, a, true);
  unpacked_node* C = unpacked_node::newFull(resF, level, size);

  // recurse
  for (int i=0; i<size; i++) {
    C->d_ref(i) = compute_r(A->d(i));

    if(addRedundentNode && resF->isTerminalNode(C->d(i)) && C->d(i)!=resF->getTransparentNode()){
    	C->d_ref(i)=((mt_forest*)resF)->makeNodeAtLevel(level-1, C->d(i));
    }
  }

  // cleanup and Reduce
  unpacked_node::recycle(A);
  b = resF->createReducedNode(-1, C);

  // Add to compute table
  return saveResult(Key, a, b);
}


// ******************************************************************
// *                                                                *
// *                        compl_mxd  class                        *
// *                                                                *
// ******************************************************************

class MEDDLY::compl_mxd : public unary_operation {
  public:
    compl_mxd(const unary_opname* oc, expert_forest* arg, expert_forest* res);

    virtual bool isStaleEntry(const node_handle* entryData);
    virtual void discardEntry(const node_handle* entryData);
    virtual void showEntry(output &strm, const node_handle* entryData) const;
    virtual void computeDDEdge(const dd_edge& a, dd_edge& b);

    node_handle compute_r(int in, int k, node_handle a);
};

MEDDLY::compl_mxd
::compl_mxd(const unary_opname* oc, expert_forest* arg, expert_forest* res)
 : unary_operation(oc, 2, 1, arg, res)
{
  // ct entry 0: level
  // ct entry 1: input node
  // ct entry 2: output node
}

bool MEDDLY::compl_mxd::isStaleEntry(const node_handle* data)
{
  return argF->isStale(data[1]) || resF->isStale(data[2]);
}

void MEDDLY::compl_mxd::discardEntry(const node_handle* data)
{
  argF->uncacheNode(data[1]);
  resF->uncacheNode(data[2]);
}

void MEDDLY::compl_mxd::showEntry(output &strm, const node_handle* data) const
{
  strm  << "[" << getName() << "(" << long(data[0]) << ", " << long(data[1])
        << "): " << long(data[2]) << "]";
}

void MEDDLY::compl_mxd::computeDDEdge(const dd_edge& a, dd_edge& b) 
{
  node_handle result = compute_r(-1, argF->getDomain()->getNumVariables(), a.getNode());
  b.set(result);
}

MEDDLY::node_handle MEDDLY::compl_mxd::compute_r(int in, int k, node_handle a)
{
  if (0==k) {
    return expert_forest::bool_Tencoder::value2handle(
      !expert_forest::bool_Tencoder::handle2value(a)
    );
  }
  if (argF->isTerminalNode(a) &&
      resF->isFullyReduced())
  {
    return expert_forest::bool_Tencoder::value2handle(
      !expert_forest::bool_Tencoder::handle2value(a)
    );
  }
  // Check compute table
  compute_table::search_key* CTsrch = useCTkey();
  MEDDLY_DCASSERT(CTsrch);
  CTsrch->reset();
  CTsrch->write(k);
  CTsrch->writeNH(a);
  compute_table::search_result &cacheFind = CT->find(CTsrch);
  if (cacheFind) {
    node_handle ans = cacheFind.readNH();
#ifdef DEBUG_MXD_COMPL
    fprintf(stderr, "\tin CT:   compl_mxd(%d, %d) : %d\n", ht, a, ans);
#endif
    doneCTkey(CTsrch);
    return resF->linkNode(ans);
  }

#ifdef DEBUG_MXD_COMPL
  fprintf(stderr, "\tstarting compl_mxd(%d, %d)\n", ht, a);
#endif

  // Initialize unpacked node
  const int size = resF->getLevelSize(k);
  const int aLevel = argF->getNodeLevel(a);
  MEDDLY_DCASSERT(!isLevelAbove(aLevel, k));
  unpacked_node* A = unpacked_node::useUnpackedNode();
  bool canSave = true;
  if (aLevel == k) {
    A->initFromNode(argF, a, true);
  } else if (k>0 || argF->isFullyReduced()) {
    A->initRedundant(argF, k, a, true);
  } else {
    MEDDLY_DCASSERT(in>=0);
    A->initIdentity(argF, k, in, a, true);
    canSave = false;
  }
  unpacked_node* C = unpacked_node::newFull(resF, aLevel, size);

  // recurse
  int nextLevel = argF->downLevel(k);
  int nnz = 0;
  bool addRedundentNode=(resF->isQuasiReduced() && (k>0 || k<-1));

  // recurse
  for (int i=0; i<size; i++) {
    C->d_ref(i) = compute_r(i, nextLevel, A->d(i));
    if (C->d(i)!=resF->getTransparentNode()) nnz++;

    if(addRedundentNode && resF->isTerminalNode(C->d(i)) && C->d(i)!=resF->getTransparentNode()){
      C->d_ref(i)=((mt_forest*)resF)->makeNodeAtLevel(nextLevel, C->d(i));
    }
  }

  // reduce, save in CT
  unpacked_node::recycle(A);
  node_handle result = resF->createReducedNode(in, C);
  if (k<0 && 1==nnz) canSave = false;
  if (canSave) {
    argF->cacheNode(a);
    compute_table::entry_builder &entry = CT->startNewEntry(CTsrch);
    entry.writeResultNH(resF->cacheNode(result));
    CT->addEntry();
  } else {
    doneCTkey(CTsrch);
  }
  return result;
}


// ******************************************************************
// *                                                                *
// *                       compl_opname class                       *
// *                                                                *
// ******************************************************************

class MEDDLY::compl_opname : public unary_opname {
  public:
    compl_opname();
    virtual unary_operation*
      buildOperation(expert_forest* ar, expert_forest* res) const;
};

MEDDLY::compl_opname::compl_opname()
 : unary_opname("Complement")
{
}

MEDDLY::unary_operation*
MEDDLY::compl_opname
::buildOperation(expert_forest* arg, expert_forest* res) const
{
  if (0==arg || 0==res) return 0;

  if (arg->getDomain() != res->getDomain())
    throw error(error::DOMAIN_MISMATCH);

  if (arg->getRangeType() != forest::BOOLEAN ||
      arg->getEdgeLabeling() != forest::MULTI_TERMINAL ||
      res->getRangeType() != forest::BOOLEAN ||
      res->getEdgeLabeling() != forest::MULTI_TERMINAL ||
      arg->isForRelations() != res->isForRelations()
  ) throw error(error::TYPE_MISMATCH);

  if (arg->isForRelations())
    return new compl_mxd(this,  arg,  res);
  else
    return new compl_mdd(this,  arg,  res);
}

// ******************************************************************
// *                                                                *
// *                           Front  end                           *
// *                                                                *
// ******************************************************************

MEDDLY::unary_opname* MEDDLY::initializeComplement()
{
  return new compl_opname;
}

