
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
    State space generation for a small, simple model (Kanban, N=1).
    The model is described in the article

    A. Miner and G. Ciardo.  "Efficient reachability set generation 
    and storage using decision diagrams", in Proc. ICATPN 1999, LNCS 1639,
    pp. 6-25.

    The model has 4 components.  Each component can be in state {W, M, B, G}
    where
    W:	the machine is waiting for a part
    M:	the machine is processing a part
    B:	a bad part was produced
    G:	a good part was produced

    The 4 machines are connected, so that output of machine 1 is used for
    machines 2 and 3, and output of machines 2 and 3 is used for machine 4.

    Machine 1 can change state locally as:
    
    W -> M
    M -> B
    B -> M
    M -> G

    Machines 2 and 3 can change state locally as:
    
    M -> B
    B -> M
    M -> G

    Machine 4 can change state locally as:
    
    M -> B
    B -> M
    M -> G
    G -> W

    The synchronization between machines 1,2,3 is:
    
    G1 -> W1
    W2 -> M2
    W3 -> M3

    The synchronization between machines 2,3,4 is:
    
    G2 -> W2
    G3 -> W3
    W4 -> M4

    Initially, all machines are in local state "W".
    There are 160 reachable states.  All combinations are possible, EXCEPT
    that machines 2 and 3 are either BOTH in state W, or BOTH NOT in state W.

    TODO in the library:

   	Write the function 
	AddMatrixElementAtLevel(dd_tempedge, level lh, int from, int to, bool term)

*/


/* Probably we need a makefile and a mechanism to specify
   the location of these header files...
*/

#include <cstdio>
#include <domain.h>
#include <forest.h>
#include <dd_edge.h>
#include <ophandle.h>

level variables[5];
int sizes[5] = { 0, 4, 4, 4, 4 };
forest_hndl relation;

