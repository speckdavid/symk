
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
#include "apply_base.h"

// #define TRACE_ALL_OPS
// #define DISABLE_CACHE

// ******************************************************************
// *                                                                *
// *                   generic_binary_mdd methods                   *
// *                                                                *
// ******************************************************************

MEDDLY::generic_binary_mdd::generic_binary_mdd(const binary_opname* code,
  expert_forest* arg1, expert_forest* arg2, expert_forest* res)
  : binary_operation(code, 2, 1, arg1, arg2, res)
{
}

MEDDLY::generic_binary_mdd::~generic_binary_mdd()
{
}

bool MEDDLY::generic_binary_mdd::isStaleEntry(const node_handle* data)
{
  return arg1F->isStale(data[0]) ||
         arg2F->isStale(data[1]) ||
         resF->isStale(data[2]);
}

void MEDDLY::generic_binary_mdd::discardEntry(const node_handle* data)
{
  arg1F->uncacheNode(data[0]);
  arg2F->uncacheNode(data[1]);
  resF->uncacheNode(data[2]);
}

void
MEDDLY::generic_binary_mdd ::showEntry(output &strm, const node_handle *data) const
{
  strm << "[" << getName() << "(" << long(data[0]) << ", " << long(data[1]) 
       << "): " << long(data[2]) << "]";
}

void MEDDLY::generic_binary_mdd::computeDDEdge(const dd_edge &a, const dd_edge &b, 
  dd_edge &c)
{
  node_handle cnode = compute(a.getNode(), b.getNode());
  c.set(cnode);
#ifdef TRACE_ALL_OPS
  printf("completed %s(%d, %d) = %d\n", 
    getName(), a.getNode(), b.getNode(), cnode);
#endif
#ifdef DEVELOPMENT_CODE
  resF->validateIncounts(true);
#endif
}


MEDDLY::node_handle 
MEDDLY::generic_binary_mdd::compute(node_handle a, node_handle b)
{
  node_handle result = 0;
  if (checkTerminals(a, b, result))
    return result;

  compute_table::search_key* Key = findResult(a, b, result);
  if (0==Key) return result;

  // Get level information
  const int aLevel = arg1F->getNodeLevel(a);
  const int bLevel = arg2F->getNodeLevel(b);

  const int resultLevel = MAX(aLevel, bLevel);
  const int resultSize = resF->getLevelSize(resultLevel);

  unpacked_node* C = unpacked_node::newFull(resF, resultLevel, resultSize);

  // Initialize readers
  unpacked_node *A = (aLevel < resultLevel) 
    ? unpacked_node::newRedundant(arg1F, resultLevel, a, true)
    : unpacked_node::newFromNode(arg1F, a, true)
  ;

  unpacked_node *B = (bLevel < resultLevel)
    ? unpacked_node::newRedundant(arg2F, resultLevel, b, true)
    : unpacked_node::newFromNode(arg2F, b, true)
  ;

  // do computation
  for (int i=0; i<resultSize; i++) {
    C->d_ref(i) = compute(A->d(i), B->d(i));
  }

  // cleanup
  unpacked_node::recycle(B);
  unpacked_node::recycle(A);

  // reduce and save result
  result = resF->createReducedNode(-1, C);
  saveResult(Key, a, b, result);

#ifdef TRACE_ALL_OPS
  printf("computed %s(%d, %d) = %d\n", getName(), a, b, result);
#endif
  return result;
}


// ******************************************************************
// *                                                                *
// *                   generic_binary_mxd methods                   *
// *                                                                *
// ******************************************************************

MEDDLY::generic_binary_mxd::generic_binary_mxd(const binary_opname* code,
  expert_forest* arg1, expert_forest* arg2, expert_forest* res)
  : binary_operation(code, 2, 1, arg1, arg2, res)
{
  // data[0] : arg1
  // data[1] : arg2
  // data[2] : result
}

MEDDLY::generic_binary_mxd::~generic_binary_mxd()
{
}

bool MEDDLY::generic_binary_mxd::isStaleEntry(const node_handle* data)
{
  return arg1F->isStale(data[0]) ||
         arg2F->isStale(data[1]) ||
         resF->isStale(data[2]);
}

void MEDDLY::generic_binary_mxd::discardEntry(const node_handle* data)
{
  arg1F->uncacheNode(data[0]);
  arg2F->uncacheNode(data[1]);
  resF->uncacheNode(data[2]);
}

void
MEDDLY::generic_binary_mxd ::showEntry(output &strm, const node_handle *data) const
{
  strm << "[" << getName() << "(" << long(data[0]) << ", " << long(data[1]) 
       << "): " << long(data[2]) << "]";
}

