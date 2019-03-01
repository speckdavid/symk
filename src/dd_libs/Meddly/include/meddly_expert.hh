// $Id$

/*

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

/*! \file meddly_expert.hh

 Implementation details for interface in meddly_expert.h.
 */

#ifndef MEDDLY_EXPERT_HH
#define MEDDLY_EXPERT_HH

// ******************************************************************
// *                                                                *
// *                 inlined  expert_domain methods                 *
// *                                                                *
// ******************************************************************

#include <iostream>

inline MEDDLY::expert_variable*
MEDDLY::expert_domain::getExpertVar(int lev) const
{
  return (expert_variable*) vars[lev];
}

inline const MEDDLY::expert_variable*
MEDDLY::expert_domain::readExpertVar(int lev) const
{
  return (expert_variable*) vars[lev];
}

inline void
MEDDLY::expert_domain::enlargeVariableBound(int vh, bool prime, int b)
{
  getExpertVar(vh)->enlargeBound(prime, b);
}

inline void
MEDDLY::expert_domain::shrinkVariableBound(int vh, int b, bool force)
{
  getExpertVar(vh)->shrinkBound(b, force);
}


// ******************************************************************
// *                                                                *
// *                 inlined  unpacked_node methods                 *
// *                                                                *
// ******************************************************************


inline void 
MEDDLY::unpacked_node::initFromNode(const expert_forest *f, 
  node_handle node, bool full)
{
  MEDDLY_DCASSERT(f);
  f->fillUnpacked(*this, node, full ? FULL_NODE : SPARSE_NODE);
}

inline void 
MEDDLY::unpacked_node::initFromNode(const expert_forest *f, 
  node_handle node, storage_style st2)
{
  MEDDLY_DCASSERT(f);
  f->fillUnpacked(*this, node, st2);
}

inline void MEDDLY::unpacked_node::initFull(const expert_forest *f, int level, int tsz)
{
  MEDDLY_DCASSERT(f);
  bind_to_forest(f, level, tsz, true);
}

inline void MEDDLY::unpacked_node::initSparse(const expert_forest *f, int level, int nnz)
{
  MEDDLY_DCASSERT(f);
  bind_to_forest(f, level, nnz, false);
}

// ****************************************************************************

inline MEDDLY::unpacked_node* 
MEDDLY::unpacked_node::newFromNode(const expert_forest *f, node_handle node, bool full)
{
  unpacked_node* U = useUnpackedNode();
  MEDDLY_DCASSERT(U);
  U->initFromNode(f, node, full);
  return U;
}

inline MEDDLY::unpacked_node* 
MEDDLY::unpacked_node::newFromNode(const expert_forest *f, node_handle node, storage_style st2)
{
  unpacked_node* U = useUnpackedNode();
  MEDDLY_DCASSERT(U);
  U->initFromNode(f, node, st2);
  return U;
}

inline MEDDLY::unpacked_node* 
MEDDLY::unpacked_node::newRedundant(const expert_forest *f, int k, node_handle node, bool full)
{
  unpacked_node* U = useUnpackedNode();
  MEDDLY_DCASSERT(U);
  U->initRedundant(f, k, node, full);
  return U;
}

inline MEDDLY::unpacked_node* 
MEDDLY::unpacked_node::newRedundant(const expert_forest *f, int k, int ev, node_handle node, bool full)
{
  unpacked_node* U = useUnpackedNode();
  MEDDLY_DCASSERT(U);
  U->initRedundant(f, k, ev, node, full);
  return U;
}

inline MEDDLY::unpacked_node* 
MEDDLY::unpacked_node::newRedundant(const expert_forest *f, int k, float ev, node_handle node, bool full)
{
  unpacked_node* U = useUnpackedNode();
  MEDDLY_DCASSERT(U);
  U->initRedundant(f, k, ev, node, full);
  return U;
}

inline MEDDLY::unpacked_node* 
MEDDLY::unpacked_node::newIdentity(const expert_forest *f, int k, int i, node_handle node, bool full)
{
  unpacked_node* U = useUnpackedNode();
  MEDDLY_DCASSERT(U);
  U->initIdentity(f, k, i, node, full);
  return U;
}

inline MEDDLY::unpacked_node* 
MEDDLY::unpacked_node::newIdentity(const expert_forest *f, int k, int i, int ev, node_handle node, bool full)
{
  unpacked_node* U = useUnpackedNode();
  MEDDLY_DCASSERT(U);
  U->initIdentity(f, k, i, ev, node, full);
  return U;
}

inline MEDDLY::unpacked_node* 
MEDDLY::unpacked_node::newIdentity(const expert_forest *f, int k, int i, float ev, node_handle node, bool full)
{
  unpacked_node* U = useUnpackedNode();
  MEDDLY_DCASSERT(U);
  U->initIdentity(f, k, i, ev, node, full);
  return U;
}

inline MEDDLY::unpacked_node* 
MEDDLY::unpacked_node::newFull(const expert_forest *f, int level, int tsz)
{
  unpacked_node* U = useUnpackedNode();
  MEDDLY_DCASSERT(U);
  U->initFull(f, level, tsz);
  return U;
}

inline MEDDLY::unpacked_node* 
MEDDLY::unpacked_node::newSparse(const expert_forest *f, int level, int nnzs)
{
  unpacked_node* U = useUnpackedNode();
  MEDDLY_DCASSERT(U);
  U->initSparse(f, level, nnzs);
  return U;
}

// ****************************************************************************

inline const void*
MEDDLY::unpacked_node::UHptr() const
{
  MEDDLY_DCASSERT(extra_unhashed);
  return extra_unhashed;
}

inline void*
MEDDLY::unpacked_node::UHdata()
{
  MEDDLY_DCASSERT(extra_unhashed);
  return extra_unhashed;
}

inline int
MEDDLY::unpacked_node::UHbytes() const
{
  return ext_uh_size;
}

inline const void*
MEDDLY::unpacked_node::HHptr() const
{
  MEDDLY_DCASSERT(extra_hashed);
  return extra_hashed;
}

inline void*
MEDDLY::unpacked_node::HHdata()
{
  MEDDLY_DCASSERT(extra_hashed);
  return extra_hashed;
}

inline int
MEDDLY::unpacked_node::HHbytes() const
{
  return ext_h_size;
}

