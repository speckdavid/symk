
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
 * testmddlib.cc
 *
 * This is stub to test the simple (or high-level) MDD interface.
 */


#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <meddly.h>
#include <meddly_expert.h>

using namespace MEDDLY;

/**
 * Model: A simple service counter
 *
 * The queue can be atmost of length 2.
 * The service counter is either occupied or not.
 * Once a job enters the queue, it can only exit via the service counter.
 *
 * Two variables: Q {0, 1, 2}, C {0, 1}
 * State: (Q, C)
 * Initial state: (0, 0)
 *
 * Transitions:
 *
 * Adding to queue:
 * (0, x) -> (1, x)
 * (1, x) -> (2, x)
 *
 * Moving from queue to service counter:
 * (1, 0) -> (0, 1)
 * (2, 0) -> (1, 1)
 *
 * Exiting from service counter:
 * (x, 1) -> (x, 0)
 */

int main(int argc, char *argv[])
{
  FILE_output meddlyout(stdout);

  initialize();
  // Create a domain
  domain *d = createDomain();
  const int N = 2;
  const int bounds[N] = {4, 2};
  // d->createVariablesTopDown(bounds, N);
  d->createVariablesBottomUp(bounds, N);

  // Create an MDD forest in this domain (to store states)
  forest* states = d->createForest(false, forest::BOOLEAN,
      forest::MULTI_TERMINAL, forest::policies(false));

#if 1
  printf("Constructing initial set of states\n");
#if 1
  // Create an edge in MDD forest
  int** v = (int **) malloc(2 * sizeof(int*));
  v[0] = (int *) malloc((N+1) * sizeof(int));
  v[0][0] = 0; v[0][1] = 0; v[0][2] = 0;
  v[1] = (int *) malloc((N+1) * sizeof(int));
  v[1][0] = 0; v[1][1] = 1; v[1][2] = 0;
  dd_edge initial_state(states);
  states->createEdge(v, 1, initial_state);
  initial_state.show(meddlyout, 2);
  // states->showInfo(meddlyout);
  // initial_state.clear();
  // initial_state.show(meddlyout, 2);
  // states->showInfo(meddlyout);
#else
  // Create an edge in MDD forest
  int** v = (int **) malloc(1 * sizeof(int*));
  v[0] = (int *) malloc((N+1) * sizeof(int));

  v[0][0] = 0; v[0][1] = 0; v[0][2] = 0;
  dd_edge stateA(states);
  states->createEdge(v, 1, stateA);
  stateA.show(meddlyout, 2);

  dd_edge stateD = stateA;
  stateD.show(meddlyout, 2);
  states->showInfo(meddlyout);

  v[0][0] = 0; v[0][1] = 1; v[0][2] = 0;
  dd_edge stateB(states);
  states->createEdge(v, 1, stateB);
  stateB.show(meddlyout, 2);

  stateD += stateB;
  stateD.show(meddlyout, 2);
  states->showInfo(meddlyout);

  v[0][0] = 0; v[0][1] = 2; v[0][2] = 0;
  dd_edge stateC(states);
  states->createEdge(v, 1, stateC);
  stateC.show(meddlyout, 2);

  stateD += stateC;
  stateD.show(meddlyout, 2);
  states->showInfo(meddlyout);

  stateA.clear();
  stateB.clear();
  stateC.clear();
  stateD.clear();
  states->showInfo(meddlyout);
#endif
#endif

  // Create another edge in MDD forest
  // Union (+) the two edges
  // Intersect (*) the two edges

  // Create a MXD forest in domain (to store transition diagrams)
  forest* transitions = d->createForest(true, forest::BOOLEAN,
      forest::MULTI_TERMINAL, forest::policies(true));

  // Construct a transition diagram in the MXD forest (using +, *)
  // Note: x here denotes "value does not change"
  // (0, x) -> (1, x)
  // (1, x) -> (2, x)
  // (1, 0) -> (0, 1)
  // (2, 0) -> (1, 1)
  // (x, 1) -> (x, 0)
  const int num_of_transitions = 5;
  int** vlist = (int **) malloc(num_of_transitions * sizeof(int*));
  int** vplist = (int **) malloc(num_of_transitions * sizeof(int*));
  for (int i = 0; i < num_of_transitions; ++i)
  {
    vlist[i] = (int *) malloc((N+1) * sizeof(int));
    vplist[i] = (int *) malloc((N+1) * sizeof(int));
  }
  vlist[0][0] = 0; vlist[0][1] = 0; vlist[0][2] = -2;
  vplist[0][0] = 0; vplist[0][1] = 1; vplist[0][2] = -2;

  vlist[0][0] = 0; vlist[1][1] = 1; vlist[1][2] = -2;
  vplist[0][0] = 0; vplist[1][1] = 2; vplist[1][2] = -2;
  
  vlist[0][0] = 0; vlist[2][1] = 1; vlist[2][2] = 0;
  vplist[0][0] = 0; vplist[2][1] = 0; vplist[2][2] = 1;
  
  vlist[0][0] = 0; vlist[3][1] = 2; vlist[3][2] = 0;
  vplist[0][0] = 0; vplist[3][1] = 1; vplist[3][2] = 1;
  
  vlist[0][0] = 0; vlist[4][1] = -2; vlist[4][2] = 1;
  vplist[0][0] = 0; vplist[4][1] = -2; vplist[4][2] = 0;

  // Create a edge representing the model's transition diagram
  dd_edge xd(transitions);
  printf("Constructing transition diagram\n");
  transitions->createEdge(vlist, vplist, num_of_transitions, xd);
  xd.show(meddlyout, 2);
  // transitions->showInfo(meddlyout);

  printf("\nCompute Table:\n");
  operation::showMonolithicComputeTable(meddlyout, true);

  dd_edge reachableStates(initial_state);
  dd_edge prevReachableStates(states);
  dd_edge postImage(states);

  while(prevReachableStates != reachableStates)
  {
    prevReachableStates = reachableStates;
    printf("\nPost-Image (mdd:%ld, mxd:%ld): ",
        long(reachableStates.getNode()), long(xd.getNode()));
    apply(POST_IMAGE, reachableStates, xd, postImage);
    printf("%ld\n", long(postImage.getNode()));
    // postImage.show(meddlyout, 2);
    printf("\nUnion (mdd:%ld, mdd:%ld): ",
        long(reachableStates.getNode()), long(postImage.getNode()));
    apply(UNION, reachableStates, postImage,
        reachableStates);
    printf("%ld\n", long(reachableStates.getNode()));
  }
  reachableStates.show(meddlyout, 2);

#if 0
  // Do PreImage
  // Do PostImage
  compute_manager* cm = getComputeManager();
  dd_edge post_image(states);
  cm->apply(POST_IMAGE, initial_state, xd, post_image);

  // Do Saturation
  dd_edge reachable_states(states);
  cm->apply(ReachableStatesDFS, initial_state, xd, reachable_states);

  // Count the number of reachable states
  long cardinality = 0;
  cm->apply(Cardinality, reachable_states, cardinality);
  printf("Number of reachable states: %d\n", cardinality);
#endif

  // Cleanup; in this case simply delete the domain
  destroyDomain(d);
  cleanup();

  return 0;
}
