
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
#include "maxmin_range.h"

namespace MEDDLY {

  class range_int;
  class range_real;

  class maxrange_int;
  class minrange_int;

  class maxrange_real;
  class minrange_real;

  class maxrange_opname;
  class minrange_opname;
};

// ******************************************************************
// *                                                                *
// *                        range_int  class                        *
// *                                                                *
// ******************************************************************

/// Abstract base class: max or min range that returns an integer.
class MEDDLY::range_int : public unary_operation {
  public:
    range_int(const unary_opname* oc, expert_forest* arg);

    // common
    virtual bool isStaleEntry(const node_handle* entryData);
    virtual void discardEntry(const node_handle* entryData);
    virtual void showEntry(output &strm, const node_handle* entryData) const;
  
  protected:
    inline compute_table::search_key* 
    findResult(node_handle a, int &b) 
    {
      compute_table::search_key* CTsrch = useCTkey();
      MEDDLY_DCASSERT(CTsrch);
      CTsrch->reset();
      CTsrch->writeNH(a);
      compute_table::search_result &cacheFind = CT->find(CTsrch);
      if (!cacheFind) return CTsrch;
      cacheFind.read(b);
      doneCTkey(CTsrch);
      return 0;
    }
    inline long saveResult(compute_table::search_key* Key, 
      node_handle a, int &b) 
    {
      argF->cacheNode(a);
      compute_table::entry_builder &entry = CT->startNewEntry(Key);
      // entry.writeKeyNH(argF->cacheNode(a));
      entry.writeResult(b);
      CT->addEntry();
      return b;
    }
};

MEDDLY::range_int::range_int(const unary_opname* oc, expert_forest* arg)
 : unary_operation(oc, 1, 1, arg, INTEGER)
{
}

bool MEDDLY::range_int::isStaleEntry(const node_handle* data)
{
  return argF->isStale(data[0]);
}

void MEDDLY::range_int::discardEntry(const node_handle* data)
{
  argF->uncacheNode(data[0]);
}

void MEDDLY::range_int::showEntry(output &strm, const node_handle* data) const
{
  strm  << "[" << getName() << "(" << long(data[0]) 
        << "): " << long(data[1]) << "(L)]";
}


// ******************************************************************
// *                                                                *
// *                        range_real class                        *
// *                                                                *
// ******************************************************************

/// Abstract base class: max or min range that returns a real.
class MEDDLY::range_real : public unary_operation {
  public:
    range_real(const unary_opname* oc, expert_forest* arg);

    // common
    virtual bool isStaleEntry(const node_handle* entryData);
    virtual void discardEntry(const node_handle* entryData);
    virtual void showEntry(output &strm, const node_handle* entryData) const;

  protected:
    inline compute_table::search_key* findResult(node_handle a, float &b) {
      compute_table::search_key* CTsrch = useCTkey();
      MEDDLY_DCASSERT(CTsrch);
      CTsrch->reset();
      CTsrch->writeNH(a);
      compute_table::search_result &cacheFind = CT->find(CTsrch);
      if (!cacheFind) return CTsrch;
      cacheFind.read(b);
      doneCTkey(CTsrch);
      return 0;
    }
    inline float saveResult(compute_table::search_key* Key, 
      node_handle a, float &b) 
    {
      argF->cacheNode(a);
      compute_table::entry_builder &entry = CT->startNewEntry(Key);
      // entry.writeKeyNH(argF->cacheNode(a));
      entry.writeResult(b);
      return b;
    }
};

MEDDLY::range_real::range_real(const unary_opname* oc, expert_forest* arg)
 : unary_operation(oc, 1, sizeof(float) / sizeof(int), arg, INTEGER)
{
}

bool MEDDLY::range_real::isStaleEntry(const node_handle* data)
{
  return argF->isStale(data[0]);
}

void MEDDLY::range_real::discardEntry(const node_handle* data)
{
  argF->uncacheNode(data[0]);
}

