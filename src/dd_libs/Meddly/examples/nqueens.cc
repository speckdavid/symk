
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
    Builds the set of solutions to the N-queens problem for 
    user-specified N.

    In other words, finds all possible ways to put N queens onto
    an NxN chessboard so that no queen is attacking any other queen.
    Clearly, we cannot have two queens in any given row, therefore
    every such solution must have exactly one queen in each row.
    We use an MDD with N variables, with variable x_i indicating
    the column position (from 1 to N) for the queen in row i.
*/

#include <cstdio>
#include <fstream>

#include "meddly.h"
#include "meddly_expert.h"
#include "timer.h"
#include "loggers.h"

// #define SHOW_ALL_SOLUTIONS

using namespace MEDDLY;

int N;
int* scratch;
const char* lfile;

bool use_folding;


void intersect_acc(dd_edge** A, int L)
{
  if (0==A[0]) {
    // find first non-zero and swap
    for (int i=1; i<L; i++) {
      if (A[i]) {
        A[0] = A[i];
        A[i] = 0;
        break;
      }
    }
    if (0==A[0]) return;
  }
  fprintf(stderr, "\t");
  for (int i=1; i<L; i++) {
    fprintf(stderr, "%d ", L-i);
    if (A[i]) {
      apply(MULTIPLY, *A[0], *A[i], *A[0]);
      delete A[i];
      A[i] = 0;
      // operation::removeStalesFromMonolithic();
    }
  }
  fprintf(stderr, "\n");
}


void intersect_fold(dd_edge** A, int L)
{
  while (L>1) {
    fprintf(stderr, "\t%2d terms to combine ", L);
    // combine adjacent pairs
    for (int i=0; i<L; i+=2) {
      if (A[i] && A[i+1]) {
        apply(MULTIPLY, *A[i], *A[i+1], *A[i]);
        delete A[i+1];
        A[i+1] = 0;
        fprintf(stderr, ".");
      }
    } // for i
    fprintf(stderr, "\n");
    // compact
    int p = 0;
    for (int i=0; i<L; i++) {
      if (A[i]) {
        if (i>p) {
          A[p] = A[i];
          A[i] = 0;
        }
        p++;
      }
    } // for i
    L = p;
  } // while
}


inline void intersect(dd_edge** A, int L)
{
  if (use_folding)
    intersect_fold(A, L);
  else
    intersect_acc(A, L);
}


void createQueenNodes(forest* f, int q, dd_edge &col, dd_edge &cp, dd_edge &cm)
{
  assert(q>0);
  assert(q<=N);
  f->createEdgeForVar(q, false, col);
  for (int i=0; i<N; i++) {
    scratch[i] = i+q;
  }
  f->createEdgeForVar(q, false, scratch, cp);
  for (int i=0; i<N; i++) {
    scratch[i] = i-q;
  }
  f->createEdgeForVar(q, false, scratch, cm);
}

bool processArgs(int argc, const char** argv, forest::policies &p)
{
  lfile = 0;
  p.setPessimistic();
  bool setN = false;
  use_folding = false;
  for (int i=1; i<argc; i++) {
    if ('-' == argv[i][0]) {
      if (strcmp("-acc", argv[i])==0) {
        use_folding = false;
        continue;
      }
      if (strcmp("-fold", argv[i])==0) {
        use_folding = true;
        continue;
      }
      if (strcmp("-opt", argv[i])==0) {
        p.setOptimistic();
        continue;
      }
      if (strcmp("-pess", argv[i])==0) {
        p.setPessimistic();
        continue;
      }
      if (strcmp("-l", argv[i])==0) {
        lfile = argv[i+1];
        i++;
        continue;
      }
      return false;
    }
    if (setN) return false;
    N = atoi(argv[i]);
    setN = true;
  }

  if (!setN || N<1) return false;
  return true;
}

int usage(const char* who)
{
  /* Strip leading directory, if any: */
  const char* name = who;
  for (const char* ptr=who; *ptr; ptr++) {
    if ('/' == *ptr) name = ptr+1;
  }
  printf("Usage: %s [options] N\n\n", name);
  printf("\t       N:  board dimension\n");
  printf("\t    -acc:  Accumulate constraints in order (default)\n");
  printf("\t   -fold:  Accumulate constraints in pairs\n");
  printf("\t    -opt:  Optimistic node deletion\n");
  printf("\t   -pess:  Pessimistic node deletion (default)\n");
  printf("\t-l lfile:  Write logging information to specified file\n\n");
  return 1;
}