void CheckVars()
{
  /* Sanity check */
  int i;
  for (i=1; i<5; i++) 
    if ((variables[i] > 4) || (variables[i] < 1)) {
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

dd_edge* MakeLocalTransitions(int machine)
{
  dd_tempedge* temp;
  temp = CreateTempEdge(relation, NULL);
  if (1==machine) {
    /* W -> M */
    Try( AddMatrixElementAtLevel(temp, variables[machine], 0, 1, true) );
  }
  /* M -> B */
  Try( AddMatrixElementAtLevel(temp, variables[machine], 1, 2, true) );
  /* B -> M */
  Try( AddMatrixElementAtLevel(temp, variables[machine], 2, 1, true) );
  /* M -> G */
  Try( AddMatrixElementAtLevel(temp, variables[machine], 1, 3, true) );
  if (4==machine) {
    /* G -> W */
    Try( AddMatrixElementAtLevel(temp, variables[machine], 3, 0, true) );
  }
  return CreateEdge(temp);
}

dd_edge* MakeSynch1_23()
{
  dd_tempedge* temp;
  const int sz = 5;
  int from[sz], to[sz];
  temp = CreateTempEdge(relation, NULL);
  /* G1 -> W1 */
  from[variables[1]] = 3; to[variables[1]] = 0;
  /* W2 -> M2 */
  from[variables[2]] = 0; to[variables[2]] = 1;
  /* W3 -> M3 */
  from[variables[3]] = 0; to[variables[3]] = 1;
  /* Don't touch machine 4 */
  from[variables[4]] = to[variables[4]] = -1;  // Don't Care
  AddMatrixElement(temp, from, to, sz, true);
  /*
  for (int i=0; i<sizes[variables[4]]; i++) {  // size of level variables[4]
    from[variables[4]] = to[variables[4]] = i;
    AddMatrixElement(temp, from, to, sz, true);
  }
  */
  return CreateEdge(temp);
}

dd_edge* MakeSynch23_4()
{
  dd_tempedge* temp;
  const int sz = 5;
  int from[sz], to[sz];
  temp = CreateTempEdge(relation, NULL);
  /* G2 -> W2 */
  from[variables[2]] = 3; to[variables[2]] = 0;
  /* G3 -> W3 */
  from[variables[3]] = 3; to[variables[3]] = 0;
  /* W4 -> M4 */
  from[variables[4]] = 0; to[variables[4]] = 1;
  /* Don't touch machine 1 */
  from[variables[1]] = to[variables[1]] = -1;  // Don't Care
  AddMatrixElement(temp, from, to, sz, true);
  /*
  for (int i=0; i<sizes[variables[1]]; i++) {  // size of level variables[1]
    from[variables[1]] = to[variables[1]] = i;
    AddMatrixElement(temp, from, to, sz, true);
  }
  */
  return CreateEdge(temp);
}

int main()
{
  int i;
  domain* d;

  forest_hndl states;

  /* sets of states */
  int initst[5] = { 0, 0, 0, 0, 0 };
  dd_edge* initial;

  /* transition relations */
  dd_edge* local[5];
  dd_edge* synch1_23;
  dd_edge* synch23_4;
  dd_edge* nsf; 

  /* Set up the state variables.
     Use one per "machine", with 4 values each:

W : 0
M : 1
B : 2
G : 3
*/
  d = CreateDomain(4, sizes, variables);

  if (NULL==d) {
    fprintf(stderr, "Couldn't create domain\n");
    return 1;
  }
  CheckVars();

  /* Create forests */
  // states = CreateForest(d, MDD, false, forest::QUASI_REDUCED, FULL_STORAGE);
  states = CreateForest(d, MDD, false, forest::FULLY_REDUCED, FULL_STORAGE);
  // relation = CreateForest(d, MXD, false, forest::QUASI_REDUCED, FULL_STORAGE);
  relation = CreateForest(d, MXD, false, forest::IDENTITY_REDUCED, FULL_STORAGE);

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
  initial = CreateVectorElement(states, initst, 5, true); 

  if (NULL==initial) {
    fprintf(stderr, "Couldn't create set of initial states\n");
    return 1;
  } else {
    fprintf(stderr, "Created set of initial states\n");
  }

  /* Build local transitions */
  local[0] = NULL;
  for (i=1; i<=4; i++) {
    local[i] = MakeLocalTransitions(i);
    fprintf(stderr, "Created Local Transition: %d\n", i);
  }

  printf("\nForest:\n");
  ShowForestNodes(stderr, relation);

  /* Build synchronizing transitions */
  synch1_23 = MakeSynch1_23();
  synch23_4 = MakeSynch23_4();

  /* Build overall next-state function */
  nsf = NULL;
  ApplyBinary(OP_UNION, local[1], local[2], nsf);
  ApplyBinary(OP_UNION, nsf, local[3], nsf);
  ApplyBinary(OP_UNION, nsf, local[4], nsf);
  ApplyBinary(OP_UNION, nsf, synch1_23, nsf);
  ApplyBinary(OP_UNION, nsf, synch23_4, nsf);

  /* print stuff and bail out for now; once this works we can 
     build the set of reachable states */

  printf("Initial states: ");
  ShowDDEdge(stdout, initial);
  printf("\nTransition relation nodes\n");
  for (i=1; i<5; i++) {
    printf("local to component %d: ", i);
    ShowDDEdge(stdout, local[i]);
    printf("\n"); 
  }
  printf("synchronizing 1_23: ");
  ShowDDEdge(stdout, synch1_23);
  printf("\nsynchronizing 23_4: ");
  ShowDDEdge(stdout, synch23_4);
  printf("\nOverall transition relation: ");
  ShowDDEdge(stdout, nsf);

  // Image operations
  dd_edge *curr = initial;
  char cmd = 'Z';
  while (cmd != 'Q') {
    printf("\n\n\
\tSelect operation\n\
\tA\tPost-Image\n\
\ta\tPost-Image U current set-of-states\n\
\tB\tPre-Image\n\
\tb\tPre-Image U current set-of-states\n\
\tR\tReachable states\n\
\tC\tShow current set-of-states\n\
\tN\tShow next-state function\n\
\tQ\tQuit image operations\n\
\tChoice: ");
    fflush(stdout);
    cmd = getc(stdin);
    while (isspace(cmd)) { cmd = getc(stdin); }
    switch (cmd) {
      case 'A':
        PostImage(curr, nsf, curr);
        break;
      case 'a':
        {
          dd_edge *temp = NULL;
          PostImage(curr, nsf, temp);
          ApplyBinary(OP_UNION, curr, temp, curr);
          ReleaseEdge(temp);
        }
        break;
      case 'B':
        PreImage(curr, nsf, curr);
        break;
      case 'b':
        {
          dd_edge *temp = NULL;
          PreImage(curr, nsf, temp);
          ApplyBinary(OP_UNION, curr, temp, curr);
          ReleaseEdge(temp);
        }
        break;
      case 'R':
        Saturate(curr, nsf, curr);
        break;
      case 'C':
        ShowDDEdge(stdout, curr);
        break;
      case 'N':
        ShowDDEdge(stdout, nsf);
        break;
    }
  }
  
  printf("\nDone\n");
  return 0;
}