void MEDDLY::range_real::showEntry(output &strm, const node_handle* data) const
{
  double answer;
  memcpy(&answer, data+1, sizeof(double));
  strm  << "[" << getName() << "(" << long(data[0]) << "): ";
  strm.put(answer, 0, 0, 'e');
  strm.put(']');
}


// ******************************************************************
// *                                                                *
// *                       maxrange_int class                       *
// *                                                                *
// ******************************************************************

/// Max range, returns an integer
class MEDDLY::maxrange_int : public range_int {
public:
  maxrange_int(const unary_opname* oc, expert_forest* arg)
    : range_int(oc, arg) { }
  virtual void compute(const dd_edge &arg, long &res) {
    res = compute_r(arg.getNode());
  }
  int compute_r(node_handle a);
};

int MEDDLY::maxrange_int::compute_r(node_handle a)
{
  // Terminal case
  if (argF->isTerminalNode(a)) return expert_forest::int_Tencoder::handle2value(a);
  
  // Check compute table
  int max;
  compute_table::search_key* Key = findResult(a, max);
  if (0==Key) return max;

  // Initialize node reader
  unpacked_node* A = unpacked_node::newFromNode(argF, a, false);

  // recurse
  max = compute_r(A->d(0));
  for (int i=1; i<A->getSize(); i++) {
    max = MAX(max, compute_r(A->d(i)));
  }

  // Cleanup
  unpacked_node::recycle(A);

  // Add entry to compute table
  return saveResult(Key, a, max);
}



// ******************************************************************
// *                                                                *
// *                       minrange_int class                       *
// *                                                                *
// ******************************************************************

/// Min range, returns an integer
class MEDDLY::minrange_int : public range_int {
public:
  minrange_int(const unary_opname* oc, expert_forest* arg)
    : range_int(oc, arg) { }
  virtual void compute(const dd_edge &arg, long &res) {
    res = compute_r(arg.getNode());
  }
  int compute_r(node_handle a);
};

int MEDDLY::minrange_int::compute_r(node_handle a)
{
  // Terminal case
  if (argF->isTerminalNode(a)) return expert_forest::int_Tencoder::handle2value(a);
  
  // Check compute table
  int min;
  compute_table::search_key* Key = findResult(a, min);
  if (0==Key) return min;

  // Initialize node reader
  unpacked_node* A = unpacked_node::newFromNode(argF, a, false);

  // recurse
  min = compute_r(A->d(0));
  for (int i=1; i<A->getSize(); i++) {
    min = MIN(min, compute_r(A->d(i)));
  }

  // Cleanup
  unpacked_node::recycle(A);

  // Add entry to compute table
  return saveResult(Key, a, min);
}



// ******************************************************************
// *                                                                *
// *                      maxrange_real  class                      *
// *                                                                *
// ******************************************************************

/// Max range, returns a real
class MEDDLY::maxrange_real : public range_real {
public:
  maxrange_real(const unary_opname* oc, expert_forest* arg)
    : range_real(oc, arg) { }
  virtual void compute(const dd_edge &arg, double &res) {
    res = compute_r(arg.getNode());
  }
  float compute_r(node_handle a);
};

float MEDDLY::maxrange_real::compute_r(node_handle a)
{
  // Terminal case
  if (argF->isTerminalNode(a)) return expert_forest::float_Tencoder::handle2value(a);
  
  // Check compute table
  float max;
  compute_table::search_key* Key = findResult(a, max);
  if (0==Key) return max;

  // Initialize node reader
  unpacked_node* A = unpacked_node::newFromNode(argF, a, false);

  // recurse
  max = compute_r(A->d(0));
  for (int i=1; i<A->getSize(); i++) {
    max = MAX(max, compute_r(A->d(i)));
  }

  // Cleanup
  unpacked_node::recycle(A);

  // Add entry to compute table
  return saveResult(Key, a, max);
}