void MEDDLY::generic_binary_mxd::computeDDEdge(const dd_edge &a, const dd_edge &b, 
  dd_edge &c)
{
  node_handle cnode = compute(a.getNode(), b.getNode());
  c.set(cnode);
#ifdef DEVELOPMENT_CODE
  resF->validateIncounts(true);
#endif
}

MEDDLY::node_handle 
MEDDLY::generic_binary_mxd::compute(node_handle a, node_handle b) 
{
  //  Compute for the unprimed levels.
  //
  node_handle result = 0;
  if (checkTerminals(a, b, result))
    return result;

  compute_table::search_key* Key = findResult(a, b, result);
  if (0==Key) return result;

  // Get level information
  const int aLevel = arg1F->getNodeLevel(a);
  const int bLevel = arg2F->getNodeLevel(b);
  int resultLevel = ABS(topLevel(aLevel, bLevel));

  int resultSize = resF->getLevelSize(resultLevel);

  unpacked_node* C = unpacked_node::newFull(resF, resultLevel, resultSize);

  // Initialize readers
  unpacked_node *A = (aLevel < resultLevel) 
    ? unpacked_node::newRedundant(arg1F, resultLevel, a, true)
    : unpacked_node::newFromNode(arg1F, a, true)
  ;

  unpacked_node *B = (bLevel < resultLevel)
    ? unpacked_node::newRedundant(arg2F, resultLevel, b, true)
    : unpacked_node::newFromNode(arg2F, b, true)
  ;

  // Do computation
  for (int j=0; j<resultSize; j++) {
    C->d_ref(j) = compute_r(j, resF->downLevel(resultLevel), A->d(j), B->d(j));
  }

  // cleanup
  unpacked_node::recycle(B);
  unpacked_node::recycle(A);

  // reduce and save result
  result = resF->createReducedNode(-1, C);
  saveResult(Key, a, b, result);

  /*
  printf("Saving %s un (%d, %d) = %d\n", getName(), a, b, result);
  printf("\tLevels %d and %d\n", aLevel, bLevel);
  */
#ifdef TRACE_ALL_OPS
  printf("computed %s(%d, %d) = %d\n", getName(), a, b, result);
#endif

  return result;
}

MEDDLY::node_handle 
MEDDLY::generic_binary_mxd::compute_r(int in, int k, node_handle a, node_handle b)
{
  //  Compute for the primed levels.
  //
  MEDDLY_DCASSERT(k<0);

  node_handle result;
  /*
  //
  // Note - we cache the primed levels, but only when "safe"
  //
  compute_table::search_key* Key = findResult(a, b, result);
  if (0==Key) {
    printf("Found %s pr (%d, %d) = %d\n", getName(), a, b, result);
    printf("\tat level %d\n", k);
    return result;
  }
  */

  // bool canSaveResult = true;

  // Get level information
  const int aLevel = arg1F->getNodeLevel(a);
  const int bLevel = arg2F->getNodeLevel(b);

  const int resultSize = resF->getLevelSize(k);

  unpacked_node* C = unpacked_node::newFull(resF, k, resultSize);

  // Initialize readers
  unpacked_node *A = unpacked_node::useUnpackedNode();
  unpacked_node *B = unpacked_node::useUnpackedNode();

  if (aLevel == k) {
    A->initFromNode(arg1F, a, true);
  } else if (arg1F->isFullyReduced()) {
    A->initRedundant(arg1F, k, a, true);
  } else {
    A->initIdentity(arg1F, k, in, a, true);
  }

  if (bLevel == k) {
    B->initFromNode(arg2F, b, true);
  } else if (arg2F->isFullyReduced()) {
    B->initRedundant(arg2F, k, b, true);
  } else {
    B->initIdentity(arg2F, k, in, b, true);
  }

  // Do computation
  for (int j=0; j<resultSize; j++) {
    C->d_ref(j) = compute(A->d(j), B->d(j));
  }

  // cleanup
  unpacked_node::recycle(B);
  unpacked_node::recycle(A);

  // reduce 
  result = resF->createReducedNode(in, C);

#ifdef TRACE_ALL_OPS
  printf("computed %s(in %d, %d, %d) = %d\n", getName(), in, a, b, result);
#endif

  return result;
}

// ******************************************************************
// *                                                                *
// *                 generic_binbylevel_mxd methods                 *
// *                                                                *
// ******************************************************************

