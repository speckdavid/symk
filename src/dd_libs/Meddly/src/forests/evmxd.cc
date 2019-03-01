
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

#include "evmxd.h"

MEDDLY::evmxd_forest
::evmxd_forest(int dsl, domain* d, range_type t, edge_labeling ev, 
  const policies &p,int* level_reduction_rule) : ev_forest(dsl, d, true, t, ev, p,level_reduction_rule)
{
  // anything to construct?
}

void MEDDLY::evmxd_forest::reorderVariables(const int* level2var)
{
	throw error(error::NOT_IMPLEMENTED);
}

void MEDDLY::evmxd_forest::swapAdjacentVariables(int level)
{
	throw error(error::NOT_IMPLEMENTED);
}

void MEDDLY::evmxd_forest::moveDownVariable(int high, int low)
{
	throw error(error::NOT_IMPLEMENTED);
}

void MEDDLY::evmxd_forest::moveUpVariable(int low, int high)
{
	throw error(error::NOT_IMPLEMENTED);
}
