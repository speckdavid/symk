
// $Id: test_mtmdd.cc 653 2016-02-17 00:00:51Z cjiang1209 $

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
 * rebuild_mdd.cc
 *
 * Rebuild a mdd forest with a different variable ordr.
 */

#include <stdlib.h>
#include <iostream>
#include <string.h>
#include "meddly.h"
#include "meddly_expert.h"
#include "reorder.h"

using namespace MEDDLY;

// Only include this file if referring to operations that are not
// available via the mddlib interface. 
// 
// Also, include this file if you would like to create a custom
// operation by deriving one of the existing operations.
// #include "operation_ext.h"

// Timer class
#include "timer.h"

#define USE_REALS 0

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

void printStats(const char* who, const forest* f)
{
  printf("%s stats:\n", who);
  const expert_forest* ef = (expert_forest*) f;
  FILE_output mout(stdout);
  ef->reportStats(mout, "\t",
    expert_forest::HUMAN_READABLE_MEMORY  |
    expert_forest::BASIC_STATS | expert_forest::EXTRA_STATS |
    expert_forest::STORAGE_STATS | expert_forest::HOLE_MANAGER_STATS
  );
}

// Tests a mtmdd operation on the elements provided.
// This function assumes that each element[i] represents
// an element in the given MTMDD.
dd_edge test_mtmdd(forest* mtmdd, const binary_opname* opCode,
    int** element, element_type* terms, int nElements)
{
  // A = first nElements/2 elements combined using +.
  // B = second nElements/2 elements combined using +.
  // C = A op B

  dd_edge A(mtmdd);
  dd_edge B(mtmdd);
  dd_edge C(mtmdd);

  int half = nElements/2;

  mtmdd->createEdge(element, terms, half, A);
  mtmdd->createEdge(element + half, terms + half, nElements - half, B);

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
#if 0
  return compute_manager::SUCCESS == ecm->apply(op, A, B)? true: false;
#else
  try {
    apply(COPY, A, B);
    return true;
  }
  catch (MEDDLY::error err) {
    printf("Apply error:\n %s\n", err.getName());
    return false;
  }
#endif
}


void printUsage(FILE *outputStream)
{
  fprintf(outputStream,
      "Usage: test_union_mtmdd <#Variables> <VariableBound> <#Elements>\
 [#terms]\n");
}


void reorderVariablesByRebuilding(dd_edge &e)
{
	printf("Initial: %ld\n", e.getForest()->getCurrentNumNodes());

	expert_forest* f = (expert_forest*)e.getForest();
	int num_var = f->getNumVariables();
	int* level2var = new int[num_var + 1];
	level2var[0] = 0;
	for(int i=1; i<=num_var; i++) {
		level2var[i] = i;
	}
	shuffle(level2var, 1, num_var);

	expert_forest* target = (expert_forest*)f->useDomain()->createForest(f->isForRelations(),
			f->getRangeType(), f->getEdgeLabeling() , f->getPolicies());
	target->reorderVariables(level2var);
	delete[] level2var;

	global_rebuilder gr(f, target);

	timer start;
	start.note_time();
	dd_edge ne = gr.rebuild(e);
	start.note_time();
	printf("Time interval: %.4e seconds\n", start.get_last_interval()/1000000.0);

	printf("Source: %ld\n", f->getCurrentNumNodes());
	printf("Final: %ld\n", target->getCurrentNumNodes());
}

int main(int argc, char *argv[])
{
  if (argc != 4 && argc != 5) {
    printUsage(stdout);
    exit(1);
  }

  srand(5u);

  // initialize number of variables, their bounds and the number of elements
  // to create

  int nVariables = 0;
  int variableBound = 0;
  int nElements = 0;
  int nTerms = 0;

  sscanf(argv[1], "%d", &nVariables);
  assert(nVariables > 0);

  sscanf(argv[2], "%d", &variableBound);
  assert(variableBound > 0);

  sscanf(argv[3], "%d", &nElements);
  assert(nElements > 0);

  if (argc == 5) {
    sscanf(argv[4], "%d", &nTerms);
    assert(nTerms > 0);
  } else {
    nTerms = variableBound;
  }

  printf("#variables: %d, variable bound: %d, #elements: %d\n",
      nVariables, variableBound, nElements);

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
      element[i][j] = int(float(variableBound) * rand() / (RAND_MAX + 1.0));
      assert(element[i][j] >= 0 && element[i][j] < variableBound);
    }
    terms[i] =
      element_type(float(nTerms) * rand() / (RAND_MAX + 1.0));
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
  forest::policies p(false);
  p.setPessimistic();