inline MEDDLY::node_handle
MEDDLY::unpacked_node::d(int n) const
{
  MEDDLY_DCASSERT(down);
  MEDDLY_CHECK_RANGE(0, n, (is_full ? size : nnzs));
  return down[n];
}

inline MEDDLY::node_handle&
MEDDLY::unpacked_node::d_ref(int n) 
{
  MEDDLY_DCASSERT(down);
  MEDDLY_CHECK_RANGE(0, n, (is_full ? size : nnzs));
  return down[n];
}


inline int
MEDDLY::unpacked_node::i(int n) const
{
  MEDDLY_DCASSERT(index);
  MEDDLY_DCASSERT(!is_full);
  MEDDLY_CHECK_RANGE(0, n, nnzs);
  return index[n];
}

inline int&
MEDDLY::unpacked_node::i_ref(int n)
{
  MEDDLY_DCASSERT(index);
  MEDDLY_DCASSERT(!is_full);
  MEDDLY_CHECK_RANGE(0, n, nnzs);
  return index[n];
}

inline const void*
MEDDLY::unpacked_node::eptr(int i) const
{
  MEDDLY_DCASSERT(edge);
  MEDDLY_CHECK_RANGE(0, i, (is_full ? size : nnzs));
  return ((char*) edge) + i * edge_bytes;
}

inline void*
MEDDLY::unpacked_node::eptr_write(int i)
{
  MEDDLY_DCASSERT(edge);
  MEDDLY_CHECK_RANGE(0, i, (is_full ? size : nnzs));
  return ((char*) edge) + i * edge_bytes;
}


inline void
MEDDLY::unpacked_node::getEdge(int n, int &val) const
{
  MEDDLY_DCASSERT(sizeof(int) == edge_bytes);
  MEDDLY::expert_forest::int_EVencoder::readValue(eptr(n), val);
}

inline void
MEDDLY::unpacked_node::getEdge(int n, float &val) const
{
  MEDDLY_DCASSERT(sizeof(float) == edge_bytes);
  MEDDLY::expert_forest::float_EVencoder::readValue(eptr(n), val);
}

inline void
MEDDLY::unpacked_node::setEdge(int n, int ev)
{
  MEDDLY_DCASSERT(sizeof(int) == edge_bytes);
  MEDDLY::expert_forest::int_EVencoder::writeValue(eptr_write(n), ev);
}

inline void
MEDDLY::unpacked_node::setEdge(int n, float ev)
{
  MEDDLY_DCASSERT(sizeof(float) == edge_bytes);
  MEDDLY::expert_forest::float_EVencoder::writeValue(eptr_write(n), ev);
}


inline int
MEDDLY::unpacked_node::ei(int i) const
{
  int ev;
  getEdge(i, ev);
  return ev;
}

inline float
MEDDLY::unpacked_node::ef(int i) const
{
  float ev;
  getEdge(i, ev);
  return ev;
}

inline int
MEDDLY::unpacked_node::getLevel() const
{
  return level;
}

inline void
MEDDLY::unpacked_node::setLevel(int k)
{
  level = k;
}

inline int
MEDDLY::unpacked_node::getSize() const
{
  MEDDLY_DCASSERT(is_full);
  return size;
}

inline int
MEDDLY::unpacked_node::getNNZs() const
{
  MEDDLY_DCASSERT(!is_full);
  return nnzs;
}

inline bool
MEDDLY::unpacked_node::isSparse() const
{
  return !is_full;
}

inline bool
MEDDLY::unpacked_node::isFull() const
{
  return is_full;
}

inline bool
MEDDLY::unpacked_node::hasEdges() const
{
  return edge_bytes;
}
inline int
MEDDLY::unpacked_node::edgeBytes() const
{
  return edge_bytes;
}

inline unsigned
MEDDLY::unpacked_node::hash() const
{
#ifdef DEVELOPMENT_CODE
  MEDDLY_DCASSERT(has_hash);
#endif
  return h;
}
inline void
MEDDLY::unpacked_node::setHash(unsigned H)
{
#ifdef DEVELOPMENT_CODE
  MEDDLY_DCASSERT(!has_hash);
  has_hash = true;
#endif
  h = H;
}

inline void
MEDDLY::unpacked_node::shrinkFull(int ns)
{
  MEDDLY_DCASSERT(isFull());
  MEDDLY_DCASSERT(ns >= 0);
  MEDDLY_DCASSERT(ns <= size);
  size = ns;
}

inline void
MEDDLY::unpacked_node::shrinkSparse(int ns)
{
  MEDDLY_DCASSERT(isSparse());
  MEDDLY_DCASSERT(ns >= 0);
  MEDDLY_DCASSERT(ns <= nnzs);
  nnzs = ns;
}

inline void
MEDDLY::unpacked_node::bind_as_full(bool full)
{
  is_full = full;
}

inline MEDDLY::unpacked_node*
MEDDLY::unpacked_node::useUnpackedNode()
{
  unpacked_node* nr;
  if (freeList) {
    nr = freeList;
    freeList = nr->next;
  }
  else {
    nr = new unpacked_node;
  }
#ifdef DEVELOPMENT_CODE
  nr->has_hash = false;
#endif
  return nr;
}

inline void
MEDDLY::unpacked_node::recycle(MEDDLY::unpacked_node* r)
{
  if (r) {
    r->next = freeList;
    freeList = r;
  }
}

inline void
MEDDLY::unpacked_node::freeRecycled()
{
  while (freeList) {
    MEDDLY::unpacked_node* n = freeList->next;
    delete freeList;
    freeList = n;
  }
}

// ******************************************************************
// *                                                                *
// *           inlined expert_forest::node_header methods           *
// *                                                                *
// ******************************************************************

inline bool
MEDDLY::expert_forest::node_header::isActive() const
{
  return offset > 0;
}
inline bool
MEDDLY::expert_forest::node_header::isZombie() const
{
  return cache_count < 0;
}
inline bool
MEDDLY::expert_forest::node_header::isDeleted() const
{
  return 0 == level;
}
inline void
MEDDLY::expert_forest::node_header::setDeleted()
{
  level = 0;
}
inline void
MEDDLY::expert_forest::node_header::setNotDeleted()
{
  level = 1;
}
inline int
MEDDLY::expert_forest::node_header::getNextDeleted() const
{
  return -offset;
}
inline void
MEDDLY::expert_forest::node_header::setNextDeleted(int n)
{
  offset = -n;
}
inline void
MEDDLY::expert_forest::node_header::makeZombie()
{
  MEDDLY_DCASSERT(cache_count > 0);
  cache_count *= -1;
  offset = 0;
}