MEDDLY::generic_binbylevel_mxd
::generic_binbylevel_mxd(const binary_opname* code, expert_forest* arg1, 
  expert_forest* arg2, expert_forest* res)
 : binary_operation(code, 3, 1, arg1, arg2, res)
{
  can_commute = false;
}

MEDDLY::generic_binbylevel_mxd::~generic_binbylevel_mxd()
{
}

bool MEDDLY::generic_binbylevel_mxd::isStaleEntry(const node_handle* data)
{
  return arg1F->isStale(data[1]) ||
         arg2F->isStale(data[2]) ||
         resF->isStale(data[3]);
}

void MEDDLY::generic_binbylevel_mxd::discardEntry(const node_handle* data)
{
  arg1F->uncacheNode(data[1]);
  arg2F->uncacheNode(data[2]);
  resF->uncacheNode(data[3]);
}

void
MEDDLY::generic_binbylevel_mxd
::showEntry(output &strm, const node_handle *data) const
{
  strm << "[" << getName() << "(" << long(data[0]) << ", " << long(data[1]) 
       << ", " << long(data[2]) << "): " << long(data[3]) << "]";
}

void MEDDLY::generic_binbylevel_mxd
::computeDDEdge(const dd_edge& a, const dd_edge& b, dd_edge& c)
{
  node_handle result = compute(
    resF->getDomain()->getNumVariables(), a.getNode(), b.getNode()
  );
  c.set(result);
#ifdef DEVELOPMENT_CODE
  resF->validateIncounts(true);
#endif
}

MEDDLY::node_handle 
MEDDLY::generic_binbylevel_mxd::compute(int level, node_handle a, node_handle b) 
{
  return compute_r(-1, level, a, b);
}

MEDDLY::node_handle 
MEDDLY::generic_binbylevel_mxd
::compute_r(int in, int resultLevel, node_handle a, node_handle b)
{
  node_handle result = 0;
  if (0==resultLevel) {
    if (checkTerminals(a, b, result))
      return result;
  }

  //
  // Note - we cache the primed levels, but only when "safe"
  //
  compute_table::search_key* Key = findResult(resultLevel, a, b, result);
  if (0==Key) return result;

  // Get level information
  const int aLevel = arg1F->getNodeLevel(a);
  const int bLevel = arg2F->getNodeLevel(b);

  int resultSize = resF->getLevelSize(resultLevel);

  unpacked_node* C = unpacked_node::newFull(resF, resultLevel, resultSize);

  bool canSaveResult = true;

  // Initialize readers
  unpacked_node* A = unpacked_node::useUnpackedNode();
  unpacked_node* B = unpacked_node::useUnpackedNode();

  if (aLevel == resultLevel) {
    A->initFromNode(arg1F, a, true);
  } else if (resultLevel>0 || arg1F->isFullyReduced()) {
    A->initRedundant(arg1F, resultLevel, a, true);
  } else {
    A->initIdentity(arg1F, resultLevel, in, a, true);
    canSaveResult = false;
  }

  if (bLevel == resultLevel) {
    B->initFromNode(arg2F, b, true);
  } else if (resultLevel>0 || arg2F->isFullyReduced()) {
    B->initRedundant(arg2F, resultLevel, b, true);
  } else {
    B->initIdentity(arg2F, resultLevel, in, b, true);
    canSaveResult = false;
  }

  // Do computation
  int nextLevel = resF->downLevel(resultLevel);
  int nnz = 0;
  for (int j=0; j<resultSize; j++) {
    node_handle d = compute_r(j, nextLevel, A->d(j), B->d(j));
    C->d_ref(j) = d;
    if (d) nnz++;
  }

  // cleanup
  unpacked_node::recycle(B);
  unpacked_node::recycle(A);

  // reduce
  result = resF->createReducedNode(in, C);

  // save result in compute table, when we can
  if (resultLevel<0 && 1==nnz) canSaveResult = false;
  if (canSaveResult)  saveResult(Key, resultLevel, a, b, result);
  else                doneCTkey(Key);

#ifdef TRACE_ALL_OPS
  printf("computed %s(in %d, %d, %d) = %d\n", getName(), in, a, b, result);
#endif

  return result;
}


// ******************************************************************
// *                                                                *
// *                   generic_binary_ev  methods                   *
// *                                                                *
// ******************************************************************

MEDDLY::generic_binary_ev::generic_binary_ev(const binary_opname* code,
  expert_forest* arg1, expert_forest* arg2, expert_forest* res)
  : binary_operation(code, 4, 2, arg1, arg2, res)
{
  can_commute = false;
}

MEDDLY::generic_binary_ev::~generic_binary_ev()
{
}