int main(int argc, const char** argv)
{
  initialize();
  forest::policies p(false);
  if (!processArgs(argc, argv, p)) return usage(argv[0]);
  timer watch;
  printf("Using %s\n", getLibraryInfo(0));
  printf("%d-Queens solutions.\n", N);
  scratch = new int[N+1];
  
  watch.note_time();
  printf("Initializing domain and forest\n");
  const char* ndp = "unknown node deletion";
  switch (p.deletion) {
    case forest::policies::NEVER_DELETE:
        ndp = "never delete";
        break;

    case forest::policies::OPTIMISTIC_DELETION:
        ndp = "optimistic node deletion";
        break;

    case forest::policies::PESSIMISTIC_DELETION:
        ndp = "pessimistic node deletion";
        break;
  }
  printf("\tUsing %s policy\n", ndp);

  for (int i=0; i<N; i++) {
    scratch[i] = N;
  }
  domain* d = createDomainBottomUp(scratch, N);
  assert(d);
  expert_forest* f = (expert_forest*)
    d->createForest(false, forest::INTEGER, forest::MULTI_TERMINAL, p);
  assert(f);

  std::ofstream log;
  forest::logger* LOG = 0;
  if (lfile) {
    log.open(lfile, std::ofstream::out);
    if (!log) {
      printf("Couldn't open %s for writing, no logging\n", lfile);
    } else {
      LOG = new simple_logger(log);
      LOG->recordNodeCounts();
      LOG->recordTimeStamps();
      char comment[80];
      snprintf(comment, 80, "Automatically generated by nqueens (N=%d)", N);
      LOG->addComment(comment);
      f->setLogger(LOG, "forest name");
      LOG->newPhase(f, "Initializing");
    }
  }

  printf("Building nodes for queen column and diagonals\n");

  dd_edge** col = new dd_edge*[N];
  dd_edge** dgp = new dd_edge*[N];
  dd_edge** dgm = new dd_edge*[N];
  dd_edge** constr = new dd_edge*[N+1];


  for (int i=0; i<N; i++) {
    col[i] = new dd_edge(f);
    dgp[i] = new dd_edge(f);
    dgm[i] = new dd_edge(f);
    createQueenNodes(f, i+1, *col[i], *dgp[i], *dgm[i]);
    constr[i] = new dd_edge(f);
    f->createEdge(int(1), *constr[i]);
  }
  constr[N] = 0;

  for (int i=0; i<N-1; i++) {
    char buffer[80];
    snprintf(buffer, 80, "Building queen %2d constraints\n", i+1);
    printf("%s\n", buffer);
    if (LOG) LOG->newPhase(f, buffer);
    for (int j=N-1; j>i; j--) {
      dd_edge uniq_col(f);
      apply(NOT_EQUAL, *col[i], *col[j], uniq_col);
      dd_edge uniq_dgp(f);
      apply(NOT_EQUAL, *dgp[i], *dgp[j], uniq_dgp);
      dd_edge uniq_dgm(f);
      apply(NOT_EQUAL, *dgm[i], *dgm[j], uniq_dgm);
      // build overall "not attacking each other" set...
      apply(MULTIPLY, uniq_col, uniq_dgp, uniq_col);
      apply(MULTIPLY, uniq_col, uniq_dgm, uniq_col);
      int k = uniq_col.getLevel()-1;
      if (k<0) k=0;
      assert(k<N);
      apply(MULTIPLY, *constr[k], uniq_col, *constr[k]);
    } // for j
  } // for i

  printf("Building solutions\n");
  fflush(stdout);
  if (LOG) LOG->newPhase(f, "Building solutions");
  intersect(constr, N);
  assert(constr[0]);
  dd_edge* solutions = constr[0];
  constr[0] = 0;
  watch.note_time();

  printf("Elapsed time: %lf seconds\n", watch.get_last_interval()/1000000.0);

  printf("Cleanup\n");
  if (LOG) LOG->newPhase(f, "Cardinality");
  for (int i=0; i<N; i++) {
    delete col[i];
    delete dgp[i];
    delete dgm[i];
    delete constr[i];
  }
  delete[] col;
  delete[] dgp;
  delete[] dgm;
  delete[] constr;

  printf("Done.\n\n");

  printf("Set of solutions requires %d nodes\n", solutions->getNodeCount());
  printf("Forest stats:\n");
  FILE_output myout(stdout);
  f->reportStats(myout, "\t", 
    expert_forest::HUMAN_READABLE_MEMORY  |
    expert_forest::BASIC_STATS | expert_forest::EXTRA_STATS |
    expert_forest::STORAGE_STATS | expert_forest::HOLE_MANAGER_STATS
  );

  long c;
  apply(CARDINALITY, *solutions, c);
  printf("\nThere are %ld solutions to the %d-queens problem\n\n", c, N);

  enumerator first(*solutions);
#ifdef SHOW_ALL_SOLUTIONS
  c = 0;
  for (; first; ++first) {
    const int* minterm = first.getAssignments();
    c++;
    printf("Solution %6d: [%d", c, minterm[1]);
    for (int i=2; i<=N; i++) printf(", %d", minterm[i]);
    printf("]\n");
  }
#else
  // show one of the solutions
  if (first) {
    const int* minterm = first.getAssignments();
    printf("One solution:\n");
    for (int i=1; i<=N; i++) {
      printf("\tQueen for row %2d in column %2d\n", i, minterm[i]+1);
    }
  }
#endif
  delete solutions;
  operation::showAllComputeTables(myout, 3);
  if (LOG) {
    LOG->newPhase(f, "Cleanup");
    destroyDomain(d);
  }
  cleanup();
  delete LOG;
  return 0;
}