// ******************************************************************
// *                                                                *
// *                  inlined node_storage methods                  *
// *                                                                *
// ******************************************************************


#ifdef INLINED_COUNT
inline MEDDLY::node_handle
MEDDLY::node_storage::getCountOf(node_address addr) const
{
  MEDDLY_DCASSERT(counts);
  MEDDLY_DCASSERT(addr > 0);
  return counts[addr];
}

inline void
MEDDLY::node_storage::setCountOf(node_address addr, int c)
{
  MEDDLY_DCASSERT(counts);
  MEDDLY_DCASSERT(addr > 0);
  counts[addr] = c;
}

inline MEDDLY::node_handle
MEDDLY::node_storage::incCountOf(node_address addr)
{
  MEDDLY_DCASSERT(counts);
  MEDDLY_DCASSERT(addr > 0);
  return ++counts[addr];
}

inline MEDDLY::node_handle
MEDDLY::node_storage::decCountOf(node_address addr)
{
  MEDDLY_DCASSERT(counts);
  MEDDLY_DCASSERT(addr > 0);
  return --counts[addr];
}
#endif

#ifdef INLINED_NEXT
inline MEDDLY::node_handle
MEDDLY::node_storage::getNextOf(node_address addr) const
{
  MEDDLY_DCASSERT(nexts);
  MEDDLY_DCASSERT(addr > 0);
  return nexts[addr];
}

inline void
MEDDLY::node_storage::setNextOf(node_address addr, MEDDLY::node_handle n)
{
  MEDDLY_DCASSERT(nexts);
  MEDDLY_DCASSERT(addr > 0);
  nexts[addr] = n;
}
#endif

inline const MEDDLY::expert_forest*
MEDDLY::node_storage::getParent() const
{
  MEDDLY_DCASSERT(parent);
  return parent;
}

inline MEDDLY::expert_forest*
MEDDLY::node_storage::getParent()
{
  MEDDLY_DCASSERT(parent);
  return parent;
}

inline void
MEDDLY::node_storage::incMemUsed(long delta)
{
  if (stats)
    stats->incMemUsed(delta);
}

inline void
MEDDLY::node_storage::decMemUsed(long delta)
{
  if (stats)
    stats->decMemUsed(delta);
}

inline void
MEDDLY::node_storage::incMemAlloc(long delta)
{
  if (stats)
    stats->incMemAlloc(delta);
}

inline void
MEDDLY::node_storage::decMemAlloc(long delta)
{
  if (stats)
    stats->decMemAlloc(delta);
}

inline void
MEDDLY::node_storage::incCompactions()
{
  if (stats)
    stats->num_compactions++;
}

inline void
MEDDLY::node_storage::updateCountArray(MEDDLY::node_handle* cptr)
{
  counts = cptr;
}

inline void
MEDDLY::node_storage::updateNextArray(MEDDLY::node_handle* nptr)
{
  nexts = nptr;
}

inline void
MEDDLY::node_storage::moveNodeOffset(MEDDLY::node_handle node, node_address old_addr,
    node_address new_addr)
{
  MEDDLY_DCASSERT(parent);
  parent->moveNodeOffset(node, old_addr, new_addr);
}

// ******************************************************************
// *                                                                *
// *                 inlined  expert_forest methods                 *
// *                                                                *
// ******************************************************************

inline MEDDLY::node_handle
MEDDLY::expert_forest::bool_Tencoder::value2handle(bool v)
{
  return v ? -1 : 0;
}

inline bool
MEDDLY::expert_forest::bool_Tencoder::handle2value(MEDDLY::node_handle h)
{
  if (-1 == h)
    return true;
  if (0 == h)
    return false;
  throw error(error::MISCELLANEOUS);
}

inline MEDDLY::node_handle
MEDDLY::expert_forest::int_Tencoder::value2handle(int v)
{
  MEDDLY_DCASSERT(4 == sizeof(MEDDLY::node_handle));
  if (v < -1073741824 || v > 1073741823) {
    // Can't fit in 31 bits (signed)
    throw error(error::VALUE_OVERFLOW);
  }
  if (v)
    v |= 0x80000000; // sets the sign bit
  return v;
}

inline int
MEDDLY::expert_forest::int_Tencoder::handle2value(MEDDLY::node_handle h)
{
  // << 1 kills the sign bit
  // >> 1 puts us back, and extends the (new) sign bit
  return (h << 1) >> 1;
}

inline MEDDLY::node_handle
MEDDLY::expert_forest::float_Tencoder::value2handle(float v)
{
  MEDDLY_DCASSERT(4 == sizeof(MEDDLY::node_handle));
  MEDDLY_DCASSERT(sizeof(float) <= sizeof(MEDDLY::node_handle));
  if (0.0 == v)
    return 0;
  intfloat x;
  x.real = v;
  // strip lsb in fraction, and add sign bit
  return (x.integer >> 1) | 0x80000000;
}

inline float
MEDDLY::expert_forest::float_Tencoder::handle2value(MEDDLY::node_handle h)
{
  MEDDLY_DCASSERT(4 == sizeof(MEDDLY::node_handle));
  MEDDLY_DCASSERT(sizeof(float) <= sizeof(MEDDLY::node_handle));
  if (0 == h)
    return 0.0;
  intfloat x;
  x.integer = (h << 1); // remove sign bit
  return x.real;
}

inline size_t
MEDDLY::expert_forest::int_EVencoder::edgeBytes()
{
  return sizeof(int);
}
inline void
MEDDLY::expert_forest::int_EVencoder::writeValue(void* ptr, int val)
{
  memcpy(ptr, &val, sizeof(int));
}
inline void
MEDDLY::expert_forest::int_EVencoder::readValue(const void* ptr, int &val)
{
  memcpy(&val, ptr, sizeof(int));
}

inline size_t
MEDDLY::expert_forest::float_EVencoder::edgeBytes()
{
  return sizeof(float);
}
inline void
MEDDLY::expert_forest::float_EVencoder::writeValue(void* ptr, float val)
{
  memcpy(ptr, &val, sizeof(float));
}
inline void
MEDDLY::expert_forest::float_EVencoder::readValue(const void* ptr, float &val)
{
  memcpy(&val, ptr, sizeof(float));
}

