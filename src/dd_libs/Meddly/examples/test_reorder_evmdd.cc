
// $Id: test_evmdd.cc 653 2016-02-17 00:00:51Z cjiang1209 $

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
#include <algorithm>
#include <random>
#include <chrono>

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
//#define USE_RANDOM_GENERATOR_BOUND 0
//#define USE_SERIAL_TERMS 0
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

void printAssignment(unsigned count, const int* element, int value, int nVariables)
{
  printf("%d: [%d", count, element[1]);
  for (int k=2; k<=nVariables; k++) {
    printf(", %d", element[k]);
  }
  printf("] = %d\n", value);
}

void displayElements(const dd_edge& result) {
  // Use iterator to display elements
  unsigned counter = 0;
  int nVariables = result.getForest()->getDomain()->getNumVariables();
  for (enumerator iter(result); iter; ++iter, ++counter) {
    int value = 0;
    iter.getValue(value);
    printAssignment(counter, iter.getAssignments(), value, nVariables);
  }
  printf("Iterator traversal: %0.4e elements\n", double(counter));
  double c;
  apply(CARDINALITY, result, c);
  printf("Cardinality: %0.4e\n", c);
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

  start.note_time();
  dd_edge result(evmdd);
  evmdd->createEdge(element, terms, nElements, result);
  start.note_time();

  if (verbose > 0) result.show(meddlyout, 2);

  printf("Time interval: %.4e seconds\n",
      start.get_last_interval()/1000000.0);
  printf("#Nodes: %d\n", result.getNodeCount());
  printf("#Edges: %d\n", result.getEdgeCount());

  // print elements
  if (verbose > 0) {
    printElements(element, terms, nElements, nVariables);
  }

//  displayElements(result);

  expert_forest* eevmdd = static_cast<expert_forest*>(evmdd);
  eevmdd->removeAllComputeTableEntries();

  int* level2var_origin = new int[nVariables + 1];
  eevmdd->getVariableOrder(level2var_origin);

  // order[0] is unused
  int* level2var = new int[nVariables + 1];
  memcpy(level2var, level2var_origin, sizeof(int) * (nVariables + 1));

  for (int i = 0; i < 10; i++) {
    std::shuffle(&level2var[1], &level2var[nVariables+1], std::default_random_engine(seed + 1));

    std::cout << "Reorder" << std::endl;

    std::cout << "#Nodes (Before): " << result.getNodeCount() << std::endl;
    std::cout << "#Edges (Before): " << result.getEdgeCount() << std::endl;

    start.note_time();
    eevmdd->reorderVariables(level2var);
//    displayElements(result);
    std::cout << "#Nodes  (After): " << result.getNodeCount() << std::endl;
    std::cout << "#Edges  (After): " << result.getEdgeCount() << std::endl;

    start.note_time();
    std::cout << "Time interval: " << start.get_last_interval()/1000000.0 << " seconds" << std::endl;
  }

  std::cout << "Reorder" << std::endl;

  std::cout << "#Nodes (Before): " << result.getNodeCount() << std::endl;
  std::cout << "#Edges (Before): " << result.getEdgeCount() << std::endl;

  start.note_time();
  eevmdd->reorderVariables(level2var_origin);
//  displayElements(result);
  std::cout << "#Nodes  (After): " << result.getNodeCount() << std::endl;
  std::cout << "#Edges  (After): " << result.getEdgeCount() << std::endl;

  start.note_time();
  std::cout << "Time interval: " << start.get_last_interval()/1000000.0 << " seconds" << std::endl;

  delete[] level2var;
  delete[] level2var_origin;

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

