
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



/**
 * test_union_mdd.cc
 *
 * Testing MDD union.
 */

#include <cstdlib>
#include <iostream>
#include "meddly.h"
#include "timer.h"

using namespace MEDDLY;

//#define TESTING_AUTO_VAR_GROWTH

//#define TESTING_TEMP_DD_EDGES
//#define TESTING_UNION_SPEED
//#define BUILD_INDEX_SET
//#define CHECK_ELEMENTS

//#define VERBOSE

#define CACHE_SIZE 262144u

FILE_output meddlyout(stdout);

void testIndexSet(const dd_edge& mdd, dd_edge& indexSet)
{
  apply(CONVERT_TO_INDEX_SET, mdd, indexSet);

#if 1
  indexSet.show(meddlyout, 3);
#else
  indexSet.show(meddlyout, 1);
#endif
}



void printUsage(FILE *outputStream)
{
  fprintf(outputStream,
      "Usage: test_union_mdd <#Variables> <VariableBound> <#Elements>\n");
}

int main(int argc, char *argv[])
{
  if (argc != 4) {
    printUsage(stdout);
    exit(1);
  }

  srand(1u);

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

  timer mallocTimer;
  long mallocTime = 0;

  int** elements = (int **) malloc(nElements * sizeof(int *));
  for (int i = 0; i < nElements; ++i)
  {
    mallocTimer.note_time();
    elements[i] = (int *) malloc((nVariables + 1) * sizeof(int));
    mallocTimer.note_time();
    mallocTime += mallocTimer.get_last_interval();

    elements[i][0] = 0;
    for (int j = nVariables; j >= 1; --j)
    {
      elements[i][j] = int(float(variableBound) * rand() / (RAND_MAX + 1.0));
      assert(elements[i][j] >= 0 && elements[i][j] < variableBound);
    }
    // print element[i]
#ifdef VERBOSE
    printf("Element %d: [%d", i, elements[i][0]);
    for (int j = 1; j <= nVariables; ++j)
    {
      printf(" %d", elements[i][j]);
    }
    printf("]\n");
#endif
  }

  // initialize the variable bounds array to provide to the domain

  int* bounds = (int *) malloc(nVariables * sizeof(int));
  assert(bounds != 0);
  for (int i = 0; i < nVariables; ++i)
  {
#ifdef TESTING_AUTO_VAR_GROWTH
    bounds[i] = 2;
#else
    bounds[i] = variableBound;
#endif
  }


  initializer_list* L = defaultInitializerList(0);
#ifdef CACHE_SIZE
  ct_initializer::setMaxSize(CACHE_SIZE);
#endif
  initialize(L);

  // Create a domain
  domain *d = createDomainBottomUp(bounds, nVariables);
  assert(d != 0);

  // Create an MDD forest in this domain (to store states)
  forest* states = d->createForest(false, forest::BOOLEAN,
      forest::MULTI_TERMINAL);
  assert(states != 0);

#if 0
  assert(forest::SUCCESS ==
      //states->setReductionRule(forest::FULLY_REDUCED));
    states->setReductionRule(forest::QUASI_REDUCED));
  assert(forest::SUCCESS ==
      states->setNodeDeletion(forest::OPTIMISTIC_DELETION));
  // states->setNodeDeletion(forest::PESSIMISTIC_DELETION));
  if (variableBound < 4) {
    assert(forest::SUCCESS ==
        states->setNodeStorage(forest::FULL_STORAGE));
  } else {
    assert(forest::SUCCESS ==
        states->setNodeStorage(forest::FULL_OR_SPARSE_STORAGE));
  }
#endif

  dd_edge initial_state(states);

  timer start;

  printf("Started... ");

#ifdef TESTING_TEMP_DD_EDGES
  printf("Hello World!\n");
  temp_dd_edge* temp = new temp_dd_edge;
  temp->forestHandle = static_cast<expert_forest*>(states);
  temp->levelHandle = nVariables;
  for (int i = 0; i < nElements; ++i)
  {
    temp->add(elements[i], 0);
  }
  assert(temp->convertToDDEdge(initial_state));
  delete temp;
#else
#ifdef TESTING_UNION_SPEED
  // Create a dd_edge per element and combine using the UNION operator.
  dd_edge** ddElements = new dd_edge*[nElements];
  for (int i = 0; i < nElements; ++i)
  {
    ddElements[i] = new dd_edge(states);
    states->createEdge(elements + i, 1, *(ddElements[i]));
  }
  // Combine dd_edges
  int nDDElements = nElements;
  while (nDDElements > 1) {
    int nCombinations = nDDElements/2;
    for (int i = 0; i < nCombinations; i++)
    {
      // Combine i and (nDDElements-1-i)
      (*(ddElements[i])) += (*(ddElements[nDDElements-1-i]));
      delete ddElements[nDDElements-1-i];
    }
    nDDElements = (nDDElements+1)/2;
  }
  initial_state = *(ddElements[0]);
  delete ddElements[0];
  delete [] ddElements;

#else
  // Use Meddly's batch addition to combine all elements in one step.
  states->createEdge(elements, nElements, initial_state);

#endif
#endif

  start.note_time();
  printf("done. Time interval: %.4e seconds\n",
      start.get_last_interval()/1000000.0);

#ifdef VERBOSE
  printf("\n\nInitial State:\n");
  initial_state.show(meddlyout, 2);
#endif

  double c;
  apply(CARDINALITY, initial_state, c);
  printf("Elements in result: %.4e\n", c);
  printf("Peak Nodes in MDD: %ld\n", states->getPeakNumNodes());
  /* TBD: FIX
  printf("Nodes in compute table: %ld\n",
      (getComputeManager())->getNumCacheEntries());
  */

#ifdef BUILD_INDEX_SET
  // TEST
  forest* evmdd = d->createForest(false, forest::INTEGER,
      forest::INDEX_SET);
  assert(evmdd != 0);
  dd_edge evmdd_states(evmdd);
  apply(CONVERT_TO_INDEX_SET, initial_state, evmdd_states);
  evmdd_states.show(meddlyout, 3);

  enumerator iter(evmdd_states);
  int currentIndex = 0;
  while (iter) {
    const int* elem = iter.getAssignments();
    printf("%d: [", currentIndex++);
    for (int i = 1; i < nVariables; i++) {
      printf("%d ", elem[i]);
    }
    int elemTerm = 0;
#if 0
    evmdd->evaluate(evmdd_states, elem, elemTerm);
#else
    iter.getValue(elemTerm);
#endif
    printf("%d] = %d\n", elem[nVariables], elemTerm);
    ++iter;
  }
#endif

#ifdef VERBOSE
  printf("\n\nForest Info:\n");
  states->showInfo(meddlyout);
#endif

#ifdef CHECK_ELEMENTS
  // make sure all the elements are in there
  for (int i = 0; i < nElements; ++i)
  {
    bool result = false;
    states->evaluate(initial_state, elements[i], result);
    assert(result == true);
  }
#endif

  // Cleanup; in this case simply delete the domain
  destroyDomain(d);
  cleanup();

  free(bounds);
  for (int i = 0; i < nElements; ++i)
  {
    mallocTimer.note_time();
    free(elements[i]);
    mallocTimer.note_time();
    mallocTime += mallocTimer.get_last_interval();
  }
  free(elements);

#ifdef VERBOSE
  printf("Malloc time: %.4e seconds\n", mallocTime/1000000.0);
#endif

  return 0;
}
