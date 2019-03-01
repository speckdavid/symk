
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


#include "mt.h"
#include "../unique_table.h"

#define ENABLE_CACHE_COUNTING 0
#define ENABLE_IN_COUNTING 0


// ******************************************************************
// *                                                                *
// *                       mt_forest  methods                       *
// *                                                                *
// ******************************************************************

int* MEDDLY::mt_forest::order;
int  MEDDLY::mt_forest::order_size;

MEDDLY::mt_forest::mt_forest(int dsl, domain *d, bool rel,
  range_type t, const policies &p,int* level_reduction_rule)
: expert_forest(dsl, d, rel, t, MULTI_TERMINAL, p, level_reduction_rule)
{
}

bool MEDDLY::mt_forest::isRedundant(const unpacked_node &nb) const
{
  if (isQuasiReduced()) return false;
  if (nb.getLevel() < 0 && isIdentityReduced()) return false;
  int rawsize = nb.isSparse() ? nb.getNNZs() : nb.getSize();
  if (rawsize < getLevelSize(nb.getLevel())) return false;
  int common = nb.d(0);
  for (int i=1; i<rawsize; i++) 
    if (nb.d(i) != common) return false;
  return true;
}

bool MEDDLY::mt_forest::isIdentityEdge(const unpacked_node &nb, int i) const
{
  if (nb.getLevel() > 0) return false;
  if (!isIdentityReduced()) return false;
  if (i<0) return false;
  return nb.d(i) != 0;
}


MEDDLY::node_handle MEDDLY::mt_forest::makeNodeAtLevel(int k, node_handle d) 
{
  MEDDLY_DCASSERT(abs(k) >= abs(getNodeLevel(d)));

  if (isFullyReduced()) return d; 

  if (isQuasiReduced() && d==getTransparentNode()) return d;

  int dk = getNodeLevel(d); 
  while (dk != k) {
    int up;
    if (dk<0) up = -dk;
    else up = isForRelations() ? -(dk+1) : dk+1;

    // make node at level "up"
    int sz = getLevelSize(up);
    unpacked_node* nb = unpacked_node::newFull(this, up, sz);

    if (isIdentityReduced() && (dk<0)) {
      // make identity reductions below as necessary
      node_handle sd;
      int si = isTerminalNode(d) ? -1 : getSingletonIndex(d, sd);
      for (int i=0; i<sz; i++) {
        nb->d_ref(i) = linkNode( (i==si) ? sd : d );
      }
    } else {
      // don't worry about identity reductions
      for (int i=0; i<sz; i++) {
        nb->d_ref(i) = linkNode(d);
      }
    }
    unlinkNode(d);
    d = createReducedNode(-1, nb);
    dk = up;
  } // while
  return d;
}

void MEDDLY::mt_forest::initStatics()
{
  order = 0;
  order_size = 0;
}

void MEDDLY::mt_forest::enlargeStatics(int n)
{
  MEDDLY_DCASSERT(n>0);
  if (n>order_size) {
    order = (int*) realloc(order, n*sizeof(int));
    //terminals = (node_handle*) realloc(terminals, n*sizeof(node_handle));
    //if (0==order || 0==terminals) {
    if (0==order) {
      throw error(error::INSUFFICIENT_MEMORY);
    }
    order_size = n;
  }
  for (int i=0; i<n; i++) order[i] = i;
}

void MEDDLY::mt_forest::clearStatics()
{
  free(order);
  order = 0;
  order_size = 0;
}

// ******************************************************************
// *                                                                *
// *                 mt_forest::mt_iterator methods                 *
// *                                                                *
// ******************************************************************

MEDDLY::mt_forest::mt_iterator::mt_iterator(const expert_forest *F)
 : iterator(F)
{
}

MEDDLY::mt_forest::mt_iterator::~mt_iterator()
{
}

void MEDDLY::mt_forest::mt_iterator::getValue(int &termVal) const
{
  MEDDLY_DCASSERT(index);
  termVal = int_Tencoder::handle2value(index[0]);
}

void MEDDLY::mt_forest::mt_iterator::getValue(float &termVal) const
{
  MEDDLY_DCASSERT(index);
  termVal = float_Tencoder::handle2value(index[0]);
}

