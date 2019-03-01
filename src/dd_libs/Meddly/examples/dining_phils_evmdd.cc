
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



/*
  State space generation for Dining Philosphers (N=2), with EV+MDDs
  to compute distances.

  The model has 2 philosophers and 2 forks.

  Each philosopher can be in state {I, W, L, R, E} where
  
  I:    idle philosopher
  WB:   philosopher is waiting for both forks
  HL:   philosopher has left fork
  HR:   philosopher has right fork
  E:    philosopher is eating

  Each fork can be in state {A, NA} where
  
  A:    fork is available
  NA:   fork is not available

  Philosphers can move from one state to another as:
  
  I -> WB
 
  The synchronization between philosopher 1 and the forks:

  WB1 ->  HR1
  A1  ->  NA1

  WB1 ->  HL1
  A2  ->  NA2

  HR1 ->  E1
  A2  ->  NA2

  HL1 ->  E1
  A1  ->  NA1

  E1  ->  I1
  NA1 ->  A1
  NA2 ->  A2

  The synchronization between philosopher 2 and the forks:

  WB2 ->  HR2
  A2  ->  NA2

  WB2 ->  HL2
  A1  ->  NA1

  HR2 ->  E2
  A1  ->  NA1

  HL2 ->  E2
  A2  ->  NA2

  E2  ->  I2
  NA1 ->  A1
  NA2 ->  A2

  Initially, all forks are in state "A" and all philosophers are in state "I".
  How many reachable states?
  Exceptions?
*/

#include <cstdio>
#include <sys/time.h>
#include <sys/resource.h>
#include <domain.h>
#include <forest.h>
#include <dd_edge.h>
#include <ophandle.h>

level *variables = NULL;
int *sizes = NULL;
int *initst = NULL;
int *initstev = NULL;
dd_edge **synch = NULL;
forest_hndl relation;

char* ll_to_pa(long long n)
{
  int size = 0;
  long long copy_n = n;
  while (copy_n > 0) { size++; copy_n = copy_n / 10; }
  int commas = (size - 1) / 3;
  size = size + commas;
  char *result = (char *) malloc ((size + 1) * sizeof(char));
  int iter = 1;
  for (int i=size-1; i>=0; i--, iter++) {
    if ((iter % 4) == 0) { result[i] = ','; }
    else { result[i] = (n % 10) + '0'; n = n / 10; }
  }
  result[size] = '\0';
  return result;
}

void Init(int N)
{
  // store for level handles
  variables = (level *) malloc((N*2 + 1) * sizeof(level));
  memset(variables, 0, (N*2 + 1) * sizeof(level));
  
  // node size for each level
  sizes = (int *) malloc((N*2 + 1) * sizeof(int));
  sizes[0] = 0;
  for (int i = N*2; i > 0; ) {
    // odd level first, belongs to forks
    sizes[i--] = 2;
    // level below is for philosophers
    sizes[i--] = 5;
  }
  
  // sets of states; initialize to all zero with zero edge values
  initst = (int *) malloc((N*2 + 1) * sizeof(int));
  memset(initst, 0, (N*2 + 1) * sizeof(level));
  initstev = (int *) malloc((N*2 + 1) * sizeof(int));
  memset(initstev, 0, (N*2 + 1) * sizeof(level));

  synch = (dd_edge **) malloc(N * sizeof(dd_edge *));

  assert(variables != NULL);
  assert(sizes != NULL);
  assert(initst != NULL);
  assert(initstev != NULL);
  assert(synch != NULL);
}

void CheckVars(int N)
{
  /* Sanity check */
  int i;
  N = N*2 + 1;
  for (i=1; i<N; i++) 
    if ((variables[i] > N) || (variables[i] < 1)) {
      fprintf(stderr, "Level handle for variable %d is %d, ", i, variables[i]);
      fprintf(stderr, "outside of expected range\n");
      exit(1);
    }
}

void Try(error_code h)
{
  switch (h) {
    case SUCCESS: 	return;

    // list out other cases here?
    default:
	fprintf(stderr, "Error\n");
	exit(1);
  }
}

void SetIntArray(int *p, int p_size, int c)
{
  for (int i=0; i<p_size; i++) p[i] = c;
}

