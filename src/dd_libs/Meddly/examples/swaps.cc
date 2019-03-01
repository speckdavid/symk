
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
    Simple model of an array of (unique) integers,
    with operations to swap neighboring elements.
*/

#include "meddly.h"
#include "meddly_expert.h"
#include "timer.h"

using namespace MEDDLY;

FILE_output meddlyout(stdout);

// #define DUMP_NSF
// #define DUMP_REACHABLE

// Helpers

inline double factorial(int n)
{
  double f = 1.0;
  for (int i=2; i<=n; i++) {
    f *= i;
  }
  return f;
}

//
//
// Encoding based on the obvious: each state variable is
// one of the array elements.
//
//

/*
    Builds a next-state relation for exchanging two variables.
    The forest must be IDENTITY REDUCED.
*/
void Exchange(int va, int vb, int N, dd_edge &answer)
{
  expert_forest* EF = (expert_forest*) answer.getForest(); 

  /* We're doing this BY HAND which means a 4 levels of nodes */

  unpacked_node* na = unpacked_node::newFull(EF, va, N);
  for (int ia=0; ia<N; ia++) {
    unpacked_node* nap = unpacked_node::newFull(EF, -va, N);
    for (int ja=0; ja<N; ja++) {
      
      // WANT vb == va' and vb' == va, so...

      // Make a singleton for vb' == va (index ia)
      unpacked_node* nbp = unpacked_node::newSparse(EF, -vb, 1);
      nbp->i_ref(0) = ia;
      nbp->d_ref(0) = EF->handleForValue(1);

      // Make a singleton for vb == va' (index ja)
      unpacked_node* nb = unpacked_node::newSparse(EF, vb, 1);
      nb->i_ref(0) = ja;
      nb->d_ref(0) = EF->createReducedNode(ja, nbp);

      nap->d_ref(ja) = EF->createReducedNode(-1, nb);
    } // for ja
    na->d_ref(ia) = EF->createReducedNode(ia, nap);
  } // for ia

  answer.set(EF->createReducedNode(-1, na));
}

/*
    Build monolithic next-state relation, using "array values" 
    state variables.  The forest must be IDENTITY REDUCED.
*/
void ValueNSF(int N, dd_edge &answer)
{
  dd_edge temp(answer);
  answer.set(0);

  for (int i=1; i<N; i++) {
    Exchange(i+1, i, N, temp);
    answer += temp;
  }
}

/*
    Build partitioned next-state relation, using "array values" 
    state variables.  The forest must be IDENTITY REDUCED.
*/
void ValueNSF(int N, satpregen_opname::pregen_relation* nsf)
{
  dd_edge temp(nsf->getRelForest());
  for (int i=1; i<N; i++) {
    Exchange(i+1, i, N, temp);
    nsf->addToRelation(temp);
  }
}

//
//
// Alternate encoding: each state variable is one of the unique integers,
// and we store its position in the array.
//
//

/*
    Builds a next-state relation for exchanging two positions.
    The forest must be IDENTITY REDUCED.
*/
void AltExchange(int pa, int pb, int N, int K, dd_edge &answer)
{
  expert_forest* EF = (expert_forest*) answer.getForest(); 

  /*
      Do the same thing at every level:
        if the position is pa, change it to pb.
        if the position is pb, change it to pa.
        otherwise, no change.
  */
  node_handle bottom = EF->handleForValue(1);

  for (int k=1; k<=K; k++) {
    unpacked_node* nk = unpacked_node::newFull(EF, k, N);

    for (int i=0; i<N; i++) {
      if (pa == i) {
        unpacked_node* nkp = unpacked_node::newSparse(EF, -k, 1);
        nkp->i_ref(0) = pb;
        nkp->d_ref(0) = bottom;
        nk->d_ref(i) = EF->createReducedNode(i, nkp);
        continue;
      }
      if (pb == i) {
        unpacked_node* nkp = unpacked_node::newSparse(EF, -k, 1);
        nkp->i_ref(0) = pa;
        nkp->d_ref(0) = bottom;
        nk->d_ref(i) = EF->createReducedNode(i, nkp);
        continue;
      }
      nk->d_ref(i) = bottom;
    } // for i

    bottom = EF->createReducedNode(-1, nk);
  }

  answer.set(bottom);
}

/*
    Build monolithic next-state relation, using "array positions" 
    state variables.  The forest must be IDENTITY REDUCED.
*/
void PositionNSF(int N, dd_edge &answer)
{
  dd_edge temp(answer);
  answer.set(0);

  for (int i=1; i<N; i++) {
    AltExchange(i-1, i, N, N, temp);
    answer += temp;
  }
}

/*
    Build partitioned next-state relation, using "array positions" 
    state variables.  The forest must be IDENTITY REDUCED.
*/
void PositionNSF(int N, satpregen_opname::pregen_relation* nsf)
{
  dd_edge temp(nsf->getRelForest());
  for (int i=1; i<N; i++) {
    AltExchange(i-1, i, N, N, temp);
    nsf->addToRelation(temp);
  }
}


//
//
// I/O crud
//
//

void printStats(const char* who, const forest* f)
{
  printf("%s stats:\n", who);
  const expert_forest* ef = (expert_forest*) f;
  ef->reportStats(meddlyout, "\t",
    expert_forest::HUMAN_READABLE_MEMORY  |
    expert_forest::BASIC_STATS | expert_forest::EXTRA_STATS |
    expert_forest::STORAGE_STATS | expert_forest::HOLE_MANAGER_STATS
  );
  meddlyout.flush();
}

