
// $Id: test_reorder_mtmxd.cc 653 2016-02-17 00:00:51Z cjiang1209 $

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



/**
 * test_reorder_mtmxd.cc
 *
 * Testing variable reordering in MTMXD for integers.
 */

#include <iostream>
#include <string.h>
#include <algorithm>
#include <random>
#include <chrono>

#include "meddly.h"
#include "meddly_expert.h"

using namespace MEDDLY;

// Timer class
#include "timer.h"

typedef int element_type;

void printUsage(FILE *outputStream)
{
  fprintf(outputStream,
      "Usage: test_union_mtmxd <#Variables> <VariableBound> <#Elements>\n");
}

void printAssignment(unsigned count, const int* element, const int* pelement, int nVariables)
{
  assert(element != 0 && pelement != 0);

  const int* curr = element + nVariables;
  const int* end = element - 1;
  printf("%d: [%d", count, *curr--);
  while (curr != end) { printf(" %d", *curr--); }

  curr = pelement + nVariables;
  end = pelement - 1;
  printf("] --> [%d", *curr--);
  while (curr != end) { printf(" %d", *curr--); }
  printf("]\n");
}

void printAssignment(unsigned count, const int* element, int nVariables)
{
  printf("%d: [%d", count, element[1]);
  for (int k=2; k<=nVariables; k++) {
    printf(", %d", element[k]);
  }
  printf("] --> [%d", element[-1]);
  for (int k=2; k<=nVariables; k++) {
    printf(", %d", element[-k]);
  }
  printf("]\n");
}

void displayElements(const dd_edge& result) {
  // Use iterator to display elements
  unsigned counter = 0;
  int nVariables = result.getForest()->getDomain()->getNumVariables();
  for (enumerator iter(result); iter; ++iter, ++counter) {
    printAssignment(counter, iter.getAssignments(), nVariables);
  }
  printf("Iterator traversal: %0.4e elements\n", double(counter));
  double c;
  apply(CARDINALITY, result, c);
  printf("Cardinality: %0.4e\n", c);
}

int main(int argc, char *argv[])
{
  if (argc != 4) {
    printUsage(stdout);
    exit(1);
  }

  unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
//  unsigned seed = 2783263681;
  std::cout << "Seed: " << seed << std::endl;

  srand(seed);

  // initialize number of variables, their bounds and the number of elements
  // to create

  int nVariables = 0;
  int variableBound = 0;
  int nElements = 0;

  sscanf(argv[1], "%d", &nVariables);
  assert(nVariables > 0);

  sscanf(argv[2], "%d", &variableBound);
  assert(variableBound > 0);

  sscanf(argv[3], "%d", &nElements);
  assert(nElements > 0);

  printf("#variables: %d, variable bound: %d, #elements: %d\n",
      nVariables, variableBound, nElements);

  // create the elements randomly
  int** from = new int*[nElements];
  int** to = new int*[nElements];
  element_type* terms = new element_type[nElements];

  for (int i = 0; i < nElements; ++i) {
    from[i] = new int[nVariables + 1];
    to[i] = new int[nVariables + 1];
    from[i][0] = 0;
    to[i][0] = 0;
    for (int j = nVariables; j > 0; --j) {
      from[i][j] = int(float(variableBound + 1) * rand() / (RAND_MAX + 1.0)) - 1;
      assert(from[i][j] >= DONT_CARE && from[i][j] < variableBound);
      // DONT_CHANGE can only be used with DONT_CARE
      to[i][j] = int(float(variableBound) * rand() / (RAND_MAX + 1.0));
      assert(to[i][j] >= 0 && to[i][j] < variableBound);
    }
    terms[i] =
      element_type(int(float(variableBound) * rand() / (RAND_MAX + 1.0)));
  }

  // initialize the variable bounds array to provide to the domain

  int* bounds = new int[nVariables];
  assert(bounds != 0);
  for (int i = 0; i < nVariables; ++i) {
    bounds[i] = variableBound;
  }

  initialize();

  // Create a domain
  domain *d = createDomainBottomUp(bounds, nVariables);
  assert(d != 0);

  // Create a MTMXD forest in this domain
  forest::policies p(true);
  p.setVarSwap();
//  p.setLevelSwap();
  p.setFullyReduced();

  forest* mtmxd =
    d->createForest(true, forest::INTEGER, forest::MULTI_TERMINAL, p);
  assert(mtmxd != 0);

  timer start;
  start.note_time();

  dd_edge result(mtmxd);
  mtmxd->createEdge(from, to, terms, nElements, result);

  start.note_time();
  std::cout << "Time interval: " << start.get_last_interval()/1000000.0 << " seconds" << std::endl;

  std::cout << "Peak Nodes in MXD: " << mtmxd->getPeakNumNodes() << std::endl;

  FILE_output myout(stdout);
  result.show(myout, 1);

  expert_forest* emtmxd = static_cast<expert_forest*>(mtmxd);
  emtmxd->removeAllComputeTableEntries();

  // oldOrder[0] is unused
  int* level2var_origin = new int[nVariables + 1];
  emtmxd->getVariableOrder(level2var_origin);

  // order[0] is unused
  int* level2var = new int[nVariables + 1];
  memcpy(level2var, level2var_origin, sizeof(int) * (nVariables + 1));

  for (int i = 0; i < 10; i++) {
    std::cout << "Reorder" << std::endl;

//    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::shuffle(&level2var[1], &level2var[nVariables+1], std::default_random_engine(seed + 1));

//    displayElements(result);
    std::cout << "#Nodes (Before): " << mtmxd->getCurrentNumNodes() << std::endl;

    start.note_time();
    emtmxd->reorderVariables(level2var);
    //displayElements(result);
    std::cout << "#Nodes  (After): " << mtmxd->getCurrentNumNodes() << std::endl;

    start.note_time();
    std::cout << "Time interval: " << start.get_last_interval()/1000000.0 << " seconds" << std::endl;
  }

  std::cout << "Reorder" << std::endl;

//  displayElements(result);
  std::cout << "#Nodes (Before): " << mtmxd->getCurrentNumNodes() << std::endl;

  start.note_time();
  emtmxd->reorderVariables(level2var_origin);
//  displayElements(result);
  std::cout << "#Nodes  (After): " << mtmxd->getCurrentNumNodes() << std::endl;

  start.note_time();
  std::cout << "Time interval: " << start.get_last_interval()/1000000.0 << " seconds" << std::endl;

  delete[] level2var;
  delete[] level2var_origin;

  // Cleanup; in this case simply delete the domain
  destroyDomain(d);
  cleanup();

  delete[] bounds;
  for (int i = 0; i < nElements; ++i)
  {
    delete[] from[i];
    delete[] to[i];
  }
  delete[] from;
  delete[] to;

  return 0;
}