dd_edge* MakeSynchP_Forks(int ph_num, int N)
{
  CHECK_RANGE(0, ph_num, N);
  dd_tempedge* temp;
  int sz = N*2 + 1;
  int *from = (int *) malloc(sz * sizeof(int));
  int *to = (int *) malloc(sz * sizeof(int));
  from[0] = 0;
  to [0] = 0;
  temp = CreateTempEdge(relation, NULL);

  /* Right Fork level is above Philosopher level */
  int ph = ph_num * 2 + 1;  // 0th philosopher at (0*2+1) = 1st level
  int rf = ph + 1;
  int lf = (ph_num > 0)? ph - 1: N*2;

  /*
  printf("%s: ph_num = %d, ph = %d, rf = %d, lf = %d\n",
      __func__, ph_num, ph, rf, lf);
  */

  /* I(ph) -> WB(ph) */
  SetIntArray(from+1, sz-1, -2);
  SetIntArray(to+1, sz-1, -2);
  from[variables[ph]] = 0;
    to[variables[ph]] = 1;
  AddMatrixElement(temp, from, to, sz, true);

  /* WB(ph) -> HR(ph), A(rf) -> NA(rf) */
  SetIntArray(from+1, sz-1, -2);
  SetIntArray(to+1, sz-1, -2);
  from[variables[ph]] = 1;
    to[variables[ph]] = 3;
  from[variables[rf]] = 0;
    to[variables[rf]] = 1;
  AddMatrixElement(temp, from, to, sz, true);
  
  /* WB(ph) -> HL(ph), A(lf) -> NA(lf) */
  SetIntArray(from+1, sz-1, -2);
  SetIntArray(to+1, sz-1, -2);
  from[variables[ph]] = 1;
    to[variables[ph]] = 2;
  from[variables[lf]] = 0;
    to[variables[lf]] = 1;
  AddMatrixElement(temp, from, to, sz, true);

  /* HR(ph) -> E(ph), A(lf) -> NA(lf) */
  SetIntArray(from+1, sz-1, -2);
  SetIntArray(to+1, sz-1, -2);
  from[variables[ph]] = 3;
    to[variables[ph]] = 4;
  from[variables[lf]] = 0;
    to[variables[lf]] = 1;
  AddMatrixElement(temp, from, to, sz, true);

  /* HL(ph) -> E(ph), A(rf) -> NA(rf) */
  SetIntArray(from+1, sz-1, -2);
  SetIntArray(to+1, sz-1, -2);
  from[variables[ph]] = 2;
    to[variables[ph]] = 4;
  from[variables[rf]] = 0;
    to[variables[rf]] = 1;
  AddMatrixElement(temp, from, to, sz, true);

  /* E(ph) -> I(ph), NA(rf) -> A(rf), NA(lf) -> A(lf) */
  SetIntArray(from+1, sz-1, -2);
  SetIntArray(to+1, sz-1, -2);
  from[variables[ph]] = 4;
    to[variables[ph]] = 0;
  from[variables[rf]] = 1;
    to[variables[rf]] = 0;
  from[variables[lf]] = 1;
    to[variables[lf]] = 0;
  AddMatrixElement(temp, from, to, sz, true);

  return CreateEdge(temp);
}

void usage()
{
  printf("Usage: dining_phils_batch [-n<#phils>|-m<#MB>|-dfs|-p|-pgif]\n");
  printf("-n   : \
      number of philosophers\n");
  printf("-m   : \
      memory available to this program in MB\n");
  printf("-dfs : \
      use depth-first algorithm to compute reachable states\n");
  printf("-p   : \
      prints initial states, nsf, reachable states on stdout\n");
  printf("-pgif: \
      writes GIFs for initial states, nsf, reachable states\n");
  printf("\n");
}