// ******************************************************************
// *                                                                *
// *                      minrange_real  class                      *
// *                                                                *
// ******************************************************************

/// Min range, returns a real
class MEDDLY::minrange_real : public range_real {
public:
  minrange_real(const unary_opname* oc, expert_forest* arg)
    : range_real(oc, arg) { }
  virtual void compute(const dd_edge &arg, double &res) {
    res = compute_r(arg.getNode());
  }
  float compute_r(node_handle a);
};

float MEDDLY::minrange_real::compute_r(node_handle a)
{
  // Terminal case
  if (argF->isTerminalNode(a)) return expert_forest::float_Tencoder::handle2value(a);
  
  // Check compute table
  float min;
  compute_table::search_key* Key = findResult(a, min);
  if (0==Key) return min;

  // Initialize node reader
  unpacked_node* A = unpacked_node::newFromNode(argF, a, false);

  // recurse
  min = compute_r(A->d(0));
  for (int i=1; i<A->getSize(); i++) {
    min = MIN(min, compute_r(A->d(i)));
  }

  // Cleanup
  unpacked_node::recycle(A);

  // Add entry to compute table
  return saveResult(Key, a, min);
}



// ******************************************************************
// *                                                                *
// *                     maxrange_opname  class                     *
// *                                                                *
// ******************************************************************

class MEDDLY::maxrange_opname : public unary_opname {
  public:
    maxrange_opname();
    virtual unary_operation*
      buildOperation(expert_forest* ar, opnd_type res) const;
};

MEDDLY::maxrange_opname::maxrange_opname() : unary_opname("Max_range")
{
}

MEDDLY::unary_operation*
MEDDLY::maxrange_opname::buildOperation(expert_forest* ar, opnd_type res) const
{
  if (0==ar) return 0;

  if (ar->getEdgeLabeling() != forest::MULTI_TERMINAL)
    throw error(error::NOT_IMPLEMENTED);

  switch (res) {
    case INTEGER:
      if (forest::INTEGER != ar->getRangeType())
        throw error(error::TYPE_MISMATCH);
      return new maxrange_int(this,  ar);

    case REAL:
      if (forest::REAL != ar->getRangeType())
        throw error(error::TYPE_MISMATCH);
      return new maxrange_real(this,  ar);

    default:
      throw error(error::TYPE_MISMATCH);
  } // switch

  throw error(error::MISCELLANEOUS);
}

// ******************************************************************
// *                                                                *
// *                     minrange_opname  class                     *
// *                                                                *
// ******************************************************************

class MEDDLY::minrange_opname : public unary_opname {
  public:
    minrange_opname();
    virtual unary_operation*
      buildOperation(expert_forest* ar, opnd_type res) const;
};

MEDDLY::minrange_opname::minrange_opname() : unary_opname("Min_range")
{
}

MEDDLY::unary_operation*
MEDDLY::minrange_opname::buildOperation(expert_forest* ar, opnd_type res) const
{
  if (0==ar) return 0;

  if (ar->getEdgeLabeling() != forest::MULTI_TERMINAL)
    throw error(error::NOT_IMPLEMENTED);

  switch (res) {
    case INTEGER:
      if (forest::INTEGER != ar->getRangeType())
        throw error(error::TYPE_MISMATCH);
      return new minrange_int(this,  ar);

    case REAL:
      if (forest::REAL != ar->getRangeType())
        throw error(error::TYPE_MISMATCH);
      return new minrange_real(this,  ar);

    default:
      throw error(error::TYPE_MISMATCH);
  } // switch

  throw error(error::MISCELLANEOUS);
}


// ******************************************************************
// *                                                                *
// *                           Front  end                           *
// *                                                                *
// ******************************************************************

MEDDLY::unary_opname* MEDDLY::initializeMaxRange()
{
  return new maxrange_opname;
}

MEDDLY::unary_opname* MEDDLY::initializeMinRange()
{
  return new minrange_opname;
}

