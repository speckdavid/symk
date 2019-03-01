
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

#include "meddly.h"
#include "simple_model.h"

#include "kan_rs1.h"
#include "kan_rs2.h"
#include "kan_rs3.h"

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

// 160 states for N=1
long expected[] = { 
  1, 160, 4600, 58400, 454475, 2546432, 11261376, 
  41644800, 133865325, 384392800, 1005927208 
};

using namespace MEDDLY;

dd_edge buildReachset(domain* d, int N)
{
  // Build initial state
  int* initial = new int[17];
  for (int i=16; i; i--) initial[i] = 0;
  initial[1] = initial[5] = initial[9] = initial[13] = N;
  forest* mdd = d->createForest(0, forest::BOOLEAN, forest::MULTI_TERMINAL);
  dd_edge init_state(mdd);
  mdd->createEdge(&initial, 1, init_state);
  delete[] initial;

  // Build next-state function
  forest* mxd = d->createForest(1, forest::BOOLEAN, forest::MULTI_TERMINAL);
  dd_edge nsf(mxd);
  buildNextStateFunction(kanban, 16, mxd, nsf); 

  dd_edge reachable(mdd);
  apply(REACHABLE_STATES_DFS, init_state, nsf, reachable);
  
  return reachable;
}

bool matches(const char* mark, const int* minterm, int np)
{
  for (int i=0; i<np; i++) 
    if (mark[i]-48 != minterm[i]) return false;
  return true;
}

long checkRS(int N, const char* rs[]) 
{
  int sizes[16];

  for (int i=15; i>=0; i--) sizes[i] = N+1;
  domain* d = createDomainBottomUp(sizes, 16);

  dd_edge reachable = buildReachset(d, N);

  // enumerate states
  long c = 0;
  for (enumerator i(reachable); i; ++i) {
    if (c>=expected[N]) {
      c++;
      break;
    }
    if (!matches(rs[c], i.getAssignments()+1, 16)) {
      fprintf(stderr, "Marking %ld mismatched\n", c);
      break;
    }
    c++;
  }

  destroyDomain(d);
  return c;
}

void showRS(int N)
{
  int sizes[16];

  for (int i=15; i>=0; i--) sizes[i] = N+1;
  domain* d = createDomainBottomUp(sizes, 16);

  dd_edge reachable = buildReachset(d, N);

  // enumerate states
  long c = 0;
  for (enumerator i(reachable); i; ++i) {
    const int* minterm = i.getAssignments();
    printf("%5ld: %d", c, minterm[1]);
    for (int l=2; l<=16; l++) printf(", %d", minterm[l]);
    printf("\n");
    c++;
  }

  destroyDomain(d);
}

int main()
{
  MEDDLY::initialize();

  long c;

  printf("Checking Kanban reachability set, N=1\n");
  c = checkRS(1, kanban_rs1);
  if (c != expected[1]) return 1;
  printf("\t%ld markings checked out\n", c);

  printf("Checking Kanban reachability set, N=2\n");
  c = checkRS(2, kanban_rs2);
  if (c != expected[2]) return 1;
  printf("\t%ld markings checked out\n", c);

  printf("Checking Kanban reachability set, N=3\n");
  c = checkRS(3, kanban_rs3);
  if (c != expected[3]) return 1;
  printf("\t%ld markings checked out\n", c);

  MEDDLY::cleanup();
  printf("Done\n");
  return 0;
}