#if USE_REALS
  forest* mtmdd =
    d->createForest(false, forest::REAL, forest::MULTI_TERMINAL, p);
#else
  forest* mtmdd =
    d->createForest(false, forest::INTEGER, forest::MULTI_TERMINAL, p);
#endif
  assert(mtmdd != 0);

  // print elements
  if (verbose > 0) {
    for (int i = 0; i < nElements; ++i)
    {
      printf("Element %d: [%d", i, element[i][0]);
      for (int j = 1; j <= nVariables; ++j)
      {
        printf(" %d", element[i][j]);
      }
#if USE_REALS
      printf(": %f]\n", terms[i]);
#else
      printf(": %d]\n", terms[i]);
#endif
    }
  }

  timer start;
  start.note_time();
  dd_edge result = test_mtmdd(mtmdd, PLUS, element, terms, nElements);
  start.note_time();
  printf("Time interval: %.4e seconds\n",
      start.get_last_interval()/1000000.0);

  printf("Peak Nodes in MDD: %ld\n", mtmdd->getPeakNumNodes());

  reorderVariablesByRebuilding(result);

#if 0
  // Convert mtmdd to mdd
  forest* mdd =
    d->createForest(false, forest::BOOLEAN, forest::MULTI_TERMINAL);

  dd_edge toMdd(mdd);
  printf("\n\nConversion MTMDD to MDD: ");

  if (test_conversion(result, toMdd)) {
    printf("Success!\n");
    if (verbose > 0) {
      printf("toMdd: ");
      if (verbose > 1) toMdd.show(stdout, 3); else toMdd.show(stdout, 2);
    }
  } else {
    printf("Fail!\n");
  }

  printf("\n\nMTMDD Forest Info:\n");
  mtmdd->showInfo(stdout);
  printf("\n\nMDD Forest Info:\n");
  mdd->showInfo(stdout);

  // Convert mtmdd to ev+mdd
  forest* evmdd =
    d->createForest(false, forest::INTEGER, forest::EVPLUS);
  assert(evmdd != 0);
  assert(forest::SUCCESS ==
      //  evmdd->setNodeDeletion(forest::PESSIMISTIC_DELETION));
      evmdd->setNodeDeletion(forest::OPTIMISTIC_DELETION));

  dd_edge toEvMdd(evmdd);
  printf("\n\nConversion MTMDD to EV+MDD: ");

  if (test_conversion(result, toEvMdd)) {
    printf("Success!\n");

    if (verbose > 0) {
      printf("MtMdd: ");
      result.show(stdout, verbose > 1? 3: 2);
      printf("toEvMdd: ");
      toEvMdd.show(stdout, verbose > 1? 3: 2);
    }

  } else {
    printf("Fail!\n");
  }

  printf("\n\nMTMDD Forest Info:\n");
  mtmdd->showInfo(stdout);
  printf("\n\nEVMDD Forest Info:\n");
  evmdd->showInfo(stdout);
#endif

//  if (true) {
//    dd_edge reachableStates(result);
//    start.note_time();
//    unsigned counter = 0;
//    for (enumerator iter(reachableStates);
//        iter; ++iter, ++counter)
//    {
//      const int* element = iter.getAssignments();
//      const int* curr = element + nVariables;
//      const int* end = element - 1;
//      printf("%d: [%d", counter, *curr--);
//      while (curr != end) { printf(" %d", *curr--); }
//      printf("]\n");
//    }
//    start.note_time();
//    printf("Iterator traversal time (%0.4e elements): %0.4e seconds\n",
//        double(counter), start.get_last_interval()/double(1000000.0));
//    double c;
//    apply(CARDINALITY, reachableStates, c);
//    printf("Cardinality: %0.4e\n", c);
//  }

  // Cleanup; in this case simply delete the domain
  destroyDomain(d);

  free(bounds);
  for (int i = 0; i < nElements; ++i)
  {
    free(element[i]);
  }
  free(element);

  cleanup();

  return 0;
}