template<typename T>
  inline MEDDLY::node_handle
  MEDDLY::expert_forest::handleForValue(T v) const
  {
    switch (getRangeType()) {
      case BOOLEAN:
        return bool_Tencoder::value2handle(v);
      case INTEGER:
        return int_Tencoder::value2handle(v);
      case REAL:
        return float_Tencoder::value2handle(v);
      default:
        throw error(error::MISCELLANEOUS);
    }
  }

template<typename T>
  inline void
  MEDDLY::expert_forest::getValueFromHandle(MEDDLY::node_handle n, T& v) const
  {
    MEDDLY_DCASSERT(isTerminalNode(n));
    switch (getRangeType()) {
      case BOOLEAN:
        v = bool_Tencoder::handle2value(n);
        return;
      case INTEGER:
        v = int_Tencoder::handle2value(n);
        return;
      case REAL:
        v = float_Tencoder::handle2value(n);
        return;
      default:
        throw error(error::MISCELLANEOUS);
    }
  }

inline bool
MEDDLY::expert_forest::getBooleanFromHandle(MEDDLY::node_handle n) const
{
  MEDDLY_DCASSERT(isTerminalNode(n));
  switch (getRangeType()) {
    case BOOLEAN:
      return bool_Tencoder::handle2value(n);
    case INTEGER:
      return int_Tencoder::handle2value(n);
    case REAL:
      return float_Tencoder::handle2value(n);
    default:
      throw error(error::MISCELLANEOUS);
  }
}

inline int
MEDDLY::expert_forest::getIntegerFromHandle(MEDDLY::node_handle n) const
{
  MEDDLY_DCASSERT(isTerminalNode(n));
  switch (getRangeType()) {
    case BOOLEAN:
      return bool_Tencoder::handle2value(n);
    case INTEGER:
      return int_Tencoder::handle2value(n);
    case REAL:
      return float_Tencoder::handle2value(n);
    default:
      throw error(error::MISCELLANEOUS);
  }
}

inline float
MEDDLY::expert_forest::getRealFromHandle(MEDDLY::node_handle n) const
{
  MEDDLY_DCASSERT(isTerminalNode(n));
  switch (getRangeType()) {
    case BOOLEAN:
      return bool_Tencoder::handle2value(n);
    case INTEGER:
      return int_Tencoder::handle2value(n);
    case REAL:
      return float_Tencoder::handle2value(n);
    default:
      throw error(error::MISCELLANEOUS);
  }
}

inline MEDDLY::expert_forest::statset&
MEDDLY::expert_forest::changeStats()
{
  return stats;
}

inline char
MEDDLY::expert_forest::edgeBytes() const
{
  return edge_bytes;
}

inline bool
MEDDLY::expert_forest::areEdgeValuesHashed() const
{
  return hash_edge_values;
}

inline char
MEDDLY::expert_forest::unhashedHeaderBytes() const
{
  return unhashed_bytes;
}

inline char
MEDDLY::expert_forest::hashedHeaderBytes() const
{
  return hashed_bytes;
}

inline const MEDDLY::expert_domain*
MEDDLY::expert_forest::getExpertDomain() const
{
  return (MEDDLY::expert_domain*) getDomain();
}

inline MEDDLY::expert_domain*
MEDDLY::expert_forest::useExpertDomain()
{
  return (MEDDLY::expert_domain*) useDomain();
}

// --------------------------------------------------
// Node address information
// --------------------------------------------------

inline MEDDLY::node_address
MEDDLY::expert_forest::getNodeAddress(node_handle p) const
{
  MEDDLY_DCASSERT(address);
  MEDDLY_DCASSERT(isValidNonterminalIndex(p));
  return address[p].offset;
}

inline void
MEDDLY::expert_forest::setNodeAddress(node_handle p, node_address a)
{
  MEDDLY_DCASSERT(address);
  MEDDLY_DCASSERT(isValidNonterminalIndex(p));
  address[p].offset = a;
}

// --------------------------------------------------
// Node level information
// --------------------------------------------------

inline int
MEDDLY::expert_forest::getNodeLevel(node_handle p) const
{
  if (isTerminalNode(p)) return 0;
  MEDDLY_DCASSERT(address);
  MEDDLY_DCASSERT(isValidNonterminalIndex(p));
  return address[p].level;
}

inline bool
MEDDLY::expert_forest::isPrimedNode(node_handle p) const
{
  return getNodeLevel(p) < 0;
}

inline bool
MEDDLY::expert_forest::isUnprimedNode(node_handle p) const
{
  return getNodeLevel(p) > 0;
}

inline int
MEDDLY::expert_forest::getNumVariables() const
{
  return getDomain()->getNumVariables();
}

inline int
MEDDLY::expert_forest::getMinLevelIndex() const
{
  return isForRelations() ? -getNumVariables() : 0;
}

inline bool
MEDDLY::expert_forest::isValidLevel(int k) const
{
  return (k >= getMinLevelIndex()) && (k <= getNumVariables());
}

inline int
MEDDLY::expert_forest::getLevelSize(int lh) const
{
  MEDDLY_DCASSERT(isValidLevel(lh));
  int var=getVarByLevel(lh);
  if (var < 0) {
    return getDomain()->getVariableBound(-var, true);
  }
  else {
    return getDomain()->getVariableBound(var, false);
  }
}

inline int
MEDDLY::expert_forest::getVariableSize(int var) const
{
  return getDomain()->getVariableBound(var, false);
}

inline void
MEDDLY::expert_forest::setNodeLevel(node_handle p, int level)
{
  MEDDLY_DCASSERT(address);
  MEDDLY_DCASSERT(isValidNonterminalIndex(p));
  MEDDLY_DCASSERT(isValidLevel(level));
  address[p].level = level;
}

// --------------------------------------------------
// Managing incoming edge counts
// --------------------------------------------------

inline bool
MEDDLY::expert_forest::trackingInCounts() const
{
  return true;
  // For now; in the future, we might have other garbage collection schemes
  // that do not use reference counts
}

inline int 
// MEDDLY::expert_forest::readInCount(MEDDLY::node_handle p) const
MEDDLY::expert_forest::getNodeInCount(MEDDLY::node_handle p) const
{
  return nodeMan->getCountOf(getNodeAddress(p));
}