int usage(const char* who)
{
  /* Strip leading directory, if any: */
  const char* name = who;
  for (const char* ptr=who; *ptr; ptr++) {
    if ('/' == *ptr) name = ptr+1;
  }
  printf("\nUsage: %s nnnn [options]\n\n", name);
  printf("\tnnnn: array size\n\n");
  printf("\t-bfs: use traditional iterations\n\n");
  printf("\t-dfs: use fastest saturation (currently, -msat)\n");
//  printf("\t-esat: use saturation by events\n");
//  printf("\t-ksat: use saturation by levels\n");
  printf("\t-msat: use monolithic saturation (default)\n\n");
  printf("\t-alt: use alternate description\n");
  return 1;
}

void runWithArgs(int N, char method, bool alternate)
{
  timer watch;

  printf("+-------------------------------------------+\n");
  printf("|   Initializing swaps model for N = %-4d   |\n", N);
  printf("+-------------------------------------------+\n");
  fflush(stdout);

  /*
     Initialize domain
  */
  int* sizes = new int[N];
  for (int i=0; i<N; i++) sizes[i] = N;
  domain* D = createDomainBottomUp(sizes, N);
  delete[] sizes;

  /*
     Build initial state
  */
  int* initial = new int[N+1];
  initial[0] = 0;
  for (int i=1; i<=N; i++) initial[i] = i-1;
  forest::policies p(false);
  forest* mdd = D->createForest(0, forest::BOOLEAN, forest::MULTI_TERMINAL, p);
  dd_edge init_state(mdd);
  mdd->createEdge(&initial, 1, init_state);
  delete[] initial;
  
  /*
     Build next-state function
  */
  forest* mxd = D->createForest(1, forest::BOOLEAN, forest::MULTI_TERMINAL);
  dd_edge nsf(mxd);
  satpregen_opname::pregen_relation* ensf = 0;
  specialized_operation* sat = 0;

  if ('s' == method) {
    ensf = new satpregen_opname::pregen_relation(mdd, mxd, mdd, 16);
  }
  if ('k' == method) {
    ensf = new satpregen_opname::pregen_relation(mdd, mxd, mdd);
  }

  watch.note_time();
  if (ensf) {
    if (alternate) {
      PositionNSF(N, ensf);
    } else {
      ValueNSF(N, ensf);
    }
  } else {
    if (alternate) {
      PositionNSF(N, nsf);
    } else {
      ValueNSF(N, nsf);
    }
  }
  watch.note_time();

  printf("Next-state function construction took %.4f seconds\n",
          watch.get_last_interval()/1000000.0);
  if (0==ensf) {
    printf("Next-state function MxD has\n\t%d nodes\n\t\%d edges\n",
      nsf.getNodeCount(), nsf.getEdgeCount());
  }

#ifdef DUMP_NSF
  printf("Next-state function:\n");
  nsf.show(meddlyout, 2);
#endif

  printStats("MxD", mxd);

  /*
      Build reachable states
  */
  watch.note_time();
  dd_edge reachable(mdd);
  switch (method) {
    case 'b':
        printf("Building reachability set using traditional algorithm\n");
        fflush(stdout);
        apply(REACHABLE_STATES_BFS, init_state, nsf, reachable);
        break;

    case 'm':
        printf("Building reachability set using saturation\n");
        fflush(stdout);
        apply(REACHABLE_STATES_DFS, init_state, nsf, reachable);
        break;

    case 'k':
    case 's':
        printf("Building reachability set using saturation, relation");
        if ('k'==method)  printf(" by levels\n");
        else              printf(" by events\n");
        fflush(stdout);
        if (0==SATURATION_FORWARD) {
          throw error(error::UNKNOWN_OPERATION);
        }
        sat = SATURATION_FORWARD->buildOperation(ensf);
        if (0==sat) {
          throw error(error::INVALID_OPERATION);
        }
        sat->compute(init_state, reachable);
        break;

    default:
        printf("Error - unknown method\n");
        exit(2);
  };
  watch.note_time();
  printf("Reachability set construction took %.4f seconds\n",
          watch.get_last_interval()/1000000.0);
  printf("Reachability set MDD has\n\t%d nodes\n\t\%d edges\n",
    reachable.getNodeCount(), reachable.getEdgeCount());
  fflush(stdout);
//  mdd->garbageCollect();

#ifdef DUMP_REACHABLE
  printf("Reachable states:\n");
  reachable.show(meddlyout, 2);
#endif

  printStats("MDD", mdd);
  operation::showAllComputeTables(meddlyout, 3);
  meddlyout.flush();

  /*
      Determine cardinality
  */
  double c;
  apply(CARDINALITY, reachable, c);
  printf("Counted (approx) %g reachable states\n", c);
  printf("(There should be %g states)\n", factorial(N));
}

int main(int argc, const char** argv)
{
  int N = -1;
  char method = 'm';
  bool alt = false;

  for (int i=1; i<argc; i++) {
    if (strcmp("-bfs", argv[i])==0) {
      method = 'b';
      continue;
    }
    if (strcmp("-dfs", argv[i])==0) {
      method = 'm';
      continue;
    }
    if (strcmp("-esat", argv[i])==0) {
      method = 's';
      continue;
    }
    if (strcmp("-ksat", argv[i])==0) {
      method = 'k';
      continue;
    }
    if (strcmp("-msat", argv[i])==0) {
      method = 'm';
      continue;
    }
    if (strcmp("-alt", argv[i])==0) {
      alt = true;
      continue;
    }
    N = atoi(argv[i]);
  }

  if (N<0) return usage(argv[0]);

  MEDDLY::initialize();

  try {
    runWithArgs(N, method, alt);
    MEDDLY::cleanup();
    return 0;
  }
  catch (MEDDLY::error e) {
    printf("Caught MEDDLY error: %s\n", e.getName());
    return 1;
  }

}