bool MEDDLY::generic_binary_ev::isStaleEntry(const node_handle* data)
{
  return arg1F->isStale(data[1]) ||
         arg2F->isStale(data[3]) ||
         resF->isStale(data[5]);
}

void MEDDLY::generic_binary_ev::discardEntry(const node_handle* data)
{
  arg1F->uncacheNode(data[1]);
  arg2F->uncacheNode(data[3]);
  resF->uncacheNode(data[5]);
}

// ******************************************************************
// *                                                                *
// *                 generic_binary_evplus  methods                 *
// *                                                                *
// ******************************************************************

MEDDLY::generic_binary_evplus::generic_binary_evplus(const binary_opname* code,
  expert_forest* arg1, expert_forest* arg2, expert_forest* res)
  : generic_binary_ev(code, arg1, arg2, res)
{
}

MEDDLY::generic_binary_evplus::~generic_binary_evplus()
{
}

void MEDDLY::generic_binary_evplus
::showEntry(output &strm, const node_handle *data) const
{
  strm << "[" << getName() << "(<" << long(data[0]) << ":" << long(data[1]) 
       << ">, <" << long(data[2]) << ":" << long(data[3]) << ">): <"
       << long(data[4]) << ":" << long(data[5]) << ">]";
}

void MEDDLY::generic_binary_evplus
::computeDDEdge(const dd_edge& a, const dd_edge& b, dd_edge& c)
{
  node_handle result;
  int ev, aev, bev;
  a.getEdgeValue(aev);
  b.getEdgeValue(bev);
  compute(aev, a.getNode(), bev, b.getNode(), ev, result);
  c.set(result, ev);
#ifdef DEVELOPMENT_CODE
  resF->validateIncounts(true);
#endif
}

void MEDDLY::generic_binary_evplus
::compute(int aev, node_handle a, int bev, node_handle b, 
  int& cev, node_handle& c)
{
  if (checkTerminals(aev, a, bev, b, cev, c))
    return;

  compute_table::search_key* Key = findResult(aev, a, bev, b, cev, c);
  if (0==Key) return;

  // Get level information
  const int aLevel = arg1F->getNodeLevel(a);
  const int bLevel = arg2F->getNodeLevel(b);

  const int resultLevel = aLevel > bLevel? aLevel: bLevel;
  const int resultSize = resF->getLevelSize(resultLevel);

  // Initialize result
  unpacked_node* nb = unpacked_node::newFull(resF, resultLevel, resultSize);

  // Initialize readers
  unpacked_node *A = (aLevel < resultLevel) 
    ? unpacked_node::newRedundant(arg1F, resultLevel, 0, a, true)
    : unpacked_node::newFromNode(arg1F, a, true)
  ;

  unpacked_node *B = (bLevel < resultLevel)
    ? unpacked_node::newRedundant(arg2F, resultLevel, 0, b, true)
    : unpacked_node::newFromNode(arg2F, b, true)
  ;


  // do computation
  for (int i=0; i<resultSize; i++) {
    int ev;
    node_handle ed;
    std::cout << "generic apply! Danger!" << std::endl;
    compute(aev + A->ei(i), A->d(i), 
            bev + B->ei(i), B->d(i), 
            ev, ed);
    nb->d_ref(i) = ed;
    nb->setEdge(i, ev);
  }

  // cleanup
  unpacked_node::recycle(B);
  unpacked_node::recycle(A);

  // Reduce
  node_handle cl;
  resF->createReducedNode(-1, nb, cev, cl);
  c = cl;

  // Add to CT
  saveResult(Key, aev, a, bev, b, cev, c);
}


// ******************************************************************
// *                                                                *
// *                 generic_binary_evtimes methods                 *
// *                                                                *
// ******************************************************************

MEDDLY::generic_binary_evtimes
::generic_binary_evtimes(const binary_opname* code, expert_forest* arg1, 
  expert_forest* arg2, expert_forest* res)
: generic_binary_ev(code, arg1, arg2, res)
{
}

MEDDLY::generic_binary_evtimes::~generic_binary_evtimes()
{
}

void MEDDLY::generic_binary_evtimes
::showEntry(output &strm, const node_handle *data) const
{
  float ev0;
  float ev2;
  float ev4;
  compute_table::readEV(data+0, ev0);
  compute_table::readEV(data+2, ev2);
  compute_table::readEV(data+4, ev4);
  strm << "[" << getName() << "(<" << ev0 << ":" << long(data[1]) 
       << ">, <" << ev2 << ":" << long(data[3]) << ">): <"
       << ev4 << ":" << long(data[5]) << ">]";
}