inline MEDDLY::node_handle
MEDDLY::expert_forest::linkNode(MEDDLY::node_handle p)
{
  MEDDLY_DCASSERT(isActiveNode(p));
  if (isTerminalNode(p)) return p;
  MEDDLY_DCASSERT(isValidNonterminalIndex(p));
  MEDDLY_DCASSERT(!isPessimistic() || !isZombieNode(p));

  long count = nodeMan->incCountOf(getNodeAddress(p));
  if (1 == count) {
    // Reclaim an orphan node
    stats.reclaimed_nodes++;
    stats.orphan_nodes--;
  }
#ifdef TRACK_DELETIONS
  fprintf(stdout, "\t+Node %d count now %ld\n", p, count);
  fflush(stdout);
#endif
  return p;
}

inline void
MEDDLY::expert_forest::unlinkNode(MEDDLY::node_handle p)
{
  MEDDLY_DCASSERT(isActiveNode(p));
  if (isTerminalNode(p)) return;
  MEDDLY_DCASSERT(isValidNonterminalIndex(p));
  MEDDLY_DCASSERT(!isPessimistic() || !isZombieNode(p));
  MEDDLY_DCASSERT(getNodeInCount(p) > 0);

  long count = nodeMan->decCountOf(getNodeAddress(p));

#ifdef TRACK_DELETIONS
  fprintf(stdout, "\t-Node %d count now %ld\n", p, count);
  fflush(stdout);
#endif
  if (count) return;

  handleNewOrphanNode(p);
}

// --------------------------------------------------
// Managing cache counts
// --------------------------------------------------

inline bool
MEDDLY::expert_forest::trackingCacheCounts() const
{
  return true;
  // For now; in the future, we might use some other scheme
}

inline int 
MEDDLY::expert_forest::getNodeCacheCount(MEDDLY::node_handle p) const
{
  if (isTerminalNode(p)) return 0;
  MEDDLY_DCASSERT(address);
  MEDDLY_DCASSERT(isValidNonterminalIndex(p));
  return address[p].cache_count;
}

inline MEDDLY::node_handle
MEDDLY::expert_forest::cacheNode(MEDDLY::node_handle p)
{
  MEDDLY_DCASSERT(isActiveNode(p));
  if (isTerminalNode(p)) return p;
  MEDDLY_DCASSERT(address);
  MEDDLY_DCASSERT(isValidNonterminalIndex(p));
  address[p].cache_count++;

#ifdef TRACK_CACHECOUNT
  fprintf(stdout, "\t+Node %d is in %d caches\n", p, getNodeCacheCount(p));
  fflush(stdout);
#endif
  return p;
}

inline void
MEDDLY::expert_forest::uncacheNode(MEDDLY::node_handle p)
{
  if (isTerminalNode(p)) return;
  MEDDLY_DCASSERT(address);
  MEDDLY_DCASSERT(isValidNonterminalIndex(p));
  MEDDLY_DCASSERT(
      isActiveNode(p) || (!isActiveNode(p) && isPessimistic()
          && isZombieNode(p)));

  if (isPessimistic() && isZombieNode(p)) {
    // special case: we store the negative of the count.
    MEDDLY_DCASSERT(address[p].cache_count < 0);
    address[p].cache_count++;
    if (0 == address[p].cache_count) {
      stats.zombie_nodes--;
      recycleNodeHandle(p);
    }
    return;
  }
  // we store the actual count.
  MEDDLY_DCASSERT(address[p].cache_count > 0);
  address[p].cache_count--;
#ifdef TRACK_CACHECOUNT
  fprintf(stdout, "\t-Node %d is in %d caches\n", p, address[p].cache_count);
  fflush(stdout);
#endif

  if (0 == address[p].cache_count && 0 == getNodeInCount(p)) {
    MEDDLY_DCASSERT(!isPessimistic());
    stats.orphan_nodes--;
    deleteNode(p);
  }
}

// --------------------------------------------------
// Marking and unmarking nodes
// --------------------------------------------------

inline void
MEDDLY::expert_forest::markAllNodes()
{
  //
  // This is horrible but for now...
  //
  MEDDLY_DCASSERT(address);
  for (node_handle i=1; i<=a_last; i++) {
    address[i].marked = true;
  }
}

inline void
MEDDLY::expert_forest::unmarkAllNodes()
{
  //
  // This is horrible but for now...
  //
  MEDDLY_DCASSERT(address);
  for (node_handle i=1; i<=a_last; i++) {
    address[i].marked = false;
  }
}

inline void
MEDDLY::expert_forest::markNode(node_handle p)
{
  MEDDLY_DCASSERT(address);
  MEDDLY_DCASSERT(isValidNonterminalIndex(p));
  address[p].marked = true;
}

inline void
MEDDLY::expert_forest::unmarkNode(node_handle p)
{
  MEDDLY_DCASSERT(address);
  MEDDLY_DCASSERT(isValidNonterminalIndex(p));
  address[p].marked = false;
}

inline bool 
MEDDLY::expert_forest::isNodeMarked(node_handle p) const
{
  MEDDLY_DCASSERT(address);
  MEDDLY_DCASSERT(isValidNonterminalIndex(p));
  return address[p].marked;
}

// --------------------------------------------------
// Node status
// --------------------------------------------------

inline bool
MEDDLY::expert_forest::isActiveNode(node_handle p) const
{
  return (isValidNodeIndex(p) && (isTerminalNode(p) || getNodeAddress(p) > 0));
}

inline bool
MEDDLY::expert_forest::isZombieNode(node_handle p) const
{
  MEDDLY_DCASSERT(isValidNodeIndex(p));
  MEDDLY_DCASSERT(!isTerminalNode(p));
  return (getNodeCacheCount(p) < 0);
}

inline bool
MEDDLY::expert_forest::isDeletedNode(node_handle p) const
{
  MEDDLY_DCASSERT(isValidNonterminalIndex(p));
  return !(isActiveNode(p) || isZombieNode(p));
}

inline bool
MEDDLY::expert_forest::isTerminalNode(MEDDLY::node_handle p)
{
  return (p < 1);
}

inline bool
MEDDLY::expert_forest::isValidNonterminalIndex(MEDDLY::node_handle node) const
{
  return (node > 0) && (node <= a_last);
}

inline bool
MEDDLY::expert_forest::isValidNodeIndex(MEDDLY::node_handle node) const
{
  return node <= a_last;
}

inline MEDDLY::node_handle
MEDDLY::expert_forest::getLastNode() const
{
  return a_last;
}

