
// $Id$

/*
    Meddly: Multi-terminal and Edge-valued Decision Diagram LibrarY.
    Copyright (C) 2011, Iowa State University Research Foundation, Inc.

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

#include <cstdlib>
#include <string.h>
#include <unistd.h>

#include "meddly.h"
#include "simple_model.h"

#define WRITE_MXD
#define WRITE_MDD
#define WRITE_EVMDD
// #define DEBUG_FILE

const char* kanban[] = {
  "X-+..............",  // Tin1
  "X.-+.............",  // Tr1
  "X.+-.............",  // Tb1
  "X.-.+............",  // Tg1
  "X.....-+.........",  // Tr2
  "X.....+-.........",  // Tb2
  "X.....-.+........",  // Tg2
  "X+..--+..-+......",  // Ts1_23
  "X.........-+.....",  // Tr3
  "X.........+-.....",  // Tb3
  "X.........-.+....",  // Tg3
  "X....+..-+..--+..",  // Ts23_4
  "X.............-+.",  // Tr4
  "X.............+-.",  // Tb4
  "X............+..-",  // Tout4
  "X.............-.+"   // Tg4
};

long expected[] = { 
  1, 160, 4600, 58400, 454475, 2546432, 11261376, 
  41644800, 133865325, 384392800, 1005927208 
};

using namespace MEDDLY;


// Build the domain
inline domain* buildKanbanDomain(int N)
{
  int sizes[16];
  for (int i=15; i>=0; i--) sizes[i] = N+1;
  return createDomainBottomUp(sizes, 16);
}

// Build the initial state
inline void buildInitial(int N, forest* mdd, dd_edge &init_state)
{
  int initial[17];
  for (int i=16; i; i--) initial[i] = 0;
  initial[1] = initial[5] = initial[9] = initial[13] = N;
  int* initptr = initial;
  mdd->createEdge(&initptr, 1, init_state);
}

/*
    Generate transition relation, reachability set for given N,
    and write them to a file.
*/
long writeReachset(FILE* s, int N)
{
  // Build domain
  domain* d = buildKanbanDomain(N);

  // Build initial state
  forest* mdd = d->createForest(0, forest::BOOLEAN, forest::MULTI_TERMINAL);
  dd_edge init_state(mdd);
  buildInitial(N, mdd, init_state);

  // Build next-state function
  forest* mxd = d->createForest(1, forest::BOOLEAN, forest::MULTI_TERMINAL);
  dd_edge nsf(mxd);
  buildNextStateFunction(kanban, 16, mxd, nsf); 

  // Build reachable states
  dd_edge reachable(mdd);
  apply(REACHABLE_STATES_DFS, init_state, nsf, reachable);

  // Build index set for reachable states
  forest* evmdd = d->createForest(0, forest::INTEGER, forest::INDEX_SET);
  dd_edge reach_index(evmdd);
  apply(CONVERT_TO_INDEX_SET, reachable, reach_index);

  long c;
  apply(CARDINALITY, reachable, c);

  FILE_output mys(s);
#ifdef WRITE_MXD
  // Write NSF
  mxd->writeEdges(mys, &nsf, 1);
#endif

#ifdef WRITE_MDD
  // Write initial state + reachable states
  dd_edge list[2];
  list[0] = init_state;
  list[1] = reachable;
  mdd->writeEdges(mys, list, 2);
#endif

#ifdef WRITE_EVMDD
  evmdd->writeEdges(mys, &reach_index, 1);
#endif

  destroyDomain(d);
  
  return c;
}


/*
    Generate transition relation, reachability set for given N,
    then read from a file and check for equality.
*/
bool generateAndRead(FILE* s, int N)
{
  // Build domain
  domain* d = buildKanbanDomain(N);

  // Build initial state
  forest* mdd = d->createForest(0, forest::BOOLEAN, forest::MULTI_TERMINAL);
  dd_edge init_state(mdd);
  buildInitial(N, mdd, init_state);

  // Build next-state function
  forest* mxd = d->createForest(1, forest::BOOLEAN, forest::MULTI_TERMINAL);
  dd_edge nsf(mxd);
  buildNextStateFunction(kanban, 16, mxd, nsf); 

  // Build reachable states
  dd_edge reachable(mdd);
  apply(REACHABLE_STATES_DFS, init_state, nsf, reachable);

  // Build index set for reachable states
  forest* evmdd = d->createForest(0, forest::INTEGER, forest::INDEX_SET);
  dd_edge reach_index(evmdd);
  apply(CONVERT_TO_INDEX_SET, reachable, reach_index);

  // Now, read from the file and verify
  FILE_input mys(s);

  dd_edge list[2];
#ifdef WRITE_MXD
  mxd->readEdges(mys, list, 1);
  if (list[0] != nsf) {
    printf("Failed to generate and read MXD\n");
    return false;
  }
#endif

#ifdef WRITE_MDD
  mdd->readEdges(mys, list, 2);
  if (list[0] != init_state) {
    printf("Failed to generate and read initial state\n");
    return false;
  }
  if (list[1] != reachable) {
    printf("Failed to generate and read reachable states\n");
    return false;
  }
#endif

#ifdef WRITE_EVMDD
  evmdd->readEdges(mys, list, 1);
  if (list[0] != reach_index) {
    printf("Failed to generate and read reachable state indexes\n");
    return false;
  }
#endif

  destroyDomain(d);
  return true;
}


