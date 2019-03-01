
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
*/


#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <set>
#include <deque>
#include <vector>

#include "meddly.h"
#include "meddly_expert.h"
#include "timer.h"

using namespace MEDDLY;


// ========= BEGIN Global definitions ===========
//
//

// N = number of machines + 1.
const int N = 5;

// Size of each variable
int sizes[N-1] = { 4, 4, 4, 4 };

// ========= END Global definitions ===========



// ========= BEGIN Reachability Set generation ===========

// Build the initial state used for reachability set generation.
// f: MDD forest.
dd_edge getInitialStates(forest* f);

// Build the next-state function used for reachaility set generation.
// f: MXD forest.
dd_edge getNSF(forest* f);

// Helpers for getNSF()
dd_edge MakeLocalTransitions(int machine, forest* mxd);
dd_edge MakeSynch1_23(forest* mxd);
dd_edge MakeSynch23_4(forest* mxd);

// Builds the reachability set.
// dfs: if true uses saturation, otherwise traditional algorithm
//      for reachability set generation.
dd_edge getReachabilitySet(const dd_edge& states, const dd_edge& nsf, bool dfs);

// ========= END Reachability Set generation ===========



// ========== BEGIN Tests ===========

// Enable/Disable tests using these variables.
const bool doMDDComplement        = true;
const bool doMXDComplement        = true;
const bool doIndexSet             = true;
const bool doIndexSetCardinality  = true;
const bool doFindFirstElement     = false;
const bool doMTMDDIterator        = false;
const bool doMTMXDIterator        = false;

// MDD complement
void testMDDComplement(const dd_edge& mdd);

// MXD complement
void testMXDComplement(const dd_edge& mxd);

// Convertinf MDD to an Index Set (EV+MDD).
void testIndexSet(domain* d, const dd_edge& mdd);

// Iterate over elements of an MDD using
// findFirstElement() and dd_edge::operator-=.
void testFindFirstElement(const dd_edge& mdd);

// Iterate over elements of an MDD using dd_edge::iterator.
void testMTMDDIterator(const dd_edge& mdd);

// Iterate over elements of an MXD using dd_edge::iterator.
void testMTMXDIterator(const dd_edge& mxd);

// ========== END Tests ===========




int main(int argc, char* argv[])
{
  bool dfs = false;
  bool make_gifs = false;
  if (argc > 1) {
    assert(argc <= 3);
    char *cmd = NULL;
    for (int i = 1; i < argc; i++) {
      cmd = argv[i];
      if (strncmp(cmd, "-gif", 6) == 0) make_gifs = true;
      else if (strncmp(cmd, "-dfs", 5) == 0) dfs = true;
      else {
        fprintf(stderr, "Usage: $ kan1_batch [-gif|-dfs]\n");
        fprintf(stderr, "-gif : create gif representing the reachable states\n");
        fprintf(stderr,
            "-dfs : use depth-first algorithm to compute reachable states");
        fprintf(stderr, "\n\n");
        exit(1);
      }
    }
  }
  if (argc > 1) dfs = true;

  initialize();

  // Create a domain and set up the state variables.
  // Use one per "machine", with 4 values each: {W = 0, M, B, G}
  domain *d = createDomainBottomUp(sizes, N-1);
  assert(d != NULL);


  // Create an MDD forest in this domain (to store states)
  forest::policies pmdd(false);
  pmdd.setPessimistic();
  forest* mdd = d->createForest(false, forest::BOOLEAN, 
    forest::MULTI_TERMINAL, pmdd);
  assert(mdd != NULL);

  // Create a MXD forest in domain (to store transition diagrams)
  forest::policies pmxd(true);
  pmxd.setPessimistic();
  forest* mxd = d->createForest(true, forest::BOOLEAN, 
    forest::MULTI_TERMINAL, pmxd);
  assert(mxd != NULL);

  // Set up initial set of states
  dd_edge initialStates = getInitialStates(mdd);

  // Build the Next-State Function (nsf)
  dd_edge nsf = getNSF(mxd);

  // Build Reachability Set
  dd_edge reachableStates = getReachabilitySet(initialStates, nsf, dfs);

  // Various test procedures
  if (doMDDComplement)      testMDDComplement(reachableStates);
  if (doMXDComplement)      testMXDComplement(nsf);
  if (doIndexSet)           testIndexSet(d, reachableStates);
  if (doFindFirstElement)   testFindFirstElement(reachableStates);
  if (doMTMDDIterator)      testMTMDDIterator(reachableStates);
  if (doMTMXDIterator)      testMTMXDIterator(nsf);

  // Cleanup
  destroyDomain(d);
  cleanup();

  fprintf(stderr, "\nDone\n");
  return 0;
}




