
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
 * test_mtmxd.cc
 *
 * Testing MTMXD operations for integers and reals.
 * Operations: min, max, +, -, *, /.
 */

#include <iostream>
#include <string.h>
#include "meddly.h"
#include "meddly_expert.h"

using namespace MEDDLY;

// Only include this file if referring to operations that are not
// available via the mddlib interface. 
// 
// Also, include this file if you would like to create a custom
// operation by deriving one of the existing operations.
// #include "operation_ext.h"

// Timer class
#include "timer.h"



// Use MTMXD with real values for terminals.
#define USE_REALS 0

// Define element_type based on USE_REALS.
#if USE_REALS
  typedef float element_type;
#else
  typedef int element_type;
#endif


// verbose: 0: minimum, 2: maximum
const int verbose = 1;

#if 0
// Given a forest and an op_code returns the corresponding op_info.
// 
// This is only valid for operations of the form C = A op B,
// where A, B and C belong to the same forest.
op_info* getOp(forest* f, compute_manager::op_code op)
{
  static const int nForests = 3;
  static op_param plist[nForests];
  static expert_compute_manager* ecm = 
    static_cast<expert_compute_manager*>(getComputeManager());
  assert(ecm != 0);
  assert(f != 0);

  plist[0].set(f);
  plist[1].set(f);
  plist[2].set(f);
  return ecm->getOpInfo(op, plist, nForests);
}


// Given a forest and an instance of an operation returns
// the corresponding op_info.
// 
// This is only valid for operations of the form C = A op B,
// where A, B and C belong to the same forest.
op_info* getOp(forest* f, old_operation* op)
{
  static const int nForests = 3;
  static op_param plist[nForests];
  static expert_compute_manager* ecm = 
    static_cast<expert_compute_manager*>(getComputeManager());
  assert(ecm != 0);
  assert(f != 0);
  assert(op != 0);

  plist[0].set(f);
  plist[1].set(f);
  plist[2].set(f);
  return ecm->getOpInfo(op, plist, nForests);
}
#endif

// Combine N elements using + operator
dd_edge doPlus(forest* mtmxd, int** from, int** to, element_type* terms, int N)
{
  // Four ways of doing this:
  // ------------------------
  //
  // Method (1):
  // (a) Use createEdge(from, to, terms, N, result)
  //
  // Method (2):
  // (a) createEdge(from + i, to + i, terms + i, 1, temp)
  // (b) result += temp
  // (c) Repeat (a), (b) for all i = 0 to N-1
  //
  // Method (3):
  // (a) createEdge(from + i, to + i, terms + i, 1, temp)
  // (b) cm->apply(PLUS, result, temp, result)
  // (c) Repeat (a), (b) for all i = 0 to N-1
  //
  // Method (4): follows after discussion.
  //
  // Discussion:
  // -----------
  //
  // Method (1) works for Union (for boolean terminals) and Plus
  // (with integers or reals as terminals).
  //
  // Method (2) works for overloaded operators of dd_edge.
  //
  // Method (3) is when overloaded operators are not availabe. Step (b)
  // actually consists of two steps:
  // (i)  Fetch op_info* corresponding to the operation and forests involved.
  // (ii) Perform the operation using op_info*.
  //
  // Fetching op_info* is not expensive but its does have a non-trivial cost
  // to it. Therefore, when the same operation is to be performed on the same
  // forest(s) repeatedly, it is better to fetch the op_info* once and then
  // use it repeatedly without having to fetch it again.
  //
  //
  // Suggestion:
  // -----------
  //
  // Method (4):
  // (a) Fetch operation PLUS that fits the forests involved.
  // (b) createEdge(from + i, to + i, terms + i, 1, temp)
  // (c) cm->apply(plusOp, result, temp, result)
  // (d) Repeat (b) and (c) for all i = 0 to N-1
  //


  // Method (4) is illustrated here:

  // Step (a): Use getOp() defined above to fetch the operation corresponding
  // to the the forests involved.
  binary_operation* plusOp = getOperation(PLUS, 
    (expert_forest*) mtmxd, (expert_forest*) mtmxd, (expert_forest*) mtmxd
  );

  dd_edge result(mtmxd);
  dd_edge temp(mtmxd);
  for (int i = 0; i < N; ++i)
  {
    // Step (b): Build edge for from[i], to[i], terms[i]
    mtmxd->createEdge(from + i, to + i, terms + i, 1, temp);

    // Step (c): Add it to result
    plusOp->compute(result, temp, result);
  }

  return result;
}


// Tests a mtmxd operation on the elements provided.
// This function assumes that each from[i] and to[i] combine
// to make up an element in the given MTMXD.
dd_edge test_mtmxd(forest* mtmxd, binary_opname* opCode,
    int** from, int** to, element_type* terms, int nElements)
{
  // A = first nElements/2 elements combined using +.
  // B = second nElements/2 elements combined using +.
  // C = A op B

  dd_edge A(mtmxd);
  dd_edge B(mtmxd);
  dd_edge C(mtmxd);

  int half = nElements/2;

  A = doPlus(mtmxd, from, to, terms, half);
  B = doPlus(mtmxd, from + half, to + half, terms + half, nElements - half);

  apply(opCode, A, B, C);

  if (verbose > 0) {
    FILE_output meddlyout(stdout);
    printf("A: ");
    A.show(meddlyout, 2);
    printf("\n\nB: ");
    B.show(meddlyout, 2);
    printf("\n\nC: ");
    C.show(meddlyout, 2);
  }

  return C;
}


