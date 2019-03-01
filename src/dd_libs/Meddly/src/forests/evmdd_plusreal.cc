
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

#include "evmdd_plusreal.h"

// ******************************************************************
// *                                                                *
// *                                                                *
// *                    evmdd_timesreal  methods                    *
// *                                                                *
// *                                                                *
// ******************************************************************

MEDDLY::evmdd_plusreal::evmdd_plusreal(int dsl, domain *d, const policies &p, int* level_reduction_rule)
 : evmdd_forest(dsl, d, REAL, EVPLUS, p, level_reduction_rule)
{
  setEdgeSize(sizeof(float), true);
  initializeForest();
}

MEDDLY::evmdd_plusreal::~evmdd_plusreal()
{ }

void MEDDLY::evmdd_plusreal::createEdge(float val, dd_edge &e)
{
  createEdgeTempl<OP, float>(val, e);
#ifdef DEVELOPMENT_CODE
  validateIncounts(true);
#endif
}

void MEDDLY::evmdd_plusreal
::createEdge(const int* const* vlist, const float* terms, int N, dd_edge &e)
{
  // binary_operation* unionOp = getOperation(PLUS, this, this, this);
  binary_operation* unionOp = 0;  // for now
  enlargeStatics(N);
  enlargeVariables(vlist, N, false);

  // TODOS(speckd): Necessary???
  int num_vars = getNumVariables();
  // Create vlist following the mapping between variable and level
  int** ordered_vlist=static_cast<int**>(malloc(N*sizeof(int*)+(num_vars+1)*N*sizeof(int)));
  if(ordered_vlist==0){
	  throw error(error::INSUFFICIENT_MEMORY);
  }

  ordered_vlist[0]=reinterpret_cast<int*>(&ordered_vlist[N]);
  for(int i=1; i<N; i++) {
	  ordered_vlist[i]=(ordered_vlist[i-1]+num_vars+1);
  }
  for(int i=0; i<=num_vars; i++) {
	  int level=getLevelByVar(i);
	  for(int j=0; j<N; j++) {
		  ordered_vlist[j][level]=vlist[j][i];
	  }
  }

  evmdd_edgemaker<OP, float>
  EM(this, ordered_vlist, terms, order, N, getDomain()->getNumVariables(), unionOp);

  float ev;
  node_handle ep;
  EM.createEdge(ev, ep);
  e.set(ep, ev);

  free(ordered_vlist);
#ifdef DEVELOPMENT_CODE
  validateIncounts(true);
#endif
}

void MEDDLY::evmdd_plusreal
::createEdgeForVar(int vh, bool vp, const float* terms, dd_edge& a)
{
  createEdgeForVarTempl<OP, float>(vh, vp, terms, a);
#ifdef DEVELOPMENT_CODE
  validateIncounts(true);
#endif
}

void MEDDLY::evmdd_plusreal
::evaluate(const dd_edge &f, const int* vlist, float &term) const
{
  evaluateT<OP, float>(f, vlist, term);
}

bool MEDDLY::evmdd_plusreal
::isTransparentEdge(node_handle ep, const void* ev) const
{
  if (ep) return false;
  return OP::isTransparentEdge(ev);
}

void MEDDLY::evmdd_plusreal
::getTransparentEdge(node_handle &ep, void* ev) const
{
  OP::makeEmptyEdge(ep, ev);
}

bool MEDDLY::evmdd_plusreal
::areEdgeValuesEqual(const void* eva, const void* evb) const
{
  float val1, val2;
  OP::readValue(eva, val1);
  OP::readValue(evb, val2);
  return !OP::notClose(val1, val2);
}

bool MEDDLY::evmdd_plusreal::isRedundant(const unpacked_node &nb) const
{
  return isRedundantTempl<OP>(nb);
}

bool MEDDLY::evmdd_plusreal::isIdentityEdge(const unpacked_node &nb, int i) const
{
  return isIdentityEdgeTempl<OP>(nb, i); 
}