// ========= BEGIN Reachability Set generation ===========


dd_edge getInitialStates(forest* f)
{
  assert(f);
  dd_edge states(f);

  int element[N];
  int* elements[] = { element };
  memset(element, 0, N * sizeof(int));
  f->createEdge(elements, 1, states);

  return states;
}


dd_edge getNSF(forest* f)
{
  assert(f);
  dd_edge nsf(f);

  // Build and add local transitions to nsf
  for (int i = 1; i <= 4; i++) {
    nsf += MakeLocalTransitions(i, f);
  }

  // Build and add synchronizing transitions to nsf
  nsf += MakeSynch1_23(f);
  nsf += MakeSynch23_4(f);

  return nsf;
}


dd_edge MakeLocalTransitions(int machine, forest* mxd)
{
  dd_edge nsf(mxd);

  int from[N];
  int to[N];
  int* froms[] = { from };
  int* tos[] = { to };

  // Initialize elements (-2 indicates don't care).
  std::fill_n(from, N, -2);
  std::fill_n(to, N, -2);

  // W -> M
  if (1 == machine) {
    // adjust the vector
    from[machine] = 0; to[machine] = 1;

    // add it to nsf
    dd_edge temp(mxd);
    mxd->createEdge(froms, tos, 1, temp);
    nsf += temp;

    // revert the vector
    from[machine] = -2; to[machine] = -2;
  }

  /* M -> B */
  {
    // adjust the vector
    from[machine] = 1; to[machine] = 2;

    // add it to nsf
    dd_edge temp(mxd);
    mxd->createEdge(froms, tos, 1, temp);
    nsf += temp;

    // revert the vector
    from[machine] = -2; to[machine] = -2;
  }

  /* B -> M */
  {
    // adjust the vector
    from[machine] = 2; to[machine] = 1;

    // add it to nsf
    dd_edge temp(mxd);
    mxd->createEdge(froms, tos, 1, temp);
    nsf += temp;

    // revert the vector
    from[machine] = -2; to[machine] = -2;
  }

  /* M -> G */
  {
    // adjust the vector
    from[machine] = 1; to[machine] = 3;

    // add it to nsf
    dd_edge temp(mxd);
    mxd->createEdge(froms, tos, 1, temp);
    nsf += temp;

    // revert the vector
    from[machine] = -2; to[machine] = -2;
  }

  /* G -> W */
  if (4 == machine) {
    // adjust the vector
    from[machine] = 3; to[machine] = 0;

    // add it to nsf
    dd_edge temp(mxd);
    mxd->createEdge(froms, tos, 1, temp);
    nsf += temp;

    // revert the vector
    from[machine] = -2; to[machine] = -2;
  }

  return nsf;
}


dd_edge MakeSynch1_23(forest* mxd)
{
  dd_edge nsf(mxd);

  int from[]    = {0, 3, 0, 0, -2};
  int to[]      = {0, 0, 1, 1, -2};
  int* froms[]  = { from };
  int* tos[]    = { to };

  mxd->createEdge(froms, tos, 1, nsf);

  return nsf;
}


dd_edge MakeSynch23_4(forest* mxd)
{
  dd_edge nsf(mxd);

  int from[]    = {0, -2, 3, 3, 0};
  int to[]      = {0, -2, 0, 0, 1};
  int* froms[]  = { from };
  int* tos[]    = { to };

  mxd->createEdge(froms, tos, 1, nsf);

  return nsf;
}