void MEDDLY::generic_binary_evtimes
::computeDDEdge(const dd_edge& a, const dd_edge& b, dd_edge& c)
{
  node_handle result; 
  float ev, aev, bev;
  a.getEdgeValue(aev);
  b.getEdgeValue(bev);
  compute(aev, a.getNode(), bev, b.getNode(), ev, result);
  c.set(result, ev);
#ifdef DEVELOPMENT_CODE
  resF->validateIncounts(true);
#endif
}

void MEDDLY::generic_binary_evtimes
::compute(float aev, node_handle a, float bev, node_handle b, 
  float& cev, node_handle& c)
{
  // Compute for the unprimed levels.
  //

  if (checkTerminals(aev, a, bev, b, cev, c))
    return;

#ifndef DISABLE_CACHE
  compute_table::search_key* Key = findResult(aev, a, bev, b, cev, c);
  if (0==Key) return;
#endif

  // Get level information
  const int aLevel = arg1F->getNodeLevel(a);
  const int bLevel = arg2F->getNodeLevel(b);

  // Initialize result
  const int resultLevel = ABS(topLevel(aLevel, bLevel));
  const int resultSize = resF->getLevelSize(resultLevel);
  unpacked_node* nb = unpacked_node::newFull(resF, resultLevel, resultSize);

  // Initialize readers
  unpacked_node *A = (aLevel < resultLevel) 
    ? unpacked_node::newRedundant(arg1F, resultLevel, 1.0f, a, true)
    : unpacked_node::newFromNode(arg1F, a, true)
  ;

  unpacked_node *B = (bLevel < resultLevel)
    ? unpacked_node::newRedundant(arg2F, resultLevel, 1.0f, b, true)
    : unpacked_node::newFromNode(arg2F, b, true)
  ;

  // do computation
  for (int i=0; i<resultSize; i++) {
    float ev;
    node_handle ed;
    compute_k(
        i, -resultLevel,
        aev * A->ef(i), A->d(i), 
        bev * B->ef(i), B->d(i), 
        ev, ed);
    nb->d_ref(i) = ed;
    nb->setEdge(i, ev);
  }

  // cleanup
  unpacked_node::recycle(B);
  unpacked_node::recycle(A);

  // Reduce
  node_handle cl;
  resF->createReducedNode(-1, nb, cev, cl);
  c = cl;

#ifndef DISABLE_CACHE
  // Add to CT
  saveResult(Key, aev, a, bev, b, cev, c);
#endif
}

void MEDDLY::generic_binary_evtimes
::compute_k(
  int in, int resultLevel,
  float aev, node_handle a,
  float bev, node_handle b,
  float& cev, node_handle& c)
{
  // Compute for the primed levels.
  //
  MEDDLY_DCASSERT(resultLevel < 0);

  if (checkTerminals(aev, a, bev, b, cev, c))
    return;

  // Get level information
  const int aLevel = arg1F->getNodeLevel(a);
  const int bLevel = arg2F->getNodeLevel(b);

  // Initialize result
  const int resultSize = resF->getLevelSize(resultLevel);
  unpacked_node* nb = unpacked_node::newFull(resF, resultLevel, resultSize);

  // Initialize readers
  unpacked_node *A = unpacked_node::useUnpackedNode();
  unpacked_node *B = unpacked_node::useUnpackedNode();

  if (aLevel == resultLevel) {
    A->initFromNode(arg1F, a, true);
  } else if (arg1F->isFullyReduced()) {
    A->initRedundant(arg1F, resultLevel, 1.0f, a, true);
  } else {
    A->initIdentity(arg1F, resultLevel, in, 1.0f, a, true);
  }

  if (bLevel == resultLevel) {
    B->initFromNode(arg2F, b, true);
  } else if (arg2F->isFullyReduced()) {
    B->initRedundant(arg2F, resultLevel, 1.0f, b, true);
  } else {
    B->initIdentity(arg2F, resultLevel, in, 1.0f, b, true);
  }

  // do computation
  for (int i=0; i<resultSize; i++) {
    float ev;
    node_handle ed;
    compute(
        aev * A->ef(i), A->d(i),
        bev * B->ef(i), B->d(i), 
        ev, ed);
    nb->d_ref(i) = ed;
    nb->setEdge(i, ev);
  }

  // cleanup
  unpacked_node::recycle(B);
  unpacked_node::recycle(A);

  // Reduce
  node_handle cl;
  resF->createReducedNode(in, nb, cev, cl);
  c = cl;
}