int main(int argc, char *argv[])
{
  int N = 0; // number of philosophers
  bool make_gifs = false;
  bool pretty_print = false;
  bool dfs = false;
  if (argc > 1) {
    assert(argc > 1 && argc <= 5);
    if (argc > 1) {
      // assert(argc == 3);
      assert(argc < 6);
      for (int i=1; i<argc; i++) {
        char *cmd = argv[i];
        if (strncmp(cmd, "-pgif", 6) == 0) make_gifs = true;
        else if (strncmp(cmd, "-p", 3) == 0) pretty_print = true;
        else if (strncmp(cmd, "-dfs", 5) == 0) dfs = true;
        else if (strncmp(cmd, "-n", 2) == 0) {
          N = strtol(&cmd[2], NULL, 10);
        } else if (strncmp(cmd, "-m", 2) == 0) {
          int mem_total = strtol(&cmd[2], NULL, 10);
          if (mem_total < 1 || mem_total > 100*1024) { // 10 GB!
            usage();
            exit(1);
          }
          // set up memory available
          InitMemoryManager(mem_total*1024*1024);
        } else {
          usage();
          exit(1);
        }
      }
    }
  }
  while (N < 2) {
    printf("Enter the number of philosophers (at least 2): ");
    scanf("%d", &N);
  }
  
  int i;
  domain* d;
  forest_hndl states;
  dd_edge* initial;

  assert(N > 1);

  // set up arrays based on N
  Init(N);
  
  /* transition relations */
  dd_edge* nsf; 

  /* Set up the state variables.
     Use one per philosopher and one per fork with values as described at
     the top of the document:
     */
  d = CreateDomain(N*2, sizes, variables);
  if (NULL==d) {
    fprintf(stderr, "Couldn't create domain\n");
    return 1;
  }
  CheckVars(N);

  /* Create forests */
#if 1
  states = CreateForest(d, EVMDD, false, forest::FULLY_REDUCED, FULL_OR_SPARSE_STORAGE);
#else
  states = CreateForest(d, EVMDD, false, forest::QUASI_REDUCED, FULL_STORAGE);
#endif

#if 1
  relation = CreateForest(d, MXD, false, forest::IDENTITY_REDUCED, FULL_OR_SPARSE_STORAGE);
#else
  relation = CreateForest(d, MXD, false, forest::IDENTITY_REDUCED, FULL_STORAGE);
#endif

  if (INVALID_FOREST == states) {
    fprintf(stderr, "Couldn't create forest of states\n");
    return 1;
  } else {
    fprintf(stderr, "Created forest of states\n");
  }
  if (INVALID_FOREST == relation) {
    fprintf(stderr, "Couldn't create forest of relations\n");
    return 1;
  } else {
    fprintf(stderr, "Created forest of relations\n");
  }

  /* Build set of initial states */
  initial = CreateEVVectorElement(states, initst, initstev, N*2 + 1, true); 

  if (NULL==initial) {
    fprintf(stderr, "Couldn't create set of initial states\n");
    return 1;
  } else {
    fprintf(stderr, "Created set of initial states\n");
  }

  /* Build synchronizing transitions */
  for (i=0; i<N; i++) {
    synch[i] = MakeSynchP_Forks(i, N);
  }

  /* Build overall next-state function */
  nsf = NULL;
  ApplyBinary(OP_UNION, synch[0], synch[1], nsf);
  for (i=2; i<N; i++) {
    ApplyBinary(OP_UNION, nsf, synch[i], nsf);
  }

  if (NULL == nsf) {
    fprintf(stderr, "Couldn't create next-state function\n");
    return 1;
  } else {
    fprintf(stderr, "Created next-state function\n");
  }

  fflush(stderr);

  dd_edge *curr = NULL;

#if 0
  printf("Initial states: ");
  ShowDDEdge(stdout, initial);
  printf("\nTransition relation nodes\n");
  for (int i=0; i<N; i++) {
    printf("synchronizing philosopher %d: ", i);
    ShowDDEdge(stdout, synch[i]);
    printf("\n"); 
  }
  printf("Overall transition relation: ");
  ShowDDEdge(stdout, nsf);
  printf("\n"); 

  // Image operations
  printf("\nInitial State: ");
  ShowDDEdge(stdout, initial);
  dd_edge *curr = NULL;
  Saturate(initial, nsf, curr);
  printf("\nStates reachable from Initial State: ");
  ShowDDEdge(stdout, curr);
  printf("\n"); 
#endif

  struct rusage start, stop;
  assert(getrusage(RUSAGE_SELF, &start) == 0);
#if 1
  if (dfs) {
    vector<dd_edge*> *edges = NULL;
    assert(SUCCESS == SplitMxd(nsf, edges));
    assert(SUCCESS == Saturate(initial, edges, curr));
  } else {
    Saturate(initial, nsf, curr);
  }
#else
  int choice = 0;
  dd_edge *prev = NULL;
  curr = CopyEdge(initial);
  while(true) {
    printf("\t1. PostImage Phil1\n");
    printf("\t2. PostImage Phil2\n");
    printf("\t3. PostImage (Phil1 U Phil2)\n");
    printf("\t4. Union (curr, prev)\n");
    printf("\t5. Print current states\n");
    printf("\t0. Stop\n");
    printf("Enter choice (0-5): ");
    scanf("%d", &choice);
    if (choice == 0) break;
    switch (choice) {
      case 1: if (prev) { ReleaseEdge(prev); } prev = CopyEdge(curr);
              PostImage(curr, synch[0], curr); break;
      case 2: if (prev) { ReleaseEdge(prev); } prev = CopyEdge(curr);
              PostImage(curr, synch[1], curr); break;
      case 3: if (prev) { ReleaseEdge(prev); } prev = CopyEdge(curr);
              PostImage(curr, nsf, curr); break;
      case 4: ApplyBinary(OP_UNION, curr, prev, curr); break;
      case 5: ShowDDEdge(stdout, curr); break;
    }
  }
#endif
  assert(getrusage(RUSAGE_SELF, &stop) == 0);

  // stop.ru_utime - start.ru_utime
  suseconds_t u_sat_time =
    (stop.ru_utime.tv_sec * 1000000 + stop.ru_utime.tv_usec) -
    (start.ru_utime.tv_sec * 1000000 + start.ru_utime.tv_usec);
  suseconds_t s_sat_time =
    (stop.ru_stime.tv_sec * 1000000 + stop.ru_stime.tv_usec) -
    (start.ru_stime.tv_sec * 1000000 + start.ru_stime.tv_usec);
  
  printf("\nTime for constructing initial states and nsf:\n");
  printf("  %ld.%06ld sec user, %ld.%06ld sec system\n",
      (long int)start.ru_utime.tv_sec, (long int)start.ru_utime.tv_usec,
      (long int)start.ru_stime.tv_sec, (long int)start.ru_stime.tv_usec);

  printf("\nTime for constructing reachability set:\n");
  printf("  %06f sec user, %06f system\n",
      u_sat_time/1000000.0, s_sat_time/1000000.0);
  fflush(stdout);

#if 1
  double card = 0; //  LONG_LONG_MAX;
  card = Cardinality(curr);
  printf("\n# of reachable states: %1.3e\n", card);
  double max_dist = 0;
  max_dist = MaxDistance(curr);
  printf("\nmax distance: %1.3e\n", max_dist);
#endif

  printf("\nPeak memory usage: %ld\n", stop.ru_maxrss);
  printf("\nIntegral shared memory usage: %ld\n", stop.ru_ixrss);

  const char *fn[] = {"reachable", "initial", "nsf", "gif"};
  if (make_gifs) {
    CreateDDEdgePic(fn[1], fn[3], initial);
    printf("Wrote initial states to %s.%s\n", fn[1], fn[3]);
    CreateDDEdgePic(fn[2], fn[3], nsf);
    printf("Wrote next-state function to %s.%s\n", fn[2], fn[3]);
    CreateDDEdgePic(fn[0], fn[3], curr);
    printf("Wrote reachable states to %s.%s\n", fn[0], fn[3]);
    char temp_fn[100];
    for (i=0; i<N; i++) {
      sprintf(temp_fn, "p%d", i);
      CreateDDEdgePic(temp_fn, fn[3], synch[i]);
      printf("Wrote nsf for philosopher %d to %s.%s\n", i, temp_fn, fn[3]);
    }
  } else if (pretty_print) {
    printf("\nInitial States: ");
    ShowDDEdge(stdout, initial);
    printf("\n");
    printf("\nNext-State Function: ");
    ShowDDEdge(stdout, nsf);
    printf("\n");
#if 1
    printf("\nMDD internal storage: ");
    ShowForestNodes(stdout, states);
    printf("\n");
#endif
    printf("\nReachable States: ");
    ShowDDEdge(stdout, curr);
    printf("\n");
  }
 
  DestroyForest(states);
  if (INVALID_FOREST != states) {
    fprintf(stderr, "Couldn't destroy forest of states\n");
    return 1;
  } else {
    fprintf(stderr, "Destroyed forest of states\n");
  }

  DestroyForest(relation);
  if (INVALID_FOREST != relation) {
    fprintf(stderr, "Couldn't destroy forest of relations\n");
    return 1;
  } else {
    fprintf(stderr, "Destroyed forest of relations\n");
  }

  DestroyDomain(d);

  printf("\n\nDONE\n");
  return 0;
}