/*
    Read the transition relation, reachability set from a file,
    then generate them for a given N and check for equality.
*/
bool readAndGenerate(FILE* s, int N)
{
  FILE_input mys(s);

  // Build domain
  domain* d = buildKanbanDomain(N);

  // Build (empty) forests
  forest* mdd = d->createForest(0, forest::BOOLEAN, forest::MULTI_TERMINAL);
  forest* mxd = d->createForest(1, forest::BOOLEAN, forest::MULTI_TERMINAL);
  forest* evmdd = d->createForest(0, forest::INTEGER, forest::INDEX_SET);

  dd_edge mxdfile;
  dd_edge mddfile[2];
  dd_edge indexfile;
  // Read from the file
#ifdef WRITE_MXD
  mxd->readEdges(mys, &mxdfile, 1);
#endif
#ifdef WRITE_MDD
  mdd->readEdges(mys, mddfile, 2);
#endif
#ifdef WRITE_EVMDD
  evmdd->readEdges(mys, &indexfile, 1);
#endif

  // Build initial state
  dd_edge init_state(mdd);
  buildInitial(N, mdd, init_state);

  // Build next-state function
  dd_edge nsf(mxd);
  buildNextStateFunction(kanban, 16, mxd, nsf); 

  // Build reachable states
  dd_edge reachable(mdd);
  apply(REACHABLE_STATES_DFS, init_state, nsf, reachable);

  // Build index set for reachable states
  dd_edge reach_index(evmdd);
  apply(CONVERT_TO_INDEX_SET, reachable, reach_index);

  // Verify
#ifdef WRITE_MXD
  if (mxdfile != nsf) {
    printf("Failed to read and generate MXD\n");
    return false;
  }
#endif
#ifdef WRITE_MDD
  if (mddfile[0] != init_state) {
    printf("Failed to generate and read initial state\n");
    return false;
  }
  if (mddfile[1] != reachable) {
    printf("Failed to generate and read reachable states\n");
    return false;
  }
#endif
#ifdef WRITE_EVMDD
  if (indexfile != reach_index) {
    printf("Failed to generate and read reachable state indexes\n");
    return false;
  }
#endif

  destroyDomain(d);
  return true;
}



/*
    Main program, of course
*/
int main()
{
  const int N = 11;
  MEDDLY::initialize();

  char filename[20];
  strcpy(filename, "kan_io.data.XXXXXX");
  mktemp(filename);
#ifdef DEBUG_FILE
  printf("Creating file %s\n", filename);
#endif

  FILE* s = fopen(filename, "w");
  if (0==s) {
    printf("Couldn't open file %s for writing\n", filename);
    return 2;
  }

  try {
    printf("Saving dds to file...\n");
    for (int n=1; n<N; n++) {
      printf("N=%2d:  ", n);
      fflush(stdout);
      long c = writeReachset(s, n);
      printf("%12ld states\n", c);
      if (c !=expected[n]) throw 1;
    }
    fclose(s);

    // read back the file into already built forest

    s = fopen(filename, "r");
    if (0==s) {
      printf("Couldn't open file %s for reading\n", filename);
      throw 2;
    }

    printf("Generate and read...\n");
    for (int n=1; n<N; n++) {
      printf("N=%2d:  ", n);
      fflush(stdout);
      if (!generateAndRead(s, n)) throw 3;
      printf("%19s\n", "verified");
    }
    fclose(s);

    // read back the file and then build up forest

    s = fopen(filename, "r");
    if (0==s) {
      printf("Couldn't open file %s for reading\n", filename);
      throw 2;
    }

    printf("Read and generate...\n");
    for (int n=1; n<N; n++) {
      printf("N=%2d:  ", n);
      fflush(stdout);
      if (!readAndGenerate(s, n)) throw 4;
      printf("%19s\n", "verified");
    }
    fclose(s);

  
    // Cleanup
#ifndef DEBUG_FILE
    remove(filename);
#endif
    MEDDLY::cleanup();
    printf("Done\n");
    return 0;
  }
  catch (int e) {
    // cleanup
#ifndef DEBUG_FILE
    remove(filename);
#endif
    MEDDLY::cleanup();
    printf("\nError %d\n", e);
    return e;
  }
  catch (MEDDLY::error e) {
    // cleanup
#ifndef DEBUG_FILE
    remove(filename);
#endif
    MEDDLY::cleanup();
    printf("\nError: %s\n", e.getName());
    return 1;
  }
  catch (...) {
    // cleanup
#ifndef DEBUG_FILE
    remove(filename);
#endif
    MEDDLY::cleanup();
    printf("\nFailed (caught exception)\n");
    return 1;
  }
}

