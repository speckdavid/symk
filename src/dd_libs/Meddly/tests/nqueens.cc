
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
    various N, and checks if the number of solutions matches
    our expectations.
*/

#include "meddly.h"

using namespace MEDDLY;

const int N_LOW  = 1;
const int N_HIGH = 12;
const long solutions[] = { 
  0, 1, 0, 0, 2, 10, 4, 40, 92, 352, 724, 2680, 14200, 73712, 365596
};

int* scratch;

#define LINEAR_INTERSECTIONS
#ifdef  LINEAR_INTERSECTIONS

void intersect(dd_edge** A, int L)
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
    fflush(stderr);
    if (A[i]) {
      apply(MULTIPLY, *A[0], *A[i], *A[0]);
      delete A[i];
      A[i] = 0;
    }
  }
  fprintf(stderr, "\n");
  fflush(stderr);
}

#else 

void intersect(dd_edge** A, int L)
{
  while (L>1) {
    printf("\t%2d terms to combine ", L);
    // combine adjacent pairs
    for (int i=0; i<L; i+=2) {
      if (A[i] && A[i+1]) {
        apply(MULTIPLY, *A[i], *A[i+1], *A[i]);
        delete A[i+1];
        A[i+1] = 0;
        printf(".");
        fflush(stdout);
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

#endif


void createQueenNodes(forest* f, int q, int N, dd_edge &col, dd_edge &cp, dd_edge &cm)
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

long buildQueenSolutions(int N)
{
  printf("Building solutions for %d", N);
  fflush(stdout);

  for (int i=0; i<N; i++) {
    scratch[i] = N;
  }
  domain* d = createDomainBottomUp(scratch, N);
  assert(d);
  forest::policies p(false);
  p.setPessimistic();
  forest* f = 
    d->createForest(false, forest::INTEGER, forest::MULTI_TERMINAL, p);
  assert(f);

  printf(" q");
  fflush(stdout);

  dd_edge** col = new dd_edge*[N];
  dd_edge** dgp = new dd_edge*[N];
  dd_edge** dgm = new dd_edge*[N];
  dd_edge** constr = new dd_edge*[N+1];

  for (int i=0; i<N; i++) {
    col[i] = new dd_edge(f);
    dgp[i] = new dd_edge(f);
    dgm[i] = new dd_edge(f);
    createQueenNodes(f, i+1, N, *col[i], *dgp[i], *dgm[i]);
    constr[i] = new dd_edge(f);
    f->createEdge(int(1), *constr[i]);
  }
  constr[N] = 0;

  printf("ue");
  fflush(stdout);

  for (int i=0; i<N-1; i++) {
  //  printf("\tBuilding queen %2d constraints\n", i+1);
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

  printf("ens\n");
  fflush(stdout);

  intersect(constr, N);
  assert(constr[0]);
  long c;
  apply(CARDINALITY, *constr[0], c);
  // cleanup
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
  destroyForest(f);
  destroyDomain(d);
  return c;
}

int main()
{
  initialize();
  scratch = new int[N_HIGH+1];

  for (int i=N_LOW; i<=N_HIGH; i++) {
    long sols = buildQueenSolutions(i);
    if (sols != solutions[i]) {
      printf("%d queens, expected %ld solutions, obtained %ld\n", 
              i, solutions[i], sols);
      return 1;
    }
    printf("%d queens has %ld solutions\n", i, sols);
  }
  cleanup();
  return 0;
}
