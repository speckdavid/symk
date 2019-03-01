
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
    Builds the set of solutions to the queen cover problem for 
    user-specified board size NxN.

    In other words, finds all possible ways to put queens onto
    an NxN chessboard so that every square either contains a queen,
    or is attached by a queen.

    State: 
      For each square, does it hold a queen.

    Constraints:
      conjunction over all squares:
        is this square covered

*/

#include <cstdlib>
#include <fstream>

#include "meddly.h"
#include "meddly_expert.h"
#include "loggers.h"
#include "timer.h"

using namespace MEDDLY;

int N;
FILE* outfile;
const char* lfile;

dd_edge** qic;
dd_edge** qidp;
dd_edge** qidm;


forest* buildQueenForest(forest::policies &p)
{
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


  int* vars = new int[N*N];
  for (int i=0; i<N*N; i++) {
    vars[i] = 2;
  }
  domain* d = createDomainBottomUp(vars, N*N);
  assert(d);
  forest* f = 
    d->createForest(false, forest::INTEGER, forest::MULTI_TERMINAL, p);
  assert(f);

  delete[] vars;
  return f;
}

inline int ijmap(int i, int j)
{
  return i*N+j+1;
}

void hasQueen(forest* f, int i, int j, dd_edge &e)
{
  f->createEdgeForVar(ijmap(i, j), false, e);
}

void queenInRow(forest* f, int r, dd_edge &e)
{
  f->createEdge(int(0), e);
  if (r<0 || r>=N) return;
  for (int j=0; j<N; j++) {
    dd_edge tmp(f);
    hasQueen(f, r, j, tmp);
    apply(MAXIMUM, e, tmp, e);
  }
}

void queenInCol(forest* f, int c, dd_edge &e)
{
  if (c>=0 && c<N && qic[c]) {
    e = *qic[c];
    return;
  }
  f->createEdge(int(0), e);
  if (c<0 || c>=N) return;
  for (int i=0; i<N; i++) {
    dd_edge tmp(f);
    hasQueen(f, i, c, tmp);
    apply(MAXIMUM, e, tmp, e);
  }
  qic[c] = new dd_edge(e);
}

void queenInDiagP(forest* f, int d, dd_edge &e)
{
  if (d>=0 && d<2*N-1 && qidp[d]) {
    e = *qidp[d];
    return;
  }
  f->createEdge(int(0), e);
  if (d<0 || d>=2*N-1) return;
  for (int i=0; i<N; i++) for (int j=0; j<N; j++) if (i+j==d) {
    dd_edge tmp(f);
    hasQueen(f, i, j, tmp);
    apply(MAXIMUM, e, tmp, e);
  }
  qidp[d] = new dd_edge(e);
}

void queenInDiagM(forest* f, int d, dd_edge &e)
{
  if (d>-N && d<N && qidm[d+N-1]) {
    e = *qidm[d+N-1];
    return;
  }
  f->createEdge(int(0), e);
  if (d<=-N || d>=N) return;
  for (int i=0; i<N; i++) for (int j=0; j<N; j++) if (i-j==d) {
    dd_edge tmp(f);
    hasQueen(f, i, j, tmp);
    apply(MAXIMUM, e, tmp, e);
  }
  qidm[d+N-1] = new dd_edge(e);
}

bool processArgs(int argc, const char** argv, forest::policies &p)
{
  lfile = 0;
  p.setPessimistic();
  N = -1;
  int i;
  for (i=1; i<argc; i++) {
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
    N = atoi(argv[i]);
    break;
  }
  if (N<1) return false;
  if (++i < argc) {
    outfile = fopen(argv[2], "w");
    if (0==outfile) {
      printf("Couldn't open %s for writing, no solutions will be written\n", argv[2]);
    }
  } else {
    outfile = 0;
  }
  return true;
}

int usage(const char* who)
{
  /* Strip leading directory, if any: */
  const char* name = who;
  for (const char* ptr=who; *ptr; ptr++) {
    if ('/' == *ptr) name = ptr+1;
  }
  printf("Usage: %s <-opt> <-pess> <-l lfile> N <outfile>\n\n", name);
  printf("\t        N:  board dimension\n");
  printf("\t     -opt:  Optimistic node deletion\n");
  printf("\t    -pess:  Pessimistic node deletion (default)\n");
  printf("\t -l lfile:  Write logging information to specified file\n");
  printf("\t<outfile>:  if specified, we write all solutions to this file\n\n");
  return 1;
}