bool test_conversion(dd_edge& A, dd_edge& B)
{
  try {
    apply(COPY, A, B);
    return true;
  }
  catch (MEDDLY::error e) {
    return false;
  }
}


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
  int** from = (int **) malloc(nElements * sizeof(int *));
  int** to = (int **) malloc(nElements * sizeof(int *));
  element_type* terms =
    (element_type *) malloc(nElements * sizeof(element_type));

  for (int i = 0; i < nElements; ++i)
  {
    from[i] = (int *) malloc((nVariables + 1) * sizeof(int));
    to[i] = (int *) malloc((nVariables + 1) * sizeof(int));
    from[i][0] = 0;
    to[i][0] = 0;
    for (int j = nVariables; j > 0; --j)
    {
      from[i][j] = int(float(variableBound) * rand() / (RAND_MAX + 1.0));
      assert(from[i][j] >= 0 && from[i][j] < variableBound);
      to[i][j] = int(float(variableBound) * rand() / (RAND_MAX + 1.0));
      assert(to[i][j] >= 0 && to[i][j] < variableBound);
    }
    terms[i] =
      element_type(int(float(variableBound) * rand() / (RAND_MAX + 1.0)));
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

  // Create a MTMXD forest in this domain
  forest::policies p(true);
#if USE_REALS
  forest* mtmxd =
    d->createForest(true, forest::REAL, forest::MULTI_TERMINAL, p);
#else
  forest* mtmxd =
    d->createForest(true, forest::INTEGER, forest::MULTI_TERMINAL, p);
#endif
  assert(mtmxd != 0);

  // print elements
#if 0
  for (int i = 0; i < nElements; ++i)
  {
    printf("Element %d: [%d", i, from[i][0]);
    for (int j = 1; j <= nVariables; ++j)
    {
      printf(" %d", from[i][j]);
    }
    printf(" ->");
    for (int j = 0; j <= nVariables; ++j)
    {
      printf(" %d", to[i][j]);
    }
#if USE_REALS
    printf(": %f]\n", terms[i]);
#else
    printf(": %d]\n", terms[i]);
#endif
  }
#endif

  timer start;
  start.note_time();
#if 0
  dd_edge result = test_mtmxd(mtmxd, compute_manager::EQUAL,
      from, to, terms, nElements);
#else
  dd_edge result(mtmxd);
  mtmxd->createEdge(from, to, terms, nElements, result);
#endif
  start.note_time();
  printf("Time interval: %.4e seconds\n",
      start.get_last_interval()/1000000.0);

  printf("Peak Nodes in MXD: %ld\n", mtmxd->getPeakNumNodes());
  /* TBD: FIX
  printf("Nodes in compute table: %ld\n",
      (getComputeManager())->getNumCacheEntries());
  */

  FILE_output myout(stdout);
  result.show(myout, 2);

  // Use iterator to display elements
  if (true) {
    unsigned counter = 0;
    for (enumerator iter(result);
        iter; ++iter, ++counter)
    {
      printAssignment(counter, iter.getAssignments(), nVariables);
    }
    printf("Iterator traversal: %0.4e elements\n", double(counter));
    double c;
    apply(CARDINALITY, result, c);
    printf("Cardinality: %0.4e\n", c);
  }

  // Test Row and Column Iterators
#if 1
  if (true) {
    enumerator beginIter(result);
    const int* element = beginIter.getAssignments();
    const int* curr = 0;
    const int* end = 0;

    while (beginIter) {
      // Print minterm
      curr = element + nVariables;
      end = element + 1;
      printf("[%d", element[1]);
      for (int i=2; i<=nVariables; i++) printf(" %d", element[i]);
      printf("]\n");

#if 0
      element = beginIter.getPrimedAssignments();
      curr = element + nVariables;
      end = element - 1;
      printf("[%d", *curr--);
      while (curr != end) { printf(" %d", *curr--); }
      printf("]\n");
#endif

      // Print columns
      enumerator colIter;
      colIter.startFixedRow(result, element);
      while (colIter) {
        element = colIter.getAssignments();
        printf(" --> [%d", element[-1]);
        for (int i=2; i<=nVariables; i++) printf(" %d", element[-i]);
        printf("]\n");
        ++colIter;
      }

      // Print column
      element = beginIter.getPrimedAssignments();
      printf("[%d", element[1]);
      for (int i=2; i<=nVariables; i++) printf(" %d", element[i]);
      printf("]\n");
      colIter.startFixedColumn(result, element);

      while (colIter) {
        element = colIter.getAssignments();
        printf(" <-- [%d", element[1]);
        for (int i=2; i<=nVariables; i++) printf(" %d", element[i]);
        printf("]\n");
        ++colIter;
      }

      ++beginIter;
      element = beginIter.getAssignments();
    }
  }
#endif

#if 0
  // Convert mtmxd to mxd
  forest* mxd =
    d->createForest(true, forest::BOOLEAN, forest::MULTI_TERMINAL);

  dd_edge toMxd(mxd);
  printf("Conversion MTMXD to MXD: ");

  if (test_conversion(result, toMxd)) {
    printf("Success!\n");
    if (verbose > 0) {
      printf("\n\ntoMxd: ");
      if (verbose > 1) toMxd.show(stdout, 3); else toMxd.show(stdout, 2);
    }
  } else {
    printf("Fail!\n");
  }

  if (verbose > 1) {
    printf("\n\nForest Info:\n");
    mtmxd->showInfo(stdout);
  }
#endif

  // Cleanup; in this case simply delete the domain
  destroyDomain(d);
  cleanup();

  free(bounds);
  for (int i = 0; i < nElements; ++i)
  {
    free(from[i]);
    free(to[i]);
  }
  free(from);
  free(to);

  return 0;
}