dd_edge getReachabilitySet(const dd_edge& states, const dd_edge& nsf, bool dfs)
{
  const binary_opname* op =
    dfs
    ? REACHABLE_STATES_DFS
    : REACHABLE_STATES_BFS;

  dd_edge rs(states);
  apply(op, states, nsf, rs);

  return rs;
}


// ========= END Reachability Set generation ===========



// ========== BEGIN Tests ===========


void testMDDComplement(const dd_edge& mdd)
{
  dd_edge complement(mdd);
  timer compTimer;
  compTimer.note_time();
  apply(COMPLEMENT, mdd, complement);
  compTimer.note_time();

  fprintf(stderr, "\nMDD:\n");
  mdd.show(stderr, 1);

  fprintf(stderr, "\nComplement(MDD):\n");
  complement.show(stderr, 1);

  fprintf(stderr, "\nComplement(MDD) took %.4e seconds\n",
      compTimer.get_last_interval()/1000000.0);
}


void testMXDComplement(const dd_edge& mxd)
{
  forest* f = mxd.getForest();
  assert(f);

  dd_edge complement(mxd);
  timer compTimer;
  compTimer.note_time();
  apply(COMPLEMENT, mxd, complement);
  compTimer.note_time();

  fprintf(stderr, "\nMXD:\n");
  mxd.show(stderr, 1);

  fprintf(stderr, "\nComplement(MXD):\n");
  complement.show(stderr, 1);

  fprintf(stderr, "\nComplement(MXD) took %.4e seconds\n",
      compTimer.get_last_interval()/1000000.0);

  // Complement of MXD using Mxd-Difference: (1 - MXD).

  dd_edge one(mxd);
  f->createEdge(true, one);
  compTimer.note_time();
  dd_edge diff = one - mxd;
  compTimer.note_time();

  fprintf(stderr, "\n(1 - MXD):\n");
  complement.show(stderr, 1);

  fprintf(stderr, "\n(1 - MXD) took %.4e seconds\n",
      compTimer.get_last_interval()/1000000.0);

  // Check if Complement(MXD) == (1 - MXD)

  fprintf(stderr, "diff %s complement\n\n",
      diff == complement? "==": "!=");
}


void testIndexSet(domain* d, const dd_edge& mdd)
{
  assert(d);

  // Create a EV+MDD forest in this domain (to store index set)
  forest* evplusmdd = d->createForest(false, forest::INTEGER, forest::INDEX_SET);
  assert(evplusmdd);
  expert_forest* ef = static_cast<expert_forest*>(evplusmdd);

  // Convert MDD to Index Set EV+MDD and print the states
  dd_edge indexSet(evplusmdd);
  apply(CONVERT_TO_INDEX_SET, mdd, indexSet);

  // Get Index Set Cardinality
  double cardinality = indexSet.getCardinality();
  int node = indexSet.getNode();
  fprintf(stderr, "Cardinality of node %d: %d\n", node,
      ef->getIndexSetCardinality(node));

  // Print index set elements
  int element[N];
  for (int index = 0; index < int(cardinality); index++)
  {
    evplusmdd->getElement(indexSet, index, element);
    fprintf(stderr, "Element at index %d: [ ", index);
    for (int i = N - 1; i > 0; i--)
    {
      fprintf(stderr, "%d ", element[i]);
    }
    fprintf(stderr, "]\n");
  }

  // For each node in the index set, print its cardinality.
  if (doIndexSetCardinality) {
    std::set<int> visited;
    std::deque<int> toVisit;
    toVisit.push_back(indexSet.getNode());

    while (!toVisit.empty()) {
      int n = toVisit.front();
      toVisit.pop_front();
      if (visited.find(n) != visited.end()) continue;
      visited.insert(n);
      // explore n
      if (ef->isFullNode(n)) {
        int sz = ef->getFullNodeSize(n);
        for (int i = 0; i < sz; i++)
        {
          int dp = ef->getFullNodeDownPtr(n, i);
          if (!ef->isTerminalNode(dp) && visited.find(dp) == visited.end())
            toVisit.push_back(dp);
        }
      } else {
        int sz = ef->getSparseNodeSize(n);
        for (int i = 0; i < sz; i++)
        {
          int dp = ef->getSparseNodeDownPtr(n, i);
          if (!ef->isTerminalNode(dp) && visited.find(dp) == visited.end())
            toVisit.push_back(dp);
        }
      }
    }

    fprintf(stderr, "Nodes in the Index Set:\n");
    for (std::set<int>::iterator iter = visited.begin();
        iter != visited.end(); ++iter)
    {
      fprintf(stderr, "Node %d: Cardinality %d, Height: %d, InCount: %d\n",
          *iter, ef->getIndexSetCardinality(*iter),
          ef->getNodeHeight(*iter), ef->getInCount(*iter));
    }
  }
}


