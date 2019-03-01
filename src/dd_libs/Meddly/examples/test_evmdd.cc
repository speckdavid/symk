
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
 * test_evmdd.cc
 *
 * Testing EV+MDD operations for integers.
 * Operations: min, max, +, -, *, /.
 */

#include <iostream>
#include "meddly.h"

// #define USE_EXPERT_INTERFACE
#ifdef USE_EXPERT_INTERFACE
#include "meddly_expert.h"
#endif

using namespace MEDDLY;

// Timer class
#include "timer.h"

#define TEST_INDEX_SET
#define USE_SEQUENTIAL_PLUS 0
#define USE_RANDOM_GENERATOR_BOUND 0
#define USE_SERIAL_TERMS 0
#define USE_REALS 0

#if USE_REALS
  typedef float element_type;
#else
  typedef int element_type;
#endif


// verbose: 0: minimum, 2: maximum
const int verbose = 0;

FILE_output meddlyout(stdout);

#ifdef USE_EXPERT_INTERFACE

// Given a forest and an op_code returns the corresponding op_info.
// 
// This is only valid for operations of the form C = A op B,
// where A, B and C belong to the same forest.
op_info* getOp(forest* f, compute_manager::op_code op)
{
  static const int nForests = 3;
  static forest* forests[nForests];
  static expert_compute_manager* ecm = 
    static_cast<expert_compute_manager*>(getComputeManager());
  assert(ecm != 0);
  assert(f != 0);

  forests[0] = f;
  forests[1] = f;
  forests[2] = f;
  return ecm->getOpInfo(op, forests, nForests);
}


// Given a forest and an instance of an operation returns
// the corresponding op_info.
// 
// This is only valid for operations of the form C = A op B,
// where A, B and C belong to the same forest.
op_info* getOp(forest* f, operation* op)
{
  static const int nForests = 3;
  static forest* forests[nForests];
  static expert_compute_manager* ecm = 
    static_cast<expert_compute_manager*>(getComputeManager());
  assert(ecm != 0);
  assert(f != 0);
  assert(op != 0);

  forests[0] = f;
  forests[1] = f;
  forests[2] = f;
  return ecm->getOpInfo(op, forests, nForests);
}

#endif

// Tests a evmdd operation on the elements provided.
// This function assumes that each element[i] represents
// an element in the given MTMDD.
dd_edge test_evmdd(forest* evmdd, const binary_opname* opCode,
    int** element, element_type* terms, int nElements)
{
  // A = first nElements/2 elements combined using +.
  // B = second nElements/2 elements combined using +.
  // C = A op B

  dd_edge A(evmdd);
  dd_edge B(evmdd);
  dd_edge C(evmdd);

  int half = nElements/2;

  evmdd->createEdge(element, terms, half, A);
  evmdd->createEdge(element + half, terms + half, nElements - half, B);

#ifdef USE_EXPERT_INTERFACE
  binary_operation* op = getOperation(opCode, A, B, C);
  assert(op != NULL);
  op->compute(op, A, B, C);
#else
  apply(opCode, A, B, C);
#endif

  if (verbose > 0) {
    printf("A: ");
    A.show(meddlyout, 2);
    printf("\n\nB: ");
    B.show(meddlyout, 2);
    printf("\n\nC: ");
    C.show(meddlyout, 2);
    printf("\n\n");
  }

  return C;
}


dd_edge test_evmdd_plus(forest* evmdd,
    int** element, element_type* terms, int nElements)
{
  // Adds all elements sequentially

  dd_edge A(evmdd);
  dd_edge B(evmdd);

  for (int i = 0; i < nElements; i++)
  {
    if (verbose > 0) printf("element %d...", i);
    evmdd->createEdge(&element[i], &terms[i], 1, A);
    B += A;
    if (verbose > 0) {
      printf(" done.\n");
      A.show(meddlyout, 2);
    }
  }

  return B;
}


void printElements(int** elements, element_type* terms, int nElements,
    int nVariables)
{
  // print elements
  for (int i = 0; i < nElements; ++i)
  {
    printf("Element %d: [%d", i, elements[i][0]);
    for (int j = 1; j <= nVariables; ++j)
    {
      printf(" %d", elements[i][j]);
    }
#if USE_REALS
    printf(": %f]\n", terms[i]);
#else
    printf(": %d]\n", terms[i]);
#endif
  }
}

