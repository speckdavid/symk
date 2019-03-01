
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



#include "defines.h"

// #define DEBUG_CLEANUP

// #define DEBUG_ITER_BEGIN

// ******************************************************************
// *                                                                *
// *                                                                *
// *                  enumerator::iterator methods                  *
// *                                                                *
// *                                                                *
// ******************************************************************

MEDDLY::enumerator::iterator::iterator(const expert_forest* f)
{
  prindex = 0;
  F = f;
  if (0==f) {
    rawpath = path = 0;
    rawnzp = nzp = 0;
    rawindex = index = 0;
    minLevel = 0;
    maxLevel = 0;
    level_change = 0;
    return;
  }
  int N = f->getNumVariables();
  maxLevel = N;
  if (f->isForRelations()) {
    rawpath = new unpacked_node[2*N+1];
    rawnzp = new int[2*N+1];
    rawindex = new int[2*N+1];
    path = rawpath + N;
    nzp = rawnzp + N;
    index = rawindex + N;
    minLevel = -N;
  } else {
    rawpath = new unpacked_node[N+1];
    rawnzp = new int[N+1];
    rawindex = new int[N+1];
    path = rawpath;
    nzp = rawnzp;
    index = rawindex;
    minLevel = 1;
  }
  level_change = N+1;
}

MEDDLY::enumerator::iterator::~iterator()
{
  delete[] rawpath;
  delete[] rawnzp;
  delete[] rawindex;
  delete[] prindex;
}

bool MEDDLY::enumerator::iterator::start(const dd_edge &e)
{
  throw error(error::INVALID_OPERATION);
}

bool MEDDLY::enumerator::iterator::start(const dd_edge &e, const int* m)
{
  throw error(error::INVALID_OPERATION);
}

const int* MEDDLY::enumerator::iterator::getPrimedAssignments()
{
  if (0==F) return 0;
  if (!F->isForRelations()) return 0;
  if (0==prindex) {
    prindex = new int[1+maxLevel];
    prindex[0] = 0;
  }
  MEDDLY_DCASSERT(index);
  for (int k=maxLevel; k; k--) {
    prindex[k] = index[-k];
  }
  return prindex; 
}

void MEDDLY::enumerator::iterator::getValue(int &) const
{
  throw error(error::TYPE_MISMATCH);
}

void MEDDLY::enumerator::iterator::getValue(float &) const
{
  throw error(error::TYPE_MISMATCH);
}

// ******************************************************************
// *                                                                *
// *                                                                *
// *                       enumerator methods                       *
// *                                                                *
// *                                                                *
// ******************************************************************

MEDDLY::enumerator::enumerator()
{
  I = 0;
  is_valid = false;
  T = EMPTY;
}

MEDDLY::enumerator::enumerator(type t, const forest* F)
{
  I = 0;
  init(t, F);
  is_valid = false;
}

MEDDLY::enumerator::enumerator(const dd_edge &e)
{
  I = 0;
  init(FULL, e.getForest());
  start(e);
}

MEDDLY::enumerator::~enumerator()
{
  delete I;
}

void MEDDLY::enumerator::init(type t, const forest* f)
{
  delete I;
  is_valid = false;

  T = t;
  const expert_forest* F = smart_cast<const expert_forest*>(f);
  if (0==F) {
    I = 0;
    return;
  }

  switch (t) {
    case FULL:
      I = F->makeFullIter();
      break;

    case ROW_FIXED:
      I = F->makeFixedRowIter();
      break;

    case COL_FIXED:
      I = F->makeFixedColumnIter();
      break;

    default:
      I = 0;
      return;
  }
  if (I->build_error()) {
    delete I;
    I = 0;
  } 
}

void MEDDLY::enumerator::start(const dd_edge &e)
{
  if (0==I) return;
  if (FULL != T) throw error(error::MISCELLANEOUS);
  MEDDLY_DCASSERT(I);
  is_valid = I->start(e);
}

void MEDDLY::enumerator::startFixedRow(const dd_edge &e, const int* minterm)
{
  if (0==I) return;
  if (ROW_FIXED != T) throw error(error::MISCELLANEOUS);
  MEDDLY_DCASSERT(I);
  is_valid = I->start(e, minterm);
}

void MEDDLY::enumerator::startFixedColumn(const dd_edge &e, const int* minterm)
{
  if (0==I) return;
  if (COL_FIXED != T) throw error(error::MISCELLANEOUS);
  MEDDLY_DCASSERT(I);
  is_valid = I->start(e, minterm);
}

