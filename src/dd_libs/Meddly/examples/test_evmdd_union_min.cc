
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



#if 0
#include <mdds.h>
#include <compute_table.h>
#include <evmdd_ops.h>
#endif

#include <domain.h>
#include <forest.h>
#include <dd_edge.h>
#include <ophandle.h>


int main(int argc, char *argv[])
{
  const int sz = 4;
  int bound[sz] = {2, 2, 2, 2};
  int height[sz] = {0, 1, 2, 3};
  forest_type f = EVMDD;
#if 0
  reduction_rule r = forest::QUASI_REDUCED;
#else
  reduction_rule r = forest::FULLY_REDUCED;
#endif
  node_storage s = FULL_OR_SPARSE_STORAGE;

  initialize();
  
  domain *d = CreateDomain(sz-1, bound, height);
  assert(NULL != d);
  forest_hndl evmdd = CreateForest(d, f, false, r, s);
  assert(INVALID_FOREST != evmdd);

  // Union by adding a vector element at a time
#if 1
  int val1[sz] = {0, 4, 1, 1};
  int dptr1[sz] = {0, 0, -1, -1};
  dd_tempedge* node1 = CreateTempEdge(evmdd, NULL);
  AddEVVectorElement(node1, dptr1, val1, sz, true);
  ShowTempEdge(stderr, node1);

  int val2[sz] = {0, 2, 2, 3};
  int dptr2[sz] = {0, -1, -1, 0};
  dd_edge* node2 = CreateEVVectorElement(evmdd, dptr2, val2, sz, true);
  ShowDDEdge(stderr, node2);
  AddEVVectorElement(node1, dptr2, val2, sz, true);
  ShowTempEdge(stderr, node1);

  fflush(stderr);
#endif

  // Union by adding two nodes
#if 1
  int val3[sz] = {0, 4, 1, 1};
  int dptr3[sz] = {0, 0, -1, -1};
  dd_edge* node3 = CreateEVVectorElement(evmdd, dptr3, val3, sz, true);
  ShowDDEdge(stderr, node3);

  int val4[sz] = {0, 2, 2, 3};
  int dptr4[sz] = {0, -1, -1, 0};
  dd_edge* node4 = CreateEVVectorElement(evmdd, dptr4, val4, sz, true);
  ShowDDEdge(stderr, node4);

  dd_edge* node5 = node3; //NULL;
  ApplyBinary(OP_UNION, node3, node4, node5);
  ShowDDEdge(stderr, node5);
  fflush(stderr);

  // int val2[sz] = {0, 1, 2, 3};
  // int dptr2[sz] = {0, 0, 0, 0};
#endif

#if 0
  evmdd_node node3;
  ops->computeUnionMin(node1, node2, node3);
  std::cerr << "Edge-Value: " << node3.value
    << ", Node: " << node3.node
    << std::endl;
#endif
  
#if 0
  evmdd_node node4;
  ops->computeUnionMin(node1, node2, node4);
  std::cerr << "Edge-Value: " << node4.value
    << ", Node: " << node4.node
    << std::endl;
#endif
 
#if 0
  ops->report(stderr);
  delete ops;

  evmdd_nm->showAll(stderr);
  evmdd_nm->reportMemoryUsage(stderr);
  delete evmdd_nm;
#endif

  ShowForestNodes(stderr, evmdd);
  DestroyForest(evmdd);
  DestroyDomain(d);
  cleanup();
  return 0;
}
