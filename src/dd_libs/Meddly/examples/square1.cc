
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
    Program to analyze the "Square 1" puzzle.

    Organized as follows.
    Top has 12 slots, but some pieces take up two consecutive slots.
    Bottom is the same way.
    We can do an "exchange" twist if there are no double-wide pieces
    in the critical slots.
*/

#include "meddly.h"
#include "meddly_expert.h"
#include "timer.h"

// #define DEBUG_EVENTS
// #define DUMP_NSF

using namespace MEDDLY;

FILE_output meddlyout(stdout);

const int Vars = 25;

/*
    Set the variable number for each type of piece
*/
const int  wideLeft[8] = { 2, 5,  8, 11, 14, 17, 20, 23 };
const int wideRight[8] = { 3, 6,  9, 12, 15, 18, 21, 24 };
const int    narrow[8] = { 4, 7, 10, 13, 16, 19, 22, 25 };

/*
  For each variable number, what is it.
*/
bool isWideLeft[Vars+1];
bool isWideRight[Vars+1];
bool isNarrow[Vars+1];


void rotateMove(const int* delta, int N, dd_edge &answer)
{
  expert_forest* EF = (expert_forest*) answer.getForest();
  int K = Vars;

  /*
    Do the same thing at every level with size N:
      change position i to position delta[i],
      for i in {0..N-1}.
  */
  node_handle bottom = EF->handleForValue(1);

  for (int k=1; k<=K; k++) {
    if (EF->getLevelSize(k) != N) {
      // must be the exchange level
      continue;
    }

    unpacked_node* nk = unpacked_node::newFull(EF, k, N);

    for (int i=0; i<N; i++) {
      if (delta[i] < 0) {
        nk->d_ref(i) = EF->handleForValue(0);
      } else {
        unpacked_node* nkp = unpacked_node::newSparse(EF, -k, 1);
        nkp->i_ref(0) = delta[i];
        nkp->d_ref(0) = EF->linkNode(bottom);
        nk->d_ref(i) = EF->createReducedNode(i, nkp);
      }
    } // for i

    EF->unlinkNode(bottom);
    bottom = EF->createReducedNode(-1, nk);
  } // for k

  answer.set(bottom, 0);
}


void exchangeMove(bool left, dd_edge &answer)
{
  expert_forest* EF = (expert_forest*) answer.getForest();
  int K = EF->getNumVariables();

  node_handle bottom = EF->handleForValue(1);

  for (int k=1; k<=K; k++) {
    if (EF->getLevelSize(k) == 2) {
      // the exchange level
      unpacked_node* nk = unpacked_node::newFull(EF, k, 2);
      unpacked_node* nkp = unpacked_node::newSparse(EF, -k, 1);
      nkp->i_ref(0) = 1;
      nkp->d_ref(0) = EF->linkNode(bottom);
      nk->d_ref(0) = EF->createReducedNode(0, nkp);
      nkp = unpacked_node::newSparse(EF, -k, 1);
      nkp->i_ref(0) = 0;
      nkp->d_ref(0) = EF->linkNode(bottom);
      nk->d_ref(1) = EF->createReducedNode(1, nkp);
      EF->unlinkNode(bottom);
      bottom = EF->createReducedNode(-1, nk);
      continue;
    }

    unpacked_node* nk = unpacked_node::newFull(EF, k, 24);
    for (int i=0; i<24; i++) {
      if (isWideLeft[k] && 0==i%6) {
        // can't rotate, wide piece is in the way
        nk->d_ref(i) = 0;
        continue;
      }
      
      if ((i%12>5) == left) {
        // exchange this piece
        // i -> (i+12)%24
        unpacked_node* nkp = unpacked_node::newSparse(EF, -k, 1);
        nkp->i_ref(0) = (i+12)%24;
        nkp->d_ref(0) = EF->linkNode(bottom);
        nk->d_ref(i) = EF->createReducedNode(-1, nkp);
      } else {
        // this piece doesn't move
        nk->d_ref(i) = EF->linkNode(bottom);
      }

    } // for i
    EF->unlinkNode(bottom);
    bottom = EF->createReducedNode(-1, nk);
  } //for k

  answer.set(bottom, 0);
}


