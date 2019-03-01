
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


#include "mtmddint.h"

MEDDLY::mt_mdd_int::mt_mdd_int(int dsl, domain *d, const policies &p,int* level_reduction_rule, int tv)
: mtmdd_forest(dsl, d, INTEGER, p,level_reduction_rule)
{ 
  initializeForest();

  transparent=int_Tencoder::value2handle(tv);
}

MEDDLY::mt_mdd_int::~mt_mdd_int()
{ }

void MEDDLY::mt_mdd_int::createEdge(int term, dd_edge& e)
{
  createEdgeTempl<int_Tencoder, int>(term, e);
#ifdef DEVELOPMENT_CODE
  validateIncounts(true);
#endif
}

void MEDDLY::mt_mdd_int::createEdge(const int* const* vlist, const int* terms, int N, dd_edge &e)
{
  binary_operation* unionOp = getOperation(PLUS, this, this, this);
  enlargeStatics(N);
  enlargeVariables(vlist, N, false);

  int num_vars=getNumVariables();

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

  mtmdd_edgemaker<int_Tencoder, int>
  EM(this, ordered_vlist, terms, order, N, num_vars, unionOp);

  e.set(EM.createEdge());

  free(ordered_vlist);
  
#ifdef DEVELOPMENT_CODE
  validateIncounts(true);
#endif
}

void MEDDLY::mt_mdd_int::
createEdgeForVar(int vh, bool vp, const int* terms, dd_edge& a)
{
  createEdgeForVarTempl<int_Tencoder, int>(vh, vp, terms, a);
#ifdef DEVELOPMENT_CODE
  validateIncounts(true);
#endif
}

void MEDDLY::mt_mdd_int
::evaluate(const dd_edge &f, const int* vlist, int &term) const
{
  term = int_Tencoder::handle2value(evaluateRaw(f, vlist));
}

void MEDDLY::mt_mdd_int::showTerminal(output &s, node_handle tnode) const
{
  int_Tencoder::show(s, tnode);
}

void MEDDLY::mt_mdd_int::writeTerminal(output &s, node_handle tnode) const
{
  int_Tencoder::write(s, tnode);
}

MEDDLY::node_handle MEDDLY::mt_mdd_int::readTerminal(input &s)
{
  return int_Tencoder::read(s);
}

const char* MEDDLY::mt_mdd_int::codeChars() const
{
  return "dd_tvi";
}