//
// Unorganized from here
//


inline int
MEDDLY::expert_forest::getIndexSetCardinality(MEDDLY::node_handle node) const
{
  MEDDLY_DCASSERT(isIndexSet());
  if (isTerminalNode(node)) return (node != 0) ? 1 : 0;
  // yes iff the unhashed extra header is non-zero.
  const int* uhh = (const int*) nodeMan->getUnhashedHeaderOf(getNodeAddress(node));
  MEDDLY_DCASSERT(*uhh > 0);
  return *uhh;
}

inline MEDDLY::node_handle
MEDDLY::expert_forest::getNext(MEDDLY::node_handle p) const
{
  MEDDLY_DCASSERT(address);
  MEDDLY_DCASSERT(isValidNonterminalIndex(p));
  return nodeMan->getNextOf(getNodeAddress(p));
}

inline void
MEDDLY::expert_forest::setNext(MEDDLY::node_handle p, MEDDLY::node_handle n)
{
  MEDDLY_DCASSERT(address);
  MEDDLY_DCASSERT(isValidNonterminalIndex(p));
  nodeMan->setNextOf(getNodeAddress(p), n);
}

inline unsigned
MEDDLY::expert_forest::hash(MEDDLY::node_handle p) const
{
  MEDDLY_DCASSERT(address);
  MEDDLY_DCASSERT(isValidNonterminalIndex(p));
  return hashNode(p);
}

inline bool
MEDDLY::expert_forest::isStale(MEDDLY::node_handle node) const
{
  return isMarkedForDeletion() || (isTerminalNode(node) ? terminalNodesAreStale
      : isPessimistic() ? isZombieNode(node) : (getNodeInCount(node) == 0));
}

inline unsigned
MEDDLY::expert_forest::hashNode(MEDDLY::node_handle p) const
{
  return nodeMan->hashNode(getNodeLevel(p), getNodeAddress(p));
}

inline int
MEDDLY::expert_forest::getSingletonIndex(MEDDLY::node_handle p, MEDDLY::node_handle &down) const
{
  return nodeMan->getSingletonIndex(getNodeAddress(p), down);
}

inline MEDDLY::node_handle
MEDDLY::expert_forest::getSingletonDown(MEDDLY::node_handle node, int index) const
{
  MEDDLY::node_handle down;
  if (getSingletonIndex(node, down) == index) return down;
  return 0;
}

inline MEDDLY::node_handle
MEDDLY::expert_forest::getDownPtr(MEDDLY::node_handle p, int index) const
{
  return nodeMan->getDownPtr(getNodeAddress(p), index);
}

inline void
MEDDLY::expert_forest::getDownPtr(MEDDLY::node_handle p, int index, int& ev,
    MEDDLY::node_handle& dn) const
{
  nodeMan->getDownPtr(getNodeAddress(p), index, ev, dn);
}

inline void
MEDDLY::expert_forest::getDownPtr(MEDDLY::node_handle p, int index, float& ev,
    MEDDLY::node_handle& dn) const
{
  nodeMan->getDownPtr(getNodeAddress(p), index, ev, dn);
}

inline MEDDLY::node_handle
MEDDLY::expert_forest::getTransparentNode() const
{
  return transparent;
}

inline void 
MEDDLY::expert_forest::fillUnpacked(MEDDLY::unpacked_node &un, MEDDLY::node_handle node, unpacked_node::storage_style st2) 
const
{
  const int level = getNodeLevel(node);
  un.bind_to_forest(this, level, getLevelSize(level), true); 
  nodeMan->fillUnpacked(un, getNodeAddress(node), st2);
}


inline MEDDLY::node_handle
MEDDLY::expert_forest::createReducedNode(int in, MEDDLY::unpacked_node *un)
{
  MEDDLY_DCASSERT(un);
  un->computeHash();
  MEDDLY::node_handle q = createReducedHelper(in, *un);
#ifdef TRACK_DELETIONS
  printf("Created node %d\n", q);
#endif
  unpacked_node::recycle(un);
  return q;
}

template<class T>
inline void
MEDDLY::expert_forest::createReducedNode(int in, MEDDLY::unpacked_node *un, T& ev,
      MEDDLY::node_handle& node)
{
  MEDDLY_DCASSERT(un);
  normalize(*un, ev);
  un->computeHash();
  node = createReducedHelper(in, *un);
#ifdef TRACK_DELETIONS
  printf("Created node %d\n", node);
#endif
  unpacked_node::recycle(un);
}

inline bool
MEDDLY::expert_forest::areDuplicates(MEDDLY::node_handle node, const MEDDLY::unpacked_node &nr) const
{
  MEDDLY_DCASSERT(node > 0);
  MEDDLY_DCASSERT(address);
  MEDDLY_CHECK_RANGE(1, node, 1 + a_last);
  if (address[node].level != nr.getLevel())
    return false;
  return nodeMan->areDuplicates(address[node].offset, nr);
}

inline void
MEDDLY::expert_forest::setEdgeSize(char ebytes, bool hashed)
{
  MEDDLY_DCASSERT(0 == edge_bytes);
  edge_bytes = ebytes;
  hash_edge_values = hashed;
}

inline void
MEDDLY::expert_forest::setUnhashedSize(char ubytes)
{
  MEDDLY_DCASSERT(0 == unhashed_bytes);
  unhashed_bytes = ubytes;
}
inline void
MEDDLY::expert_forest::setHashedSize(char hbytes)
{
  MEDDLY_DCASSERT(0 == hashed_bytes);
  hashed_bytes = hbytes;
}

inline bool
MEDDLY::expert_forest::isTimeToGc() const
{
  return isPessimistic() ? (stats.zombie_nodes > deflt.zombieTrigger)
      : (stats.orphan_nodes > deflt.orphanTrigger);
}

inline void
MEDDLY::expert_forest::moveNodeOffset(MEDDLY::node_handle node, node_address /*old_addr*/,
    node_address new_addr)
{
  MEDDLY_DCASSERT(address);
  MEDDLY_DCASSERT(old_addr == address[node].offset);
  address[node].offset = new_addr;
}

inline void
MEDDLY::expert_forest::getVariableOrder(int* level2var) const
{
  // Assume sufficient space has been allocated for order
  level2var[0] = 0;
  for (int i = 1; i < getNumVariables() + 1; i++) {
    level2var[i] = var_order->getVarByLevel(i);
  }
}

