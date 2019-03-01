
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
  Testing Saturation.

  Test1: Test fully reduced MDDs with Saturation.
*/

#include "meddly.h"
#include <cstdlib>

#define VERBOSE

using namespace MEDDLY;

int main(int argc, char *argv[])
{
  // Initialize MEDDLY
  MEDDLY::initialize();

  // Number of levels in domain (excluding terminals)
  const int nLevels = 3;

  // Set up arrays bounds
  variable** vars = (variable**) malloc((1+nLevels) * sizeof(void*));
  vars[0] = 0;
  for (int i = nLevels; i; i--) { vars[i] = createVariable(2, 0); }

  // Create a domain and set up the state variables.
  domain *d = createDomain(vars, nLevels);
  assert(d != NULL);

  // Create an MDD forest in this domain (to store states)
  forest::policies pmdd(false);
  // pmdd.setQuasiReduced();
  pmdd.setFullyReduced();
  forest* mdd = 
    d->createForest(false, forest::BOOLEAN, forest::MULTI_TERMINAL, pmdd);
  assert(mdd != NULL);

  // Create a MXD forest in domain (to store transition diagrams)
  forest* mxd = d->createForest(true, forest::BOOLEAN, forest::MULTI_TERMINAL);
  assert(mxd != NULL);

  // Set up initial state array
  int state1[nLevels+1] = {0, 0, DONT_CARE, 0};
  int *states[1] = { state1 };
  dd_edge initialStates(mdd);
  mdd->createEdge(reinterpret_cast<int**>(states), 1, initialStates);

  // Create a matrix diagram to represent the next-state function
  int from1[nLevels+1] = {0, 0, DONT_CARE, DONT_CARE};
  int from2[nLevels+1] = {0, DONT_CARE, DONT_CARE, 0};
  int to1[nLevels+1] = {0, 1, 1, DONT_CHANGE};
  int to2[nLevels+1] = {0, DONT_CHANGE, DONT_CHANGE, DONT_CARE};
  int *from[2] = { from1, from2 };
  int *to[2] = { to1, to2 };
  dd_edge nsf(mxd);
  mxd->createEdge(reinterpret_cast<int**>(from), 
      reinterpret_cast<int**>(to), 2, nsf);

  dd_edge reachBFS(initialStates);
  dd_edge reachDFS(initialStates);

  apply(REACHABLE_STATES_BFS, reachBFS, nsf, reachBFS);
  apply(REACHABLE_STATES_DFS, reachDFS, nsf, reachDFS);

  int retval = (reachBFS == reachDFS)? 0: 1;

#ifdef VERBOSE
  FILE_output meddlyout(stdout);

  printf("Initial States:\n");
  initialStates.show(meddlyout, 2);

  printf("Next-State Function:\n");
  nsf.show(meddlyout, 2);

  printf("BFS states\n");
  reachBFS.show(meddlyout, 2);

  printf("DFS states\n");
  reachDFS.show(meddlyout, 2);

  if (retval) {
    printf("\nReachable states DO NOT match\n\n");
  } else {
    printf("\nReachable states match\n\n");
  }

#endif

  // Cleanup
  MEDDLY::cleanup();
  return retval;
}