void MEDDLY::evmdd_plusreal::normalize(unpacked_node &nb, float& ev) const
{
    int minindex = -1;
    int stop = nb.isSparse() ? nb.getNNZs() : nb.getSize();
    bool allInf = true;
    //bool allMinusInf = true;
    for (int i=0; i<stop; i++) {
      if (0==nb.d(i)) continue;
      if ((minindex < 0) || (nb.ef(i) < nb.ef(minindex))) {
        if (nb.ef(i) != -std::numeric_limits<float>::infinity()) {
          minindex = i;
        }
      }
      if (nb.ef(i) != std::numeric_limits<float>::infinity()) {
          allInf = false;
      }
      //if (nb.ef(i) != -std::numeric_limits<float>::infinity()) {
      //    allMinusInf = false;
      //}
    }

    /*if (allMinusInf) {
      ev = -std::numeric_limits<float>::infinity();
      for (int i=0; i<stop; i++) {
        nb.setEdge(i, 0);
      }
      return;
    } else {*/
      ev = nb.ef(minindex);
    //}

    for (int i=0; i<stop; i++) {
      if (0==nb.d(i)) continue;
      float temp;
      nb.getEdge(i, temp);
      // DAVID CHANGED: ALLOW PARTIAL FUNCTIONS (correct link of infinity) (added if cond)
      if (temp != std::numeric_limits<float>::infinity() || allInf) {
          //std::cout << temp << std::endl;
          temp -= ev;
          nb.setEdge(i, temp);
          //MEDDLY::FILE_output out(stdout);
          //nb.show(out,true);
          //std::cout << std::endl;
      } else {
          //std::cout << "skipping" << std::endl;
          nb.d_ref(i) = -1;
      }
  }
}

void MEDDLY::evmdd_plusreal::showEdgeValue(output &s, const void* edge) const
{
  OP::show(s, edge);
}

void MEDDLY::evmdd_plusreal::writeEdgeValue(output &s, const void* edge) const
{
  OP::write(s, edge);
}

void MEDDLY::evmdd_plusreal::readEdgeValue(input &s, void* edge)
{
  OP::read(s, edge);
}

const char* MEDDLY::evmdd_plusreal::codeChars() const
{
  return "dd_evpr";
}

// ******************************************************************
// *                                                                *
// *                                                                *
// *           evmdd_timesreal::evprmdd_iterator  methods           *
// *                                                                *
// *                                                                *
// ******************************************************************

MEDDLY::evmdd_plusreal::evprmdd_iterator::evprmdd_iterator(const expert_forest *F)
: iterator(F)
{
  int N = F->getNumVariables();
  acc_evs = new double[N+1];
}

MEDDLY::evmdd_plusreal::evprmdd_iterator::~evprmdd_iterator()
{
  delete[] acc_evs;
}

void MEDDLY::evmdd_plusreal::evprmdd_iterator::getValue(float &tv) const
{
  MEDDLY_DCASSERT(acc_evs);
  tv = acc_evs[0];
}

bool MEDDLY::evmdd_plusreal::evprmdd_iterator::start(const dd_edge &e)
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

bool MEDDLY::evmdd_plusreal::evprmdd_iterator::next()
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
      acc_evs[k-1] = acc_evs[k] + ev;
      break;
    }
  }
  level_change = k;
  if (k>maxLevel) {
    return false;
  }

  return first(k-1, down);
}

bool MEDDLY::evmdd_plusreal::evprmdd_iterator::first(int k, node_handle down)
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
    if (kdn < k)  path[k].initRedundant(F, k, 0, down, false);
    else          path[k].initFromNode(F, down, false);
    nzp[k] = 0;
    index[k] = path[k].i(0);
    down = path[k].d(0);
    float ev;
    path[k].getEdge(0, ev);
    acc_evs[k-1] = acc_evs[k] + ev;
  }
  // save the terminal value
  index[0] = down;
  return true;
}