/*
  Builds next-state function for square1
*/
void Square1NSF(dd_edge &answer)
{
  dd_edge topl(answer), topr(answer), botl(answer), botr(answer), 
          exchl(answer), exchr(answer);
  int rot[24];

  printf("Building event: rotate top by %d\n", 1);
  fflush(stdout);
  for (int i=0; i<12; i++) rot[i] = (i+1)%12;
  for (int i=12; i<24; i++) rot[i] = i;
  rotateMove(rot, 24, topl);

#ifdef DEBUG_EVENTS
  topl.show(meddlyout, 2);
  meddlyout.flush();
#endif

  printf("Building event: rotate top by %d\n", -1);
  fflush(stdout);
  for (int i=0; i<12; i++) rot[i] = (i+11)%12;
  rotateMove(rot, 24, topr);

#ifdef DEBUG_EVENTS
  topr.show(meddlyout, 2);
  meddlyout.flush();
#endif

  printf("Building event: rotate bottom by %d\n", 1);
  fflush(stdout);
  for (int i=0; i<12; i++) rot[i] = i;
  for (int i=12; i<24; i++) rot[i] = (i+1)%12+12;
  rotateMove(rot, 24, botl);

#ifdef DEBUG_EVENTS
  botl.show(meddlyout, 2);
  meddlyout.flush();
#endif

  printf("Building event: rotate bottom by %d\n", -1);
  fflush(stdout);
  for (int i=12; i<24; i++) rot[i] = (i+11)%12+12;
  rotateMove(rot, 24, botr);
#ifdef DEBUG_EVENTS
  botr.show(meddlyout, 2);
  meddlyout.flush();
#endif

  printf("Building event: exchange left\n");
  fflush(stdout);
  exchangeMove(true, exchl);
#ifdef DEBUG_EVENTS
  exchl.show(meddlyout, 2);
  meddlyout.flush();
#endif

  printf("Building event: exchange right\n");
  fflush(stdout);
  exchangeMove(false, exchr);
#ifdef DEBUG_EVENTS
  exchl.show(meddlyout, 2);
  meddlyout.flush();
#endif

  printf("Accumulating events...");
  fflush(stdout);

  answer = topl + topr + botl + botr + exchl + exchr;

  printf("\n");
  fflush(stdout);

#ifdef DUMP_NSF
  printf("Overall NSF:\n");
  answer.show(meddlyout, 2);
  meddlyout.flush();
#endif
}


void printStats(const char* who, const forest* f)
{
  printf("%s stats:\n", who);
  const expert_forest* ef = (expert_forest*) f;
  ef->reportStats(meddlyout, "\t",
    expert_forest::HUMAN_READABLE_MEMORY  |
    expert_forest::BASIC_STATS | expert_forest::EXTRA_STATS |
    expert_forest::STORAGE_STATS | expert_forest::HOLE_MANAGER_STATS
  );
}


int main()
{
  /*
    Build the variable arrays
  */
  for (int i=0; i<=Vars; i++) {
    isWideLeft[i] = isWideRight[i] = isNarrow[i] = false;
  }
  for (int i=0; i<8; i++) {
    isWideLeft[wideLeft[i]] = true;
    isWideRight[wideRight[i]] = true;
    isNarrow[narrow[i]] = true;
  }

  MEDDLY::initialize();

  //
  // Build the domain
  //
  int scratch[1+Vars];
  for (int i=0; i<=Vars; i++) scratch[i] = 0;
  for (int i=1; i<=Vars; i++) {
    int iwl = isWideLeft[i];
    int iwr = isWideRight[i];
    int isn = isNarrow[i];
    if (iwl + iwr + isn > 1) {
      fprintf(stderr, "Error - variable %d duplicated\n", i);
      return 1;
    }
    if (iwl + iwr + isn == 0) {
      // exchanged or not
      scratch[i-1] = 2;
    } else {
      scratch[i-1] = 24;  // total number of positions for each piece
    }
  } // for i

  domain* D = createDomainBottomUp(scratch, Vars);
 
  //
  // Build NSF for possible "1-step" moves
  //
  timer watch;
  forest* mxd = D->createForest(true, forest::BOOLEAN, forest::MULTI_TERMINAL);
  dd_edge nsf(mxd);
  watch.note_time();
  Square1NSF(nsf);
  watch.note_time();
  printf("Next-state function construction took %.4f seconds\n",
          watch.get_last_interval()/1000000.0);
  printf("Next-state function MxD has\n\t%d nodes\n\t\%d edges\n",
    nsf.getNodeCount(), nsf.getEdgeCount());
  printStats("MxD", mxd);

  //
  // Build initial configuration
  //
  forest::policies p(true);
//  p.setPessimistic();
  forest* mdd = D->createForest(false, forest::BOOLEAN, forest::MULTI_TERMINAL);
  dd_edge initial(mdd);
  int perm = 0;
  for (int i=1; i<=Vars; i++) {
    if (isWideLeft[i] | isWideRight[i] | isNarrow[i]) {
      scratch[i] = perm;
      perm++;
    } else {
      scratch[i] = 0;
    }
  }
  const int* foo = scratch;
  mdd->createEdge(&foo, 1, initial);

  //
  // Saturate!
  //
  printf("Building reachability set using saturation\n");
  fflush(stdout);
  dd_edge reachable(mdd);
  watch.note_time();
  apply(REACHABLE_STATES_DFS, initial, nsf, reachable);
  watch.note_time();
  printf("Reachability set construction took %.4f seconds\n",
          watch.get_last_interval()/1000000.0);
  printf("Reachability set MDD has\n\t%d nodes\n\t\%d edges\n",
    nsf.getNodeCount(), nsf.getEdgeCount());
  printStats("MDD", mdd);
  operation::showAllComputeTables(meddlyout, 3);
  meddlyout.flush();

#ifdef DUMP_REACHABLE
  printf("Reachable states:\n");
  reachable.show(meddlyout, 2);
#endif

  //
  // Determine cardinality
  //
  double c;
  apply(CARDINALITY, reachable, c);
  printf("Counted (approx) %g reachable states\n", c);

  //
  // Cleanup
  //
  MEDDLY::cleanup();
  return 0;
}