int main(int argc, const char** argv)
{
  initialize();
  forest::policies p(false);
  if (!processArgs(argc, argv, p)) return usage(argv[0]);
  printf("Using %s\n", getLibraryInfo(0));

  timer stopwatch;
  stopwatch.note_time();

  printf("\nDetermining queen covers for %dx%d chessboard.\n", N, N);

  forest* f = buildQueenForest(p);
  
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
      snprintf(comment, 80, "Automatically generated by queen_cover (N=%d)", N);
      LOG->addComment(comment);
      f->setLogger(LOG, "forest name");
      LOG->newPhase(f, "Initializing");
    }
  }

  dd_edge num_queens(f);
  f->createEdge(int(0), num_queens);
  for (int i=0; i<N; i++) for (int j=0; j<N; j++) {
    dd_edge tmp(f);
    hasQueen(f, i, j, tmp);
    apply(PLUS, num_queens, tmp, num_queens);
  } // for i,j

  // "By-hand" compute tables for queen in col, diag
  qic = new dd_edge*[N];
  for (int i=0; i<N; i++) qic[i] = 0;
  qidp = new dd_edge*[2*N-1];
  qidm = new dd_edge*[2*N-1];
  for (int i=0; i<2*N-1; i++) {
    qidp[i] = qidm[i] = 0;
  }

  dd_edge solutions(f);

  int q;
  for (q=1; q<=N; q++) {

    char buffer[80];
    snprintf(buffer, 80, "Trying to cover with %d queens", q);
    printf("\n%s\n", buffer);
    if (LOG) LOG->newPhase(f, buffer);

    // Build all possible placements of q queens:
    dd_edge qqueens(f);
    f->createEdge(q, qqueens);
    apply(EQUAL, qqueens, num_queens, qqueens);
    solutions = qqueens;

    printf("\tAdding constraints by row\n\t\t");
    for (int i=0; i<N; i++) {
      printf("%d", N-i);
      fflush(stdout);
      dd_edge rowcov = qqueens;
      dd_edge qir(f);
      queenInRow(f, i, qir);
      for (int j=0; j<N; j++) {
        dd_edge col(f);
        queenInCol(f, j, col);
        dd_edge dgp(f);
        queenInDiagP(f, i+j, dgp);
        dd_edge dgm(f);
        queenInDiagM(f, i-j, dgm);
        // "OR" together the possible attackers for this square
        apply(MAXIMUM, col, dgp, col);
        apply(MAXIMUM, col, dgm, col);
        apply(MAXIMUM, col, qir, col);
        // "AND" with this row
        apply(MULTIPLY, rowcov, col, rowcov);
      } // for j
      printf(", ");
      fflush(stdout);
      // "AND" directly with set of covers
      apply(MULTIPLY, solutions, rowcov, solutions);
    } // for i

    printf("\n");

    // Any solutions?
    if (0==solutions.getNode()) {
      printf("\tNo solutions\n");
      continue;
    } else {
      printf("\tSuccess\n");
      break;
    }
  } // for q

  // Cleanup
  for (int i=0; i<N; i++) delete qic[i];
  for (int i=0; i<2*N-1; i++) {
    delete qidp[i];
    delete qidm[i];
  }
  delete[] qic;
  delete[] qidp;
  delete[] qidm;

  printf("\n%lg seconds CPU time elapsed\n", 
    stopwatch.get_last_interval() / 1000000.0);
  printf("Forest stats:\n");
  FILE_output myout(stdout);
  expert_forest* ef = (expert_forest*)f;
  ef->reportStats(myout, "\t", 
    expert_forest::HUMAN_READABLE_MEMORY  |
    expert_forest::BASIC_STATS | expert_forest::EXTRA_STATS |
    expert_forest::STORAGE_STATS | expert_forest::HOLE_MANAGER_STATS
  );
  operation::showAllComputeTables(myout, 3);

  long c;
  apply(CARDINALITY, solutions, c);
  printf("\nFor a %dx%d chessboard, ", N, N);
  printf("there are %ld covers with %d queens\n\n", c, q);

  if (outfile) {
    printf("Writing solutions to file %s\n", argv[2]);
  
    fprintf(outfile, "%d # Board dimension\n\n", N);
    // show the solutions
    enumerator iter(solutions);
    long counter;
    for (counter = 1; iter; ++iter, ++counter) {
      fprintf(outfile, "solution %5ld:  ", counter);
      const int* minterm = iter.getAssignments();
      for (int i=0; i<N; i++) for (int j=0; j<N; j++) {
        if (minterm[ijmap(i,j)]) {
          fprintf(outfile, "(%2d, %2d) ", i+1, j+1);
        }
      }
      fprintf(outfile, "\n");
    } // for iter
    fprintf(outfile, "\n");
  }

  if (LOG) LOG->newPhase(f, "Cleanup");
  cleanup();
  delete LOG;
  return 0;
}