void printUsage(FILE *outputStream)
{
  fprintf(outputStream,
      "Usage: test_evmdd <#Variables> <VariableBound> <#Elements>\n");
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

#ifdef USE_RANDOM_GENERATOR_BOUND
  int randomGeneratorBound = 0;
  printf("Enter random generator bound (<= variable bound): ");
  scanf("%d", &randomGeneratorBound);
  assert(randomGeneratorBound <= variableBound);
#endif

  // create the elements randomly
  int** element = (int **) malloc(nElements * sizeof(int *));
  element_type* terms =
    (element_type *) malloc(nElements * sizeof(element_type));

  for (int i = 0; i < nElements; ++i)
  {
    element[i] = (int *) malloc((nVariables + 1) * sizeof(int));
    element[i][0] = 0;
    for (int j = nVariables; j > 0; --j)
    {
#ifdef USE_RANDOM_GENERATOR_BOUND
      element[i][j] =
        int(float(randomGeneratorBound) * rand() / (RAND_MAX + 1.0));
      assert(element[i][j] >= 0 && element[i][j] < randomGeneratorBound);
#else
      element[i][j] = int(float(variableBound) * rand() / (RAND_MAX + 1.0));
      assert(element[i][j] >= 0 && element[i][j] < variableBound);
#endif
    }
  }

  for (int i = 0; i < nElements; ++i)
  {
#ifdef USE_SERIAL_TERMS
    // terms[i] = i + 1;
    terms[i] = i;
#else
#ifdef USE_RANDOM_GENERATOR_BOUND
    terms[i] =
      element_type(float(randomGeneratorBound) * rand() / (RAND_MAX + 1.0));
#else
    terms[i] =
      element_type(float(variableBound) * rand() / (RAND_MAX + 1.0));
#endif
#endif
  }

  // initialize the variable bounds array to provide to the domain

  int* bounds = (int *) malloc(nVariables * sizeof(int));
  assert(bounds != 0);
  for (int i = 0; i < nVariables; ++i)
  {
    bounds[i] = variableBound;
  }

  initialize();

  // Create a domain
  domain *d = createDomainBottomUp(bounds, nVariables);
  assert(d != 0);

  // Create a MTMDD forest in this domain
  forest::policies p1(false);
  p1.setPessimistic();
#if USE_REALS
  forest* evmdd = d->createForest(false, forest::REAL, forest::EVTIMES, p1);
#else
  forest* evmdd = d->createForest(false, forest::INTEGER, forest::EVPLUS, p1);
#endif
  assert(evmdd != 0);

  // print elements
  if (verbose > 0) {
    printElements(element, terms, nElements, nVariables);
  }

  timer start;
  start.note_time();

#if 0
#ifndef USE_SEQUENTIAL_PLUS
  dd_edge result = test_evmdd(evmdd, compute_manager::PLUS,
      element, terms, nElements);
#else
  dd_edge result = test_evmdd_plus(evmdd, element, terms, nElements);
#endif
#else
  start.note_time();
  dd_edge result(evmdd);
  evmdd->createEdge(element, terms, nElements, result);
  start.note_time();
  printf("Batch Addition:\n");
  if (verbose > 0) result.show(meddlyout, 2);

  printf("Time interval: %.4e seconds\n",
      start.get_last_interval()/1000000.0);
  printf("#Nodes: %d\n", result.getNodeCount());
  printf("#Edges: %d\n", result.getEdgeCount());

  // print elements
  if (verbose > 0) {
    printElements(element, terms, nElements, nVariables);
  }

  start.note_time();
  dd_edge result1 = test_evmdd_plus(evmdd, element, terms, nElements);
  start.note_time();
  printf("Sequential Addition:\n");
  if (verbose > 0) result1.show(meddlyout, 2);

  printf("Time interval: %.4e seconds\n",
      start.get_last_interval()/1000000.0);

  if (result == result1) {
    printf("Batch Addition == Sequential Addition!\n");
  } else {
    printf("Batch Addition != Sequential Addition!\n");
#if 0
    dd_edge result2 = result1;
    assert(compute_manager::SUCCESS ==
        getComputeManager()->apply(compute_manager::EQUAL,
          result, result1, result2));
    result2.show(meddlyout, 2);
#endif
  }
#endif

  start.note_time();
  printf("Time interval: %.4e seconds\n",
      start.get_last_interval()/1000000.0);

  printf("Peak Nodes in MDD: %ld\n", evmdd->getPeakNumNodes());
  /* TBD: FIX
  printf("Entries in compute table: %ld\n",
      (getComputeManager())->getNumCacheEntries());
  */

  if (verbose > 1) {
    printf("\n\nForest Info:\n");
    evmdd->showInfo(meddlyout);
  }

#if 0
  printf("EVMDD: ");
  result.show(meddlyout, 2);
#endif

  if (true) {
    dd_edge reachableStates(result);
    start.note_time();
    unsigned counter = 0;
    for (enumerator iter(reachableStates);
        iter; ++iter, ++counter)
    {
      const int* element = iter.getAssignments();
      const int* curr = element + nVariables;
      const int* end = element - 1;
      printf("%d: [%d", counter, *curr--);
      while (curr != end) { printf(" %d", *curr--); }
      printf("]\n");
    }
    start.note_time();
    printf("Iterator traversal time (%0.4e elements): %0.4e seconds\n",
        double(counter), start.get_last_interval()/double(1000000.0));
    printf("Cardinality: %0.4e\n", reachableStates.getCardinality());
  }


  // Build equivalent MDD
  destroyForest(evmdd);

#ifdef TEST_INDEX_SET

  forest::policies p2(false);
  p2.setPessimistic();
  forest* mdd 
    = d->createForest(false, forest::BOOLEAN, forest::MULTI_TERMINAL, p2);
  assert(mdd != 0);

  start.note_time();
  printf("Building equivalent MDD...\n");
  dd_edge mddResult(mdd);
  mdd->createEdge(element, nElements, mddResult);
  start.note_time();
  printf("Time interval: %.4e seconds\n",
      start.get_last_interval()/1000000.0);

  printf("\nMDD:\n");

#if 1
  // print the elements
  {
    enumerator iter(mddResult);
    while (iter) {
      const int* elem = iter.getAssignments();
      int val = 0;
      iter.getValue(val);
      printf("[");
      for (int i = 1; i < nVariables; i++) {
        printf("%d ", elem[i]);
      }
      printf("%d]: %d\n", elem[nVariables], val);
      ++iter;
    }
  }
#endif

  printf("MDD Cardinality: %1.6e\n", mddResult.getCardinality());
  printf("Peak Nodes in MDD: %ld\n", mdd->getPeakNumNodes());
  /* TBD: FIX
  printf("Entries in compute table: %ld\n",
      (getComputeManager())->getNumCacheEntries());
  */

  // Create a EV+MDD forest in this domain (to store index set)
  forest* evplusmdd = d->createForest(false, forest::INTEGER, forest::INDEX_SET);
  assert(evplusmdd != NULL);

  // Convert MDD to Index Set EV+MDD and print the states
  dd_edge indexSet(evplusmdd);
  apply(CONVERT_TO_INDEX_SET, mddResult, indexSet);

  printf("\nIndex Set (EV+MDD):\n");

#if 1
  // print the elements
  {
    enumerator iter(indexSet);
    while (iter) {
      const int* elem = iter.getAssignments();
      int val = 0;
      iter.getValue(val);
      printf("[");
      for (int i = 1; i < nVariables; i++) {
        printf("%d ", elem[i]);
      }
      printf("%d]: %d\n", elem[nVariables], val);
      ++iter;
    }
  }
#endif

  printf("Index Set Cardinality: %1.6e\n", indexSet.getCardinality());
  printf("Peak Nodes in Index Set: %ld\n", evplusmdd->getPeakNumNodes());
  /* TBD: FIX
  printf("Entries in compute table: %ld\n",
      (getComputeManager())->getNumCacheEntries());
  */

#endif

  printf("\n");
  // Cleanup; in this case simply delete the domain
  destroyDomain(d);
  cleanup();

  free(bounds);
  for (int i = 0; i < nElements; ++i)
  {
    free(element[i]);
  }
  free(element);

  return 0;
}