inline std::shared_ptr<const MEDDLY::variable_order>
MEDDLY::expert_forest::variableOrder() const
{
  return var_order;
}

// ******************************************************************
// *                                                                *
// *                     inlined opname methods                     *
// *                                                                *
// ******************************************************************

inline int
MEDDLY::opname::getIndex() const
{
  return index;
}

inline const char*
MEDDLY::opname::getName() const
{
  return name;
}

// ******************************************************************
// *                                                                *
// *               inlined specialized_opname methods               *
// *                                                                *
// ******************************************************************

inline void
MEDDLY::specialized_opname::arguments::setAutoDestroy(bool destroy)
{
  destroyWhenDone = destroy;
}

inline bool
MEDDLY::specialized_opname::arguments::autoDestroy() const
{
  return destroyWhenDone;
}

// ******************************************************************
// *                                                                *
// *                inlined numerical_opname methods                *
// *                                                                *
// ******************************************************************

inline MEDDLY::specialized_operation*
MEDDLY::numerical_opname::buildOperation(const dd_edge &x_ind,
    const dd_edge &A, const dd_edge &y_ind) const
{
  numerical_args na(x_ind, A, y_ind);
  na.setAutoDestroy(false); // na will be destroyed when we return
  return buildOperation(&na);
}


// ******************************************************************
// *                                                                *
// *                inlined satpregen_opname methods                *
// *                                                                *
// ******************************************************************


inline bool
MEDDLY::satpregen_opname::pregen_relation::isFinalized() const
{
  return 0 == next;
}

inline MEDDLY::node_handle*
MEDDLY::satpregen_opname::pregen_relation::arrayForLevel(int k) const
{
  MEDDLY_DCASSERT(isFinalized());
  MEDDLY_CHECK_RANGE(1, k, K + 1);
  if (level_index) {
    // "by events"
    if (level_index[k - 1] > level_index[k]) {
      return events + level_index[k];
    }
    else {
      // empty list
      return 0;
    }
  }
  else {
    // "by levels"
    if (events[k])
      return events + k;
    else
      return 0;
  }
}

inline int
MEDDLY::satpregen_opname::pregen_relation::lengthForLevel(int k) const
{
  MEDDLY_DCASSERT(isFinalized());
  MEDDLY_CHECK_RANGE(1, k, K + 1);
  if (level_index) {
    // "by events"
    return level_index[k - 1] - level_index[k];
  }
  else {
    // "by levels"
    return events[k] ? 1 : 0;
  }
}

inline MEDDLY::forest*
MEDDLY::satpregen_opname::pregen_relation::getInForest() const
{
  return insetF;
}

inline MEDDLY::forest*
MEDDLY::satpregen_opname::pregen_relation::getRelForest() const
{
  return mxdF;
}

inline MEDDLY::forest*
MEDDLY::satpregen_opname::pregen_relation::getOutForest() const
{
  return outsetF;
}


// ******************************************************************
// *                                                                *
// *                 inlined  satotf_opname methods                 *
// *                                                                *
// ******************************************************************


inline MEDDLY::expert_forest*
MEDDLY::satotf_opname::subevent::getForest() {
  return f;
} 

inline int
MEDDLY::satotf_opname::subevent::getNumVars() const {
  return num_vars;
}

inline const int*
MEDDLY::satotf_opname::subevent::getVars() const {
  return vars;
}

inline int
MEDDLY::satotf_opname::subevent::getTop() const {
  return top;
}

inline bool
MEDDLY::satotf_opname::subevent::isFiring() const {
  return is_firing;
}

inline bool
MEDDLY::satotf_opname::subevent::isEnabling() const {
  return !is_firing;
}

inline const MEDDLY::dd_edge&
MEDDLY::satotf_opname::subevent::getRoot() const {
  return root;
}

#if 0
inline void
MEDDLY::satotf_opname::subevent::setRoot(const MEDDLY::dd_edge& dd) {
  root = dd;
}
#endif

// ****************************************************************************

inline MEDDLY::expert_forest*
MEDDLY::satotf_opname::otf_relation::getInForest() const
{
  return insetF;
}

inline MEDDLY::expert_forest*
MEDDLY::satotf_opname::otf_relation::getRelForest() const
{
  return mxdF;
}

inline MEDDLY::expert_forest*
MEDDLY::satotf_opname::otf_relation::getOutForest() const
{
  return outsetF;
}

inline bool
MEDDLY::satotf_opname::otf_relation::isConfirmed(int level, int i) const
{
  if (level < num_levels &&  i >= 0) {
    return (i < insetF->getLevelSize(level) && confirmed[level][i]);
  }
  throw MEDDLY::error::INVALID_ARGUMENT;
}

inline int
MEDDLY::satotf_opname::otf_relation::getNumOfEvents(int level) const
{
  MEDDLY_CHECK_RANGE(1, level, num_levels);
  return num_events_by_top_level[level];
}

inline MEDDLY::node_handle
MEDDLY::satotf_opname::otf_relation::getEvent(int level, int i)
{
  MEDDLY_CHECK_RANGE(0, i, getNumOfEvents(level));
  return events_by_top_level[level][i]->getRoot().getNode();
}


inline bool
MEDDLY::satotf_opname::otf_relation::rebuildEvent(int level, int i)
{
  MEDDLY_CHECK_RANGE(0, i, getNumOfEvents(level));
  return events_by_top_level[level][i]->rebuild();
}

inline const bool*
MEDDLY::satotf_opname::otf_relation::getLocalStates(int level)
{
  MEDDLY_CHECK_RANGE(0, level, num_levels);
  return confirmed[level];
}

inline int
MEDDLY::satotf_opname::otf_relation::getNumConfirmed(int level) const
{
  MEDDLY_CHECK_RANGE(0, level, num_levels);
  return num_confirmed[level];
}


// ******************************************************************
// *                                                                *
// *                 inlined  compute_table methods                 *
// *                                                                *
// ******************************************************************


inline MEDDLY::operation*
MEDDLY::compute_table::search_key::getOp() const
{
  return op;
}

inline void
MEDDLY::compute_table::search_result::setValid()
{
  is_valid = true;
}
inline void
MEDDLY::compute_table::search_result::setInvalid()
{
  is_valid = false;
}
inline
MEDDLY::compute_table::search_result::operator bool() const
{
  return is_valid;
}

