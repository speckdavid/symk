
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

#include "evmdd_timesreal.h"

// ******************************************************************
// *                                                                *
// *                                                                *
// *                    evmdd_timesreal  methods                    *
// *                                                                *
// *                                                                *
// ******************************************************************

MEDDLY::evmdd_timesreal::evmdd_timesreal(int dsl, domain *d, const policies &p,int* level_reduction_rule)
 : evmdd_forest(dsl, d, REAL, EVTIMES, p,level_reduction_rule)
{
  setEdgeSize(sizeof(float), true);
  initializeForest();
}

MEDDLY::evmdd_timesreal::~evmdd_timesreal()
{ }

void MEDDLY::evmdd_timesreal::createEdge(float val, dd_edge &e)
{
  createEdgeTempl<OP, float>(val, e);
#ifdef DEVELOPMENT_CODE
  validateIncounts(true);
#endif
}

void MEDDLY::evmdd_timesreal
::createEdge(const int* const* vlist, const float* terms, int N, dd_edge &e)
{
  // binary_operation* unionOp = getOperation(PLUS, this, this, this);
  binary_operation* unionOp = 0;  // for now
  enlargeStatics(N);
  enlargeVariables(vlist, N, false);

  evmdd_edgemaker<OP, float>
  EM(this, vlist, terms, order, N, getDomain()->getNumVariables(), unionOp);

  float ev;
  node_handle ep;
  EM.createEdge(ev, ep);
  e.set(ep, ev);
#ifdef DEVELOPMENT_CODE
  validateIncounts(true);
#endif
}

void MEDDLY::evmdd_timesreal
::createEdgeForVar(int vh, bool vp, const float* terms, dd_edge& a)
{
  createEdgeForVarTempl<OP, float>(vh, vp, terms, a);
#ifdef DEVELOPMENT_CODE
  validateIncounts(true);
#endif
}

void MEDDLY::evmdd_timesreal
::evaluate(const dd_edge &f, const int* vlist, float &term) const
{
  evaluateT<OP, float>(f, vlist, term);
}


bool MEDDLY::evmdd_timesreal
::areEdgeValuesEqual(const void* eva, const void* evb) const
{
  float val1, val2;
  OP::readValue(eva, val1);
  OP::readValue(evb, val2);
  return !OP::notClose(val1, val2);
}

bool MEDDLY::evmdd_timesreal::isRedundant(const unpacked_node &nb) const
{
  return isRedundantTempl<OP>(nb);
}

bool MEDDLY::evmdd_timesreal::isIdentityEdge(const unpacked_node &nb, int i) const
{
  return isIdentityEdgeTempl<OP>(nb, i); 
}


void MEDDLY::evmdd_timesreal::normalize(unpacked_node &nb, float& ev) const
{
  int minindex = -1;
  int stop = nb.isSparse() ? nb.getNNZs() : nb.getSize();
  for (int i=0; i<stop; i++) {
    if (0==nb.d(i)) continue;
    if ((minindex < 0) || (nb.ef(i) < nb.ef(minindex))) {
      minindex = i;
    }
  }
  if (minindex < 0) return; // this node will eventually be reduced to "0".
  ev = nb.ef(minindex);
  for (int i=0; i<stop; i++) {
    if (0==nb.d(i)) continue;
    float temp;
    nb.getEdge(i, temp);
    temp /= ev;
    nb.setEdge(i, temp);
  }
}

void MEDDLY::evmdd_timesreal::showEdgeValue(output &s, const void* edge) const
{
  OP::show(s, edge);
}

void MEDDLY::evmdd_timesreal::writeEdgeValue(output &s, const void* edge) const
{
  OP::write(s, edge);
}

void MEDDLY::evmdd_timesreal::readEdgeValue(input &s, void* edge)
{
  OP::read(s, edge);
}

const char* MEDDLY::evmdd_timesreal::codeChars() const
{
  return "dd_etvr";
}

// ******************************************************************
// *                                                                *
// *                                                                *
// *           evmdd_timesreal::evtrmdd_iterator  methods           *
// *                                                                *
// *                                                                *
// ******************************************************************

MEDDLY::evmdd_timesreal::evtrmdd_iterator::evtrmdd_iterator(const expert_forest *F)
: iterator(F)
{
  int N = F->getNumVariables();
  acc_evs = new double[N+1];
}

MEDDLY::evmdd_timesreal::evtrmdd_iterator::~evtrmdd_iterator()
{
  delete[] acc_evs;
}

void MEDDLY::evmdd_timesreal::evtrmdd_iterator::getValue(float &tv) const
{
  MEDDLY_DCASSERT(acc_evs);
  tv = acc_evs[0];
}

bool MEDDLY::evmdd_timesreal::evtrmdd_iterator::start(const dd_edge &e)
{
  if (F != e.getForest()) {
    throw error(error::FOREST_MISMATCH);
  }

  MEDDLY_DCASSERT(acc_evs);
  float ev;
  e.getEdgeValue(ev);
  acc_evs[maxLevel] = ev;

  return first(maxLevel, e.getNode());
}

bool MEDDLY::evmdd_timesreal::evtrmdd_iterator::next()
{
  MEDDLY_DCASSERT(F);
  MEDDLY_DCASSERT(!F->isForRelations());
  MEDDLY_DCASSERT(index);
  MEDDLY_DCASSERT(nzp);
  MEDDLY_DCASSERT(path);
  MEDDLY_DCASSERT(acc_evs);

  int k;
  node_handle down = 0;
  for (k=1; k<=maxLevel; k++) {
    nzp[k]++;
    if (nzp[k] < path[k].getNNZs()) {
      index[k] = path[k].i(nzp[k]);
      down = path[k].d(nzp[k]);
      MEDDLY_DCASSERT(down);
      float ev;
      path[k].getEdge(nzp[k], ev);
      acc_evs[k-1] = acc_evs[k] * ev;
      break;
    }
  }
  level_change = k;
  if (k>maxLevel) {
    return false;
  }

  return first(k-1, down);
}

bool MEDDLY::evmdd_timesreal::evtrmdd_iterator::first(int k, node_handle down)
{
  MEDDLY_DCASSERT(F);
  MEDDLY_DCASSERT(!F->isForRelations());
  MEDDLY_DCASSERT(index);
  MEDDLY_DCASSERT(nzp);
  MEDDLY_DCASSERT(path);
  MEDDLY_DCASSERT(acc_evs);

  if (0==down) return false;

  for ( ; k; k--) {
    MEDDLY_DCASSERT(down);
    int kdn = F->getNodeLevel(down);
    MEDDLY_DCASSERT(kdn <= k);
    if (kdn < k)  F->initRedundantReader(path[k], k, 1.0f, down, false);
    else          F->initNodeReader(path[k], down, false);
    nzp[k] = 0;
    index[k] = path[k].i(0);
    down = path[k].d(0);
    float ev;
    path[k].getEdge(0, ev);
    acc_evs[k-1] = acc_evs[k] * ev;
  }
  // save the terminal value
  index[0] = down;
  return true;
}