void testFindFirstElement(const dd_edge& mdd)
{
  forest* f = mdd.getForest();
  assert(f != 0);

  int element[N];
  int* elements[] = { element };

  dd_edge mddCopy(mdd);
  int cardinality = mddCopy.getCardinality();

  for (int index = 0; index < cardinality; index++)
  {
    memset(element, 0, N * sizeof(int));
    f->findFirstElement(mddCopy, element);

    fprintf(stderr, "Element at index %d: [ ", index);
    for (int i = N - 1; i > 0; i--)
    {
      fprintf(stderr, "%d ", element[i]);
    }
    fprintf(stderr, "]\n");

    dd_edge temp(f);
    f->createEdge(elements, 1, temp);
    mddCopy -= temp;
  }

  assert(mddCopy.getNode() == 0);
}


void testMTMDDIterator(const dd_edge& mtmdd)
{
  dd_edge copy(mtmdd);
  unsigned counter = 0;
  for (enumerator iter(copy); iter; ++iter, ++counter)
  {
    const int* element = iter.getAssignments();
    const int* curr = element + N - 1;
    const int* end = element - 1;
    fprintf(stderr, "%d: [%d", counter, *curr--);
    while (curr != end) { fprintf(stderr, " %d", *curr--); }
    fprintf(stderr, "]\n");
  }
  fprintf(stderr, "Iterator traversal: %0.4e elements\n", double(counter));
  fprintf(stderr, "Cardinality: %0.4e\n", copy.getCardinality());
}


void testMTMXDIterator(const dd_edge& mtmxd)
{
  dd_edge copy(mtmxd);
  unsigned counter = 0;
  for (enumerator iter(copy); iter; ++iter, ++counter)
  {
#ifdef NEW_ITERATORS
    const int* element = iter.getAssignments();
    assert(element != 0);

    fprintf(stderr, "%d: [%d", counter, element[1]);
    for (int i=2; i<=N; i++) { fprintf(stderr, " %d", element[i]); }

    fprintf(stderr, "] --> [%d", element[-1]);
    for (int i=2; i<=N; i++) { fprintf(stderr, " %d", element[-i]); }
    fprintf(stderr, "]\n");
#else
    const int* element = iter.getAssignments();
    const int* pelement = iter.getPrimedAssignments();
    assert(element != 0 && pelement != 0);

    const int* curr = element + N - 1;
    const int* end = element - 1;
    fprintf(stderr, "%d: [%d", counter, *curr--);
    while (curr != end) { fprintf(stderr, " %d", *curr--); }

    curr = pelement + N - 1;
    end = pelement - 1;
    fprintf(stderr, "] --> [%d", *curr--);
    while (curr != end) { fprintf(stderr, " %d", *curr--); }
    fprintf(stderr, "]\n");
#endif
  }
  fprintf(stderr, "Iterator traversal: %0.4e elements\n", double(counter));
  fprintf(stderr, "Cardinality: %0.4e\n", copy.getCardinality());
}

// ========== END Tests ===========