// convenience methods, for grabbing edge values
inline void
MEDDLY::compute_table::readEV(const MEDDLY::node_handle* p, int &ev)
{
  ev = p[0];
}
inline void
MEDDLY::compute_table::readEV(const MEDDLY::node_handle* p, float &ev)
{
  float* f = (float*) p;
  ev = f[0];
}

inline const MEDDLY::compute_table::stats&
MEDDLY::compute_table::getStats()
{
  return perf;
}


// ******************************************************************
// *                                                                *
// *                   inlined  operation methods                   *
// *                                                                *
// ******************************************************************


inline void
MEDDLY::operation::setAnswerForest(const MEDDLY::expert_forest* f)
{
  discardStaleHits = f ? f->getNodeDeletion()
      == MEDDLY::forest::policies::PESSIMISTIC_DELETION : false; // shouldn't be possible, so we'll do what's fastest.
}

inline void
MEDDLY::operation::registerInForest(MEDDLY::forest* f)
{
  if (f)
    f->registerOperation(this);
}

inline void
MEDDLY::operation::unregisterInForest(MEDDLY::forest* f)
{
  if (f)
    f->unregisterOperation(this);
}

inline MEDDLY::compute_table::search_key*
MEDDLY::operation::useCTkey()
{
  MEDDLY_DCASSERT(CT);
  compute_table::search_key* ans;
  if (CT_free_keys) {
    ans = CT_free_keys;
    CT_free_keys = ans->next;
  }
  else {
    ans = CT->initializeSearchKey(this);
  }
  return ans;
}

inline bool
MEDDLY::operation::isMarkedForDeletion() const
{
  return is_marked_for_deletion;
}

inline void
MEDDLY::operation::setNext(operation* n)
{
  next = n;
}

inline MEDDLY::operation*
MEDDLY::operation::getNext()
{
  return next;
}

inline bool
MEDDLY::operation::usesMonolithicComputeTable()
{
  return Monolithic_CT;
}

inline int
MEDDLY::operation::getIndex() const
{
  return oplist_index;
}

inline MEDDLY::operation*
MEDDLY::operation::getOpWithIndex(int i)
{
  return op_list[i];
}

inline int
MEDDLY::operation::getOpListSize()
{
  return list_size;
}

inline const char*
MEDDLY::operation::getName() const
{
  return theOpName->getName();
}

inline const MEDDLY::opname*
MEDDLY::operation::getOpName() const
{
  return theOpName;
}

inline int
MEDDLY::operation::getKeyLength() const
{
  return key_length;
}

inline int
MEDDLY::operation::getAnsLength() const
{
  return ans_length;
}

inline int
MEDDLY::operation::getCacheEntryLength() const
{
  return key_length + ans_length;
}

inline bool
MEDDLY::operation::isEntryStale(const MEDDLY::node_handle* data)
{
  return (is_marked_for_deletion || isStaleEntry(data));
}

inline void
MEDDLY::operation::doneCTkey(compute_table::search_key* K)
{
  MEDDLY_DCASSERT(K);
  K->next = CT_free_keys;
  CT_free_keys = K;
}

inline bool
MEDDLY::operation::shouldStaleCacheHitsBeDiscarded() const
{
  return discardStaleHits;
}


// ******************************************************************
// *                                                                *
// *                inlined  unary_operation methods                *
// *                                                                *
// ******************************************************************

inline bool
MEDDLY::unary_operation::matches(const MEDDLY::expert_forest* arg,
    const MEDDLY::expert_forest* res) const
{
  return (arg == argF && res == resF);
}

inline bool
MEDDLY::unary_operation::matches(const MEDDLY::expert_forest* arg, opnd_type res) const
{
  return (arg == argF && resultType == res);
}

inline bool
MEDDLY::unary_operation::checkForestCompatibility() const
{
  if (resultType == FOREST) {
    auto o1 = argF->variableOrder();
    auto o2 = resF->variableOrder();
    return o1->is_compatible_with(*o2);
  }
  else {
    return true;
  }
}

inline void
MEDDLY::unary_operation::compute(const dd_edge &arg, dd_edge &res)
{
  if (!checkForestCompatibility()) {
    throw error(error::INVALID_OPERATION);
  }
  computeDDEdge(arg, res);
}

// ******************************************************************
// *                                                                *
// *                inlined binary_operation methods                *
// *                                                                *
// ******************************************************************


inline bool
MEDDLY::binary_operation::matches(const MEDDLY::expert_forest* arg1,
    const MEDDLY::expert_forest* arg2, const MEDDLY::expert_forest* res) const
{
  return (arg1 == arg1F && arg2 == arg2F && res == resF);
}

inline void
MEDDLY::binary_operation::operationCommutes()
{
  can_commute = (arg1F == arg2F);
}

inline bool
MEDDLY::binary_operation::checkForestCompatibility() const
{
  auto o1 = arg1F->variableOrder();
  auto o2 = arg2F->variableOrder();
  auto o3 = resF->variableOrder();
  return o1->is_compatible_with(*o2) && o1->is_compatible_with(*o3);
}

inline void
MEDDLY::binary_operation::compute(const dd_edge &ar1, const dd_edge &ar2, dd_edge &res)
{
  if (!checkForestCompatibility()) {
    throw error(error::INVALID_OPERATION);
  }
  computeDDEdge(ar1, ar2, res);
}

// ******************************************************************
// *                                                                *
// *                       inlined  functions                       *
// *                                                                *
// ******************************************************************


inline MEDDLY::unary_operation*
MEDDLY::getOperation(const unary_opname* code, const dd_edge& arg,
    const dd_edge& res)
{
  return getOperation(code, (MEDDLY::expert_forest*) arg.getForest(),
      (MEDDLY::expert_forest*) res.getForest());
}

inline MEDDLY::unary_operation*
MEDDLY::getOperation(const unary_opname* code, const dd_edge& arg,
    opnd_type res)
{
  return getOperation(code, (MEDDLY::expert_forest*) arg.getForest(), res);
}

inline MEDDLY::binary_operation*
MEDDLY::getOperation(const binary_opname* code, const dd_edge& arg1,
    const dd_edge& arg2, const dd_edge& res)
{
  return getOperation(code, (MEDDLY::expert_forest*) arg1.getForest(),
      (MEDDLY::expert_forest*) arg2.getForest(), (MEDDLY::expert_forest*) res.getForest());
}

#endif



