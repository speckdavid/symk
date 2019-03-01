
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
    Implementation of operation framework.

    Actual operations are in separate files, in operations/ directory.
*/

#include "defines.h"
#include <set>
// #include "compute_table.h"

// #define DEBUG_CLEANUP
// #define DEBUG_FINALIZE
// #define DEBUG_FINALIZE_SPLIT
// #define DEBUG_EVENT_MASK

namespace MEDDLY {

  /*
      Convert from array of linked lists to contiguous array with indexes.
  
      Idea:
      traverse the array in order.
      everything after this point is still "linked lists".
      everything before this point is contiguous, but
        it is possible that one of the lists points here.
        if so, the next pointer holds the "forwarding address".
  */
  template <typename TYPE>
  void defragLists(TYPE* data, int* next, int* Lists, int NL)
  {
    int P = 0;  // "this point"
    for (int i=NL; i; i--) {
      int L = Lists[i];
      Lists[i] = P;
      while (L>=0) {
        // if L<P then there must be a forwarding address; follow it
        while (L<P) L = next[L];
        // ok, we're at the right slot now
        int nxt = next[L];
        if (L != P) {
          // element L belongs in slot P, swap them...
          SWAP(data[L], data[P]);
          next[L] = next[P];
          // ...and set up the forwarding address
          next[P] = L;
        }
        P++;
        L = nxt;
      }
    } // for k
    Lists[0] = P;
  }
}

// ******************************************************************
// *                         opname methods                         *
// ******************************************************************

int MEDDLY::opname::next_index;

MEDDLY::opname::opname(const char* n)
{
  name = n;
  index = next_index;
  next_index++;
}

MEDDLY::opname::~opname()
{
  // library must be closing
}

// ******************************************************************
// *                      unary_opname methods                      *
// ******************************************************************

MEDDLY::unary_opname::unary_opname(const char* n) : opname(n)
{
}

MEDDLY::unary_opname::~unary_opname()
{
}

MEDDLY::unary_operation* 
MEDDLY::unary_opname::buildOperation(expert_forest* ar, expert_forest* rs) const
{
  throw error(error::TYPE_MISMATCH);  
}

MEDDLY::unary_operation* 
MEDDLY::unary_opname::buildOperation(expert_forest* ar, opnd_type res) const
{
  throw error(error::TYPE_MISMATCH);
}


// ******************************************************************
// *                     binary_opname  methods                     *
// ******************************************************************

MEDDLY::binary_opname::binary_opname(const char* n) : opname(n)
{
}

MEDDLY::binary_opname::~binary_opname()
{
}

// ******************************************************************
// *                   specialized_opname methods                   *
// ******************************************************************

MEDDLY::specialized_opname::arguments::arguments()
{
  setAutoDestroy(true);
}

MEDDLY::specialized_opname::arguments::~arguments()
{
}


MEDDLY::specialized_opname::specialized_opname(const char* n) : opname(n)
{
}

MEDDLY::specialized_opname::~specialized_opname()
{
}

// ******************************************************************
// *                    numerical_opname methods                    *
// ******************************************************************

MEDDLY::numerical_opname::numerical_args
::numerical_args(const dd_edge &xi, const dd_edge &a, const dd_edge &yi)
 : x_ind(xi), A(a), y_ind(yi)
{
}

MEDDLY::numerical_opname::numerical_args::~numerical_args()
{
}


MEDDLY::numerical_opname::numerical_opname(const char* n)
 : specialized_opname(n)
{
}

MEDDLY::numerical_opname::~numerical_opname()
{
}

// ******************************************************************
// *                                                                *
// *                    satpregen_opname methods                    *
// *                                                                *
// ******************************************************************


MEDDLY::satpregen_opname::satpregen_opname(const char* n)
 : specialized_opname(n)
{
}

MEDDLY::satpregen_opname::~satpregen_opname()
{
}


void
MEDDLY::satpregen_opname::pregen_relation
::setForests(forest* inf, forest* mxd, forest* outf)
{
  insetF = inf;
  outsetF = outf;
  mxdF = smart_cast <MEDDLY::expert_forest*>(mxd);
  if (0==insetF || 0==outsetF || 0==mxdF) throw error(error::MISCELLANEOUS);

  // Check for same domain
  if (  
    (insetF->getDomain() != mxdF->getDomain()) || 
    (outsetF->getDomain() != mxdF->getDomain()) 
  )
    throw error(error::DOMAIN_MISMATCH);

  // for now, anyway, inset and outset must be same forest
  if (insetF != outsetF)
    throw error(error::FOREST_MISMATCH);

  // Check forest types
  if (
    insetF->isForRelations()    ||
    !mxdF->isForRelations()     ||
    outsetF->isForRelations()   ||
    (insetF->getRangeType() != mxdF->getRangeType())        ||
    (outsetF->getRangeType() != mxdF->getRangeType())       ||
    (insetF->getEdgeLabeling() != forest::MULTI_TERMINAL)   ||
    (outsetF->getEdgeLabeling() != forest::MULTI_TERMINAL)  ||
    (mxdF->getEdgeLabeling() != forest::MULTI_TERMINAL)     ||
    (outsetF->getEdgeLabeling() != forest::MULTI_TERMINAL)
  )
    throw error(error::TYPE_MISMATCH);

  // Forests are good; set number of variables
  K = mxdF->getDomain()->getNumVariables();
}

MEDDLY::satpregen_opname::pregen_relation
::pregen_relation(forest* inf, forest* mxd, forest* outf, int nevents)
{
  setForests(inf, mxd, outf);

  num_events = nevents;
  if (num_events) {
    events = new node_handle[num_events];
    next = new int[num_events];
  } else {
    events = 0;
    next = 0;
  }
  last_event = -1;

  level_index = new int[K+1];
  for (int k=0; k<=K; k++) level_index[k] = -1;   // null pointer
}

MEDDLY::satpregen_opname::pregen_relation
::pregen_relation(forest* inf, forest* mxd, forest* outf)
{
  setForests(inf, mxd, outf);

  events = new node_handle[K+1];
  for (int k=0; k<=K; k++) events[k] = 0;

  next = 0;
  level_index = 0;

  num_events = -1;
  last_event = -1;
}


MEDDLY::satpregen_opname::pregen_relation
::~pregen_relation()
{
  if (events && mxdF) {
    for (int k=0; k<=last_event; k++) if(events[k]) mxdF->unlinkNode(events[k]);
  }
  delete[] events;
  delete[] next;
  delete[] level_index;
}

void
MEDDLY::satpregen_opname::pregen_relation
::addToRelation(const dd_edge &r)
{
  MEDDLY_DCASSERT(mxdF);

  if (r.getForest() != mxdF)  throw error(error::FOREST_MISMATCH);

  int k = r.getLevel(); 
  if (0==k) return;
  if (k<0) k = -k;   

  if (0==level_index) {
    // relation is "by levels"

    if (0==events[k]) {
      events[k] = mxdF->linkNode(r.getNode());
    } else {
      // already have something at this level; perform the union
      dd_edge tmp(mxdF);
      tmp.set(events[k]); // does not increase incoming count
      tmp += r;
      events[k] = mxdF->linkNode(tmp.getNode());
      // tmp should be destroyed here
    }

  } else {
    // relation is "by events"

    if (isFinalized())              throw error(error::MISCELLANEOUS);
    if (last_event+1 >= num_events) throw error(error::VALUE_OVERFLOW);

    last_event++;

    events[last_event] = mxdF->linkNode(r.getNode());
    next[last_event] = level_index[k];
    level_index[k] = last_event;
  }
}


void
MEDDLY::satpregen_opname::pregen_relation
::splitMxd(splittingOption split)
{
  if (split == None) return;
  if (split == MonolithicSplit) {
    unionLevels();
    split = SplitOnly;
  }

  // For each level k, starting from the top level
  //    MXD(k) is the matrix diagram at level k.
  //    Calculate ID(k), the intersection of the diagonals of MXD(k).
  //    Subtract ID(k) from MXD(k).
  //    Add ID(k) to level(ID(k)).

#ifdef DEBUG_FINALIZE_SPLIT
  printf("Splitting events in finalize()\n");
  printf("events array: [");
  for (int i=0; i<=K; i++) {
    if (i) printf(", ");
    printf("%d", events[i]);
  }
  printf("]\n");
#endif

  // Initialize operations
  binary_operation* mxdUnion = getOperation(UNION, mxdF, mxdF, mxdF);
  MEDDLY_DCASSERT(mxdUnion);

  binary_operation* mxdIntersection =
    getOperation(INTERSECTION, mxdF, mxdF, mxdF);
  MEDDLY_DCASSERT(mxdIntersection);

  binary_operation* mxdDifference = getOperation(DIFFERENCE, mxdF, mxdF, mxdF);
  MEDDLY_DCASSERT(mxdDifference);

  for (int k = K; k > 1; k--) {
    if (0 == events[k]) continue;

    MEDDLY_DCASSERT(ABS(mxdF->getNodeLevel(events[k])) <= k);

    // Initialize unpacked nodes
    unpacked_node* Mu = (isLevelAbove(k, mxdF->getNodeLevel(events[k]))) 
      ?   unpacked_node::newRedundant(mxdF, k, events[k], true)
      :   unpacked_node::newFromNode(mxdF, events[k], true)
    ;

    unpacked_node* Mp = unpacked_node::useUnpackedNode();

    bool first = true;
    node_handle maxDiag = 0;

    // Read "rows"
    for (int i = 0; i < Mu->getSize(); i++) {
      // Initialize column reader
      if (isLevelAbove(-k, mxdF->getNodeLevel(Mu->d(i)))) {
        Mp->initIdentity(mxdF, -k, i, Mu->d(i), true);
      } else {
        Mp->initFromNode(mxdF, Mu->d(i), true);
      }

      // Intersect along the diagonal
      if (first) {
        maxDiag = mxdF->linkNode(Mp->d(i));
        first = false;
      } else {
        node_handle nmd = mxdIntersection->compute(maxDiag, Mp->d(i));
        mxdF->unlinkNode(maxDiag);
        maxDiag = nmd;
      }
    } // for i

    // Cleanup
    unpacked_node::recycle(Mp);
    unpacked_node::recycle(Mu);

    if (0 == maxDiag) {
#ifdef DEBUG_FINALIZE_SPLIT
      printf("splitMxd: event %d, maxDiag %d\n", events[k], maxDiag);
#endif
      continue;
    }

    // Subtract maxDiag from events[k]
    // Do this only for SplitOnly. Other cases are handled later.
    if (split == SplitOnly) {
      int tmp = events[k];
      events[k] = mxdDifference->compute(events[k], maxDiag);
      mxdF->unlinkNode(tmp);
#ifdef DEBUG_FINALIZE_SPLIT
      printf("SplitOnly: event %d = event %d - maxDiag %d\n",
          events[k], tmp, maxDiag);
#endif
    } 

    // Add maxDiag to events[level(maxDiag)]
    int maxDiagLevel = ABS(mxdF->getNodeLevel(maxDiag));
    int tmp = events[maxDiagLevel];
    events[maxDiagLevel] = mxdUnion->compute(maxDiag, events[maxDiagLevel]);
    mxdF->unlinkNode(tmp);
    mxdF->unlinkNode(maxDiag);

    // Subtract events[maxDiagLevel] from events[k].
    // Do this only for SplitSubtract. SplitSubtractAll is handled later.
    if (split == SplitSubtract) {
      int tmp = events[k];
      events[k] = mxdDifference->compute(events[k], events[maxDiagLevel]);
      mxdF->unlinkNode(tmp);
#ifdef DEBUG_FINALIZE_SPLIT
      printf("SplitSubtract: event %d = event %d - event[maxDiagLevel] %d\n",
          events[k], tmp, maxDiag);
#endif
    }

  } // for k

  if (split == SplitSubtractAll) {
    // Subtract event[i] from all event[j], where j > i.
    for (int i = 1; i < K; i++) {
      for (int j = i + 1; j <= K; j++) {
        if (events[i] && events[j]) {
          int tmp = events[j];
          events[j] = mxdDifference->compute(events[j], events[i]);
          mxdF->unlinkNode(tmp);
#ifdef DEBUG_FINALIZE_SPLIT
          printf("SplitSubtractAll: event %d = event %d - event %d\n",
              events[j], tmp, events[i]);
#endif
        }
      }
    }
  }

#ifdef DEBUG_FINALIZE_SPLIT
  printf("After splitting events in finalize()\n");
  printf("events array: [");
  for (int i=0; i<=K; i++) {
    if (i) printf(", ");
    printf("%d", events[i]);
  }
  printf("]\n");
#endif
}


void
MEDDLY::satpregen_opname::pregen_relation
::unionLevels()
{
  if (K < 1) return;

  binary_operation* mxdUnion = getOperation(UNION, mxdF, mxdF, mxdF);
  MEDDLY_DCASSERT(mxdUnion);

  node_handle u = events[1];
  events[1] = 0;

  for (int k = 2; k <= K; k++) {
    if (0 == events[k]) continue;
    node_handle temp = mxdUnion->compute(u, events[k]);
    mxdF->unlinkNode(events[k]);
    events[k] = 0;
    mxdF->unlinkNode(u);
    u = temp;
  }

  events[mxdF->getNodeLevel(u)] = u;
}


void
MEDDLY::satpregen_opname::pregen_relation
::finalize(splittingOption split)
{
  if (0==level_index) {
    // by levels
    switch (split) {
      case SplitOnly: printf("Split: SplitOnly\n"); break;
      case SplitSubtract: printf("Split: SplitSubtract\n"); break;
      case SplitSubtractAll: printf("Split: SplitSubtractAll\n"); break;
      case MonolithicSplit: printf("Split: MonolithicSplit\n"); break;
      default: printf("Split: None\n");
    }
    splitMxd(split);
    if (split != None && split != MonolithicSplit) {
#ifdef DEBUG_FINALIZE_SPLIT
      // Union the elements, and then re-run.
      // Result must be the same as before.
      node_handle* old_events = new node_handle[K+1];
      for(int k = 0; k <= K; k++) {
        old_events[k] = events[k];
        if (old_events[k]) mxdF->linkNode(old_events[k]);
      }
      splitMxd(MonolithicSplit);
      binary_operation* mxdDifference =
        getOperation(DIFFERENCE, mxdF, mxdF, mxdF);
      MEDDLY_DCASSERT(mxdDifference);
      for(int k = 0; k <= K; k++) {
        if (old_events[k] != events[k]) {
          node_handle diff1 = mxdDifference->compute(old_events[k], events[k]);
          node_handle diff2 = mxdDifference->compute(events[k], old_events[k]);
          printf("error at level %d, n:k %d:%d, %d:%d\n",
              k,
              diff1, mxdF->getNodeLevel(diff1),
              diff2, mxdF->getNodeLevel(diff2)
              );
          mxdF->unlinkNode(diff1);
          mxdF->unlinkNode(diff2);
        }
        if (old_events[k]) mxdF->unlinkNode(old_events[k]);
      }

      delete [] old_events;
#endif
    }
    return; 
  }

#ifdef DEBUG_FINALIZE
  printf("Finalizing pregen relation\n");
  printf("%d events total\n", last_event+1);
  printf("events array: [");
  for (int i=0; i<=last_event; i++) {
    if (i) printf(", ");
    printf("%d", events[i]);
  }
  printf("]\n");
  printf("next array: [");
  for (int i=0; i<=last_event; i++) {
    if (i) printf(", ");
    printf("%d", next[i]);
  }
  printf("]\n");
  printf("level_index array: [%d", level_index[1]);
  for (int i=2; i<=K; i++) {
    printf(", %d", level_index[i]);
  }
  printf("]\n");
#endif

  // convert from array of linked lists to contiguous array.
  defragLists(events, next, level_index, K);

  // done with next pointers
  delete[] next;
  next = 0;

#ifdef DEBUG_FINALIZE
  printf("\nAfter finalization\n");
  printf("events array: [");
  for (int i=0; i<=last_event; i++) {
    if (i) printf(", ");
    printf("%d", events[i]);
  }
  printf("]\n");
  printf("level_index array: [%d", level_index[1]);
  for (int i=2; i<=K; i++) {
    printf(", %d", level_index[i]);
  }
  printf("]\n");
#endif
}


// ******************************************************************
// *                                                                *
// *                     satotf_opname  methods                     *
// *                                                                *
// ******************************************************************

MEDDLY::satotf_opname::satotf_opname(const char* n)
 : specialized_opname(n)
{
}

MEDDLY::satotf_opname::~satotf_opname()
{
}

// ============================================================

MEDDLY::satotf_opname::subevent::subevent(forest* f, int* v, int nv, bool firing)
: vars(0), num_vars(nv), root(dd_edge(f)), top(0),
  f(static_cast<expert_forest*>(f)), is_firing(firing)
{
  MEDDLY_DCASSERT(f != 0);
  MEDDLY_DCASSERT(v != 0);
  MEDDLY_DCASSERT(nv > 0);

  vars = new int[num_vars];
  memcpy(vars, v, num_vars * sizeof(int));

  // find top
  top = vars[0];
  for (int i = 1; i < num_vars; i++) {
    if (isLevelAbove(top, vars[i])) top = vars[i];
  }

  unpminterms = pminterms = 0;
  num_minterms = size_minterms = 0;
}

MEDDLY::satotf_opname::subevent::~subevent()
{
  if (vars) delete [] vars;
  for (int i=0; i<num_minterms; i++) {
    delete[] unpminterms[i];
    delete[] pminterms[i];
  }
  free(unpminterms);
  free(pminterms);
}

void MEDDLY::satotf_opname::subevent::clearMinterms()
{
  for (int i=0; i<num_minterms; i++) {
    delete[] unpminterms[i];
    delete[] pminterms[i];
  }
  free(unpminterms);
  free(pminterms);
  unpminterms = pminterms = 0;
  num_minterms = 0;
}


void MEDDLY::satotf_opname::subevent::confirm(otf_relation& rel, int v, int i) {
  throw MEDDLY::error::NOT_IMPLEMENTED;
}


bool MEDDLY::satotf_opname::subevent::addMinterm(const int* from, const int* to)
{
  /*
  ostream_output out(std::cout);
  out << "Adding minterm: [";
  for (int i = f->getNumVariables(); i >= 0; i--) {
    out << from[i] << " -> " << to[i] << " , ";
  }
  out << "]\n";
  */

  if (num_minterms >= size_minterms) {
    int old_size = size_minterms;
    size_minterms = (0==size_minterms)? 8: MIN(2*size_minterms, 256 + size_minterms);
    unpminterms = (int**) realloc(unpminterms, size_minterms * sizeof(int**));
    pminterms = (int**) realloc(pminterms, size_minterms * sizeof(int**));
    if (0==unpminterms || 0==pminterms) return false; // malloc or realloc failed
    for (int i=old_size; i<size_minterms; i++) {
      unpminterms[i] = 0;
      pminterms[i] = 0;
    }
  }
  if (0==unpminterms[num_minterms]) {
    unpminterms[num_minterms] = new int[f->getNumVariables() + 1];
    MEDDLY_DCASSERT(0==pminterms[num_minterms]);
    pminterms[num_minterms] = new int[f->getNumVariables() + 1];
  }
  // out << "Added minterm: [";
  for (int i = f->getNumVariables(); i >= 0; i--) {
    unpminterms[num_minterms][i] = from[i];
    pminterms[num_minterms][i] = to[i];
    // out << unpminterms[num_minterms][i] << " -> " << pminterms[num_minterms][i] << " , ";
  }
  // out << "]\n";
  expert_domain* d = static_cast<expert_domain*>(f->useDomain());
  for (int i = num_vars - 1; i >= 0; i--) {
    int level = vars[i];
    // expand "to" since the set of unconfirmed local states is always larger
    if (to[level] > 0 && to[level] >= f->getLevelSize(-level)) {
      d->enlargeVariableBound(level, false, 1+to[level]);
    }
  }
  num_minterms++;
  return true;
}



void MEDDLY::satotf_opname::subevent::buildRoot() {
  if (0 == num_minterms) return;
  /*
  ostream_output out(std::cout);
  out << "Building subevent from " << num_minterms << " minterms\n";
  for (int i = 0; i < num_minterms; i++) {
    out << "minterm[" << i << "]: [ ";
    for (int j = f->getNumVariables(); j >= 0; j--) {
      out << unpminterms[i][j] << " -> " << pminterms[i][j] << ", ";
    }
    out << " ]\n";
  }
  */
#if 0
  dd_edge sum(root);
  f->createEdge(unpminterms, pminterms, num_minterms, sum);
  num_minterms = 0;
  root += sum;
  // out << "Equivalent event: " << root.getNode() << "\n";
  // root.show(out, 2);
#else
  f->createEdge(unpminterms, pminterms, num_minterms, root);
  // out << "Result: ";
  // root.show(out, 2);
#endif
}


void MEDDLY::satotf_opname::subevent::showInfo(output& out) const {
  int num_levels = f->getDomain()->getNumVariables();
  for (int i = 0; i < num_minterms; i++) {
    out << "minterm[" << i << "]: ";
    for (int lvl = num_levels; lvl > 0; lvl--) {
      out << unpminterms[i][lvl] << " -> " << pminterms[i][lvl] << ", ";
    }
    out << "]\n";
  }
  root.show(out, 2);
}

long MEDDLY::satotf_opname::subevent::mintermMemoryUsage() const {
  return long(num_minterms * 2) * long(f->getDomain()->getNumVariables()) * sizeof(int);
}

// ============================================================

MEDDLY::satotf_opname::event::event(subevent** p, int np)
: subevents(p), num_subevents(np) 
{
  if (p == 0 || np <= 0 || p[0]->getForest() == 0)
    throw MEDDLY::error::INVALID_ARGUMENT;
  f = p[0]->getForest();
  for (int i=1; i<np; i++) {
    if (p[i]->getForest() != f) throw MEDDLY::error::INVALID_ARGUMENT;
  }

  top = p[0]->getTop();
  for (int i = 1; i < np; i++) {
    if (top < p[i]->getTop()) top = p[i]->getTop();
  }

  // Find the variable that effect this event from the list of subevents.
  // TODO:
  // Not efficient. p[i] is a sorted list of integers.
  // Should be able to insert in O(n) time
  // where n is the sum(p[i]->getNumVars).
  bool all_enabling_subevents = true;
  bool all_firing_subevents = true;
  std::set<int> sVars;
  std::set<int> firingVars;

  for (int i = 0; i < np; i++) {
    const int* subeventVars = p[i]->getVars();
    sVars.insert(subeventVars, subeventVars+p[i]->getNumVars());
    if (p[i]->isFiring()) {
      all_enabling_subevents = false;
      firingVars.insert(subeventVars, subeventVars+p[i]->getNumVars());
    } else {
      all_firing_subevents = false;
    }
  }

  MEDDLY_DCASSERT(all_enabling_subevents || !firingVars.empty());

  is_disabled = (all_enabling_subevents || all_firing_subevents);

  num_vars = sVars.size();
  vars = new int[num_vars];
  int* curr = &vars[0];
  for (std::set<int>::iterator it=sVars.begin(); it!=sVars.end(); ) {
    *curr++ = *it++;
  }

  num_firing_vars = firingVars.size();
  firing_vars = new int[num_firing_vars];
  curr = &firing_vars[0];
  for (std::set<int>::iterator it=firingVars.begin(); it!=firingVars.end(); ) {
    *curr++ = *it++;
  }

  root = dd_edge(f);
  event_mask = dd_edge(f);
  event_mask_from_minterm = 0;
  event_mask_to_minterm = 0;
  needs_rebuilding = is_disabled? false: true;
}

MEDDLY::satotf_opname::event::~event()
{
  for (int i=0; i<num_subevents; i++) delete subevents[i];
  delete[] subevents;
  delete[] vars;
  delete[] firing_vars;
  delete[] event_mask_from_minterm;
  delete[] event_mask_to_minterm;
}

void MEDDLY::satotf_opname::event::buildEventMask()
{
  MEDDLY_DCASSERT(num_subevents > 0);
  MEDDLY_DCASSERT(f);

  if (0 == event_mask_from_minterm) {
    const size_t minterm_size = f->getNumVariables()+1;
    event_mask_from_minterm = new int[minterm_size];
    event_mask_to_minterm = new int[minterm_size];

    for (unsigned i = 0; i < minterm_size; i++) {
      event_mask_from_minterm[i] = MEDDLY::DONT_CARE;
      event_mask_to_minterm[i] = MEDDLY::DONT_CHANGE;
    }

    for (int i = 0; i < num_firing_vars; i++) {
      event_mask_to_minterm[firing_vars[i]] = MEDDLY::DONT_CARE;
    }
  }

  f->createEdge(&event_mask_from_minterm, &event_mask_to_minterm, 1, event_mask);
#ifdef DEBUG_EVENT_MASK
  printf("event_mask: %d\n" , event_mask.getNode());
  ostream_output out(std::cout);
  event_mask.show(out, 2);
#endif
}

bool MEDDLY::satotf_opname::event::rebuild()
{
  MEDDLY_DCASSERT(num_subevents > 0);
  if (is_disabled) return false;
  if (!needs_rebuilding) return false;
  needs_rebuilding = false;

  // An event is a conjunction of sub-events (or sub-functions).
  for (int i = 0; i < num_subevents; i++) {
    subevents[i]->buildRoot();
  }
  buildEventMask();

  dd_edge e = event_mask;
  for (int i = 0; i < num_subevents; i++) {
    e *= subevents[i]->getRoot();
  }

  /*
  if (e.getNode() == 0) {
    ostream_output out(std::cout);
    f->useDomain()->showInfo(out);
    out << "subevent: " << event_mask.getNode() << "\n";
    event_mask.show(out, 2);
    for (int i = 0; i < num_subevents; i++) {
      out << "subevent: " << subevents[i]->getRoot().getNode() << "\n";
      subevents[i]->getRoot().show(out, 2);
    }
  }
  */

  if (e == root) return false;
  root += e;
  return true;
}

void MEDDLY::satotf_opname::event::enlargeVariables()
{
  expert_domain* ed = static_cast<expert_forest*>(f)->useExpertDomain();
  for (int i = 0; i < num_vars; i++) {
    int unprimed = ABS(vars[i]);
    int primed = -unprimed;
    int unprimedSize = f->getLevelSize(unprimed);
    int primedSize = f->getLevelSize(primed);
    if (unprimedSize < primedSize) {
      ed->enlargeVariableBound(unprimed, false, primedSize);
    }
    MEDDLY_DCASSERT(f->getLevelSize(unprimed) == f->getLevelSize(primed));
  }
}


void MEDDLY::satotf_opname::event::showInfo(output& out) const {
  for (int i = 0; i < num_subevents; i++) {
    out << "subevent " << i << "\n";
    subevents[i]->showInfo(out);
  }
}

long MEDDLY::satotf_opname::event::mintermMemoryUsage() const {
  long usage = 0;
  for (int i = 0; i < num_subevents; i++) {
    usage += subevents[i]->mintermMemoryUsage();
  }
  return usage;
}

// ============================================================

MEDDLY::satotf_opname::otf_relation::otf_relation(forest* inmdd, 
  forest* mxd, forest* outmdd, event** E, int ne)
: insetF(static_cast<expert_forest*>(inmdd)),
  mxdF(static_cast<expert_forest*>(mxd)),
  outsetF(static_cast<expert_forest*>(outmdd))
{
  if (0==insetF || 0==outsetF || 0==mxdF) throw error(error::MISCELLANEOUS);

  // Check for same domain
  if (  
    (insetF->getDomain() != mxdF->getDomain()) || 
    (outsetF->getDomain() != mxdF->getDomain()) 
  )
    throw error(error::DOMAIN_MISMATCH);

  // for now, anyway, inset and outset must be same forest
  if (insetF != outsetF)
    throw error(error::FOREST_MISMATCH);

  // Check forest types
  if (
    insetF->isForRelations()    ||
    !mxdF->isForRelations()     ||
    outsetF->isForRelations()   ||
    (insetF->getRangeType() != mxdF->getRangeType())        ||
    (outsetF->getRangeType() != mxdF->getRangeType())       ||
    (insetF->getEdgeLabeling() != forest::MULTI_TERMINAL)   ||
    (mxdF->getEdgeLabeling() != forest::MULTI_TERMINAL)     ||
    (outsetF->getEdgeLabeling() != forest::MULTI_TERMINAL)
  )
    throw error(error::TYPE_MISMATCH);

  // Forests are good; set number of variables
  num_levels = mxdF->getDomain()->getNumVariables() + 1;

  // Build the events-per-level data structure
  // (0) Initialize
  num_events_by_top_level = new int[num_levels];
  num_events_by_level = new int[num_levels];
  memset(num_events_by_top_level, 0, sizeof(int)*num_levels);
  memset(num_events_by_level, 0, sizeof(int)*num_levels);

  // (1) Determine the number of events per level
  for (int i = 0; i < ne; i++) {
    if (E[i]->isDisabled()) continue;
    int nVars = E[i]->getNumVars();
    const int* vars = E[i]->getVars();
    for (int j = 0; j < nVars; j++) {
      num_events_by_level[vars[j]]++;
    }
    num_events_by_top_level[E[i]->getTop()]++;
  }

  // (2) Allocate events[i]
  events_by_top_level = new event**[num_levels];
  events_by_level = new event**[num_levels];
  for (int i = 0; i < num_levels; i++) {
    events_by_top_level[i] = num_events_by_top_level[i] > 0
      ? new event*[num_events_by_top_level[i]]: 0;
    events_by_level[i] = num_events_by_level[i] > 0
      ? new event*[num_events_by_level[i]]: 0;
    num_events_by_top_level[i] = 0; // reset this; to be used by the next loop
    num_events_by_level[i] = 0; // reset this; to be used by the next loop
  }

  // (3) Store events by level
  for (int i = 0; i < ne; i++) {
    if (E[i]->isDisabled()) continue;
    int nVars = E[i]->getNumVars();
    const int* vars = E[i]->getVars();
    for (int j = 0; j < nVars; j++) {
      int level = vars[j];
      events_by_level[level][num_events_by_level[level]++] = E[i];
    }
    int level = E[i]->getTop();
    events_by_top_level[level][num_events_by_top_level[level]++] = E[i];
    E[i]->markForRebuilding();
  }

  // Build the subevents-per-level data structure
  // (0) Initialize
  num_subevents_by_level = new int[num_levels];
  memset(num_subevents_by_level, 0, sizeof(int)*num_levels);

  // (1) Determine the number of subevents per level
  for (int i = 0; i < ne; i++) {
    if (E[i]->isDisabled()) continue;
    int nse = E[i]->getNumOfSubevents();
    subevent** se = E[i]->getSubevents();
    for (int j = 0; j < nse; j++) {
      int nVars = se[j]->getNumVars();
      const int* vars = se[j]->getVars();
      for (int k = 0; k < nVars; k++) {
        num_subevents_by_level[vars[k]]++;
      }
    }
  }

  // (2) Allocate subevents[i]
  subevents_by_level = new subevent**[num_levels];
  for (int i = 0; i < num_levels; i++) {
    subevents_by_level[i] = num_subevents_by_level[i] > 0
      ? new subevent*[num_subevents_by_level[i]]: 0;
    num_subevents_by_level[i] = 0; // reset this; to be used by the next loop
  }

  // (3) Store subevents by level
  for (int i = 0; i < ne; i++) {
    if (E[i]->isDisabled()) continue;
    int nse = E[i]->getNumOfSubevents();
    subevent** se = E[i]->getSubevents();
    for (int j = 0; j < nse; j++) {
      int nVars = se[j]->getNumVars();
      const int* vars = se[j]->getVars();
      for (int k = 0; k < nVars; k++) {
        int level = vars[k];
        subevents_by_level[level][num_subevents_by_level[level]++] = se[j];
      }
    }
  }

  // confirmed local states
  confirmed = new bool*[num_levels];
  size_confirmed = new int[num_levels];
  num_confirmed = new int[num_levels];
  confirmed[0] = 0;
  for (int i = 1; i < num_levels; i++) {
    int level_size = mxdF->getLevelSize(-i);
    confirmed[i] = (bool*) malloc(level_size * sizeof(bool));
    for (int j = 0; j < level_size; j++) {
      confirmed[i][j] = false;
    }
    size_confirmed[i] = level_size;
    num_confirmed[i] = 0;
  }

  // TBD - debug here
}

MEDDLY::satotf_opname::otf_relation::~otf_relation()
{
  // ostream_output out(std::cout);
  // showInfo(out);

  for (int i = 0; i < num_levels; i++) {
    delete[] subevents_by_level[i];
    delete[] events_by_level[i];
    delete[] events_by_top_level[i];
    free(confirmed[i]);
  }
  delete[] subevents_by_level;
  delete[] events_by_level;
  delete[] events_by_top_level;
  delete[] num_subevents_by_level;
  delete[] num_events_by_level;
  delete[] num_events_by_top_level;
  delete[] confirmed;
  delete[] size_confirmed;
  delete[] num_confirmed;
}

void MEDDLY::satotf_opname::otf_relation::clearMinterms()
{
  for (int level = 1; level < num_levels; level++) {
    // Get subevents affected by this level, and rebuild them.
    int nSubevents = num_subevents_by_level[level];
    for (int i = 0; i < nSubevents; i++) {
      subevents_by_level[level][i]->clearMinterms();
    }
  }
}


bool MEDDLY::satotf_opname::otf_relation::confirm(int level, int index)
{
  // For each subevent that affects this level:
  //    (1) call subevent::confirm()
  //    (2) for each level k affected by the subevent,
  //        (a) enlarge variable bound of k to variable bound of k'

  enlargeConfirmedArrays(level, index+1);

  MEDDLY_DCASSERT(size_confirmed[level] > index);
  if (isConfirmed(level, index)) return false; 

  // Get subevents affected by this level, and rebuild them.
  int nSubevents = num_subevents_by_level[level];
  for (int i = 0; i < nSubevents; i++) {
    subevents_by_level[level][i]->confirm(const_cast<otf_relation&>(*this),
        level, index);
  }

  // Get events affected by this level, and mark them stale.
  const int nEvents = num_events_by_level[level];
  for (int i = 0; i < nEvents; i++) {
    // events_by_level[level][i]->enlargeVariables();
    events_by_level[level][i]->markForRebuilding();
  }

  confirmed[level][index] = true;
  num_confirmed[level]++;
  return true;
}


void findConfirmedStates(MEDDLY::satotf_opname::otf_relation* rel,
    bool** confirmed, int* num_confirmed,
    MEDDLY::node_handle mdd, int level,
    std::set<MEDDLY::node_handle>& visited) {
  if (level == 0) return;
  if (visited.find(mdd) != visited.end()) return;
  visited.insert(mdd);

  MEDDLY::expert_forest* insetF = rel->getInForest();
  int mdd_level = insetF->getNodeLevel(mdd);
  if (MEDDLY::isLevelAbove(level, mdd_level)) {
    // skipped level; confirm all local states at this level
    // go to the next level
    int level_size = insetF->getLevelSize(level);
    for (int i = 0; i < level_size; i++) {
      if (!confirmed[level][i]) {
        rel->confirm(level, i);
      }
    }
    findConfirmedStates(rel, confirmed, num_confirmed, mdd, level-1, visited);
  } else {
    if (MEDDLY::isLevelAbove(mdd_level, level)) throw MEDDLY::error::INVALID_VARIABLE;
    // mdd_level == level
    MEDDLY::unpacked_node *nr = MEDDLY::unpacked_node::newFromNode(insetF, mdd, false);
    for (int i = 0; i < nr->getNNZs(); i++) {
      if (!confirmed[level][nr->i(i)]) {
        rel->confirm(level, nr->i(i));
      }
      findConfirmedStates(rel, confirmed, num_confirmed, nr->d(i), level-1, visited);
    }
    MEDDLY::unpacked_node::recycle(nr);
  }
}

void MEDDLY::satotf_opname::otf_relation::confirm(const dd_edge& set)
{
  // Perform a depth-first traversal of set:
  //    At each level, mark all enabled states as confirmed.

  // Enlarge the confirmed arrays if needed
  for (int i = 1; i < num_levels; i++) {
      enlargeConfirmedArrays(i, mxdF->getLevelSize(-i));
  }
  std::set<node_handle> visited;
  findConfirmedStates(const_cast<otf_relation*>(this),
      confirmed, num_confirmed, set.getNode(), num_levels-1, visited);

  // ostream_output out(std::cout);
  // showInfo(out);
}



void MEDDLY::satotf_opname::otf_relation::enlargeConfirmedArrays(int level, int sz)
{
#if 0
  int curr = size_confirmed[level];
  int needed = mxdF->getLevelSize(-level);
  int new_size = curr*2 < needed? needed: curr*2;
  confirmed[level] = (bool*) realloc(confirmed[level], new_size * sizeof(bool));
  if (confirmed[level] == 0) throw error::INSUFFICIENT_MEMORY;
  for ( ; curr < new_size; ) confirmed[level][curr++] = false;
  size_confirmed[level] = curr;
#else
  if (sz <= size_confirmed[level]) return;
  sz = MAX( sz , size_confirmed[level]*2 );
  confirmed[level] = (bool*) realloc(confirmed[level], sz * sizeof(bool));
  if (confirmed[level] == 0) throw error::INSUFFICIENT_MEMORY;
  for (int i = size_confirmed[level]; i < sz; i++) confirmed[level][i] = false;
  size_confirmed[level] = sz;
#endif
}


void MEDDLY::satotf_opname::otf_relation::showInfo(output &strm) const
{
  for (int level = 1; level < num_levels; level++) {
    for (int ei = 0; ei < getNumOfEvents(level); ei++) {
      events_by_top_level[level][ei]->rebuild();
    }
  }
  strm << "On-the-fly relation info:\n";
  for (int level = 1; level < num_levels; level++) {
    for (int ei = 0; ei < getNumOfEvents(level); ei++) {
      strm << "Level: " << level << ", Event:[" << ei << "]: needs rebuilding? "
        << (events_by_top_level[level][ei]->needsRebuilding()? "true": "false")
        << "\n";
      strm << "Depends on levels: [";
      for (int j = 0; j < events_by_top_level[level][ei]->getNumVars(); j++) {
        strm << " " << events_by_top_level[level][ei]->getVars()[j] << " ";
      }
      strm << "]\n";
      events_by_top_level[level][ei]->showInfo(strm);
      events_by_top_level[level][ei]->getRoot().show(strm, 2);
    }
  }
}

long MEDDLY::satotf_opname::otf_relation::mintermMemoryUsage() const {
  long usage = 0;
  for (int level = 1; level < num_levels; level++) {
    for (int ei = 0; ei < getNumOfEvents(level); ei++) {
      usage += events_by_top_level[level][ei]->mintermMemoryUsage();
    }
  }
  return usage;
}


// ******************************************************************
// *                       operation  methods                       *
// ******************************************************************

MEDDLY::operation::operation(const opname* n, int kl, int al)
{
#ifdef DEBUG_CLEANUP
  fprintf(stdout, "Creating operation %p\n", this);
  fflush(stdout);
#endif
  MEDDLY_DCASSERT(kl>=0);
  MEDDLY_DCASSERT(al>=0);
  theOpName = n;
  key_length = kl;
  ans_length = al;
  is_marked_for_deletion = false;
  next = 0;
  discardStaleHits = true;

  // 
  // assign an index to this operation
  //
  if (free_list>=0) {
    oplist_index = free_list;
    free_list = op_holes[free_list];
  } else {
    if (list_size >= list_alloc) {
      int nla = list_alloc + 256;
      op_list = (operation**) realloc(op_list, nla * sizeof(void*));
      op_holes = (int*) realloc(op_holes, nla * sizeof(int));
      if (0==op_list || 0==op_holes) throw error(error::INSUFFICIENT_MEMORY);
      for (int i=list_size; i<list_alloc; i++) {
        op_list[i] = 0;
        op_holes[i] = -1;
      }
      list_alloc = nla;
    }
    oplist_index = list_size;
    list_size++;
  }
  op_list[oplist_index] = this;

  if (key_length) { // this op uses the CT
    // 
    // Initialize CT 
    //
    if (Monolithic_CT) {
      CT = Monolithic_CT;
    } else {
      CT = ct_initializer::createForOp(this);
    }

    //
    // Initialize CT search structure
    //
    // CTsrch = CT->initializeSearchKey(this);

  } else {
    MEDDLY_DCASSERT(0==ans_length);
    CT = 0;
    // CTsrch = 0;
  }
  CT_free_keys = 0;
}

MEDDLY::operation::~operation()
{
#ifdef DEBUG_CLEANUP
  fprintf(stdout, "Deleting operation %p %s\n", this, getName());
  fflush(stdout);
#endif

  while (CT_free_keys) {
    compute_table::search_key* next = CT_free_keys->next;
    delete CT_free_keys;
    CT_free_keys = next;
  }

  // delete CTsrch;
  if (CT && (CT!=Monolithic_CT)) delete CT;
  // delete next;  // Seriously, WTF?
  if (oplist_index >= 0) {
    MEDDLY_DCASSERT(op_list[oplist_index] == this);
    op_list[oplist_index] = 0;
    op_holes[oplist_index] = free_list;
    free_list = oplist_index;
  }
#ifdef DEBUG_CLEANUP
  fprintf(stdout, "Deleted operation %p %s\n", this, getName());
  fflush(stdout);
#endif
}

void MEDDLY::operation::removeStalesFromMonolithic()
{
  if (Monolithic_CT) Monolithic_CT->removeStales();
}

void MEDDLY::operation::removeAllFromMonolithic()
{
  if (Monolithic_CT) Monolithic_CT->removeAll();
}


void MEDDLY::operation::markForDeletion()
{
#ifdef DEBUG_CLEANUP
  fprintf(stdout, "Marking operation %p %s for deletion\n", this, getName());
  fflush(stdout);
#endif
  if (is_marked_for_deletion) return;
  is_marked_for_deletion = true;
  if (CT && CT->isOperationTable()) CT->removeStales();
}

void MEDDLY::operation::destroyAllOps() 
{
  for (int i=0; i<list_size; i++) delete op_list[i];
  free(op_list);
  free(op_holes);
  op_list = 0;
  op_holes = 0;
  list_size = 0;
  list_alloc = 0;
  free_list = -1;
}

void MEDDLY::operation::removeStaleComputeTableEntries()
{
  if (CT) CT->removeStales();
}

void MEDDLY::operation::removeAllComputeTableEntries()
{
#ifdef DEBUG_CLEANUP
  fprintf(stdout, "Removing entries for operation %p %s\n", this, getName());
  fflush(stdout);
#endif
  if (is_marked_for_deletion) return;
  is_marked_for_deletion = true;
  if (CT) CT->removeStales();
  is_marked_for_deletion = false;
#ifdef DEBUG_CLEANUP
  fprintf(stdout, "Removed entries for operation %p %s\n", this, getName());
  fflush(stdout);
#endif
}

void MEDDLY::operation::showMonolithicComputeTable(output &s, int verbLevel)
{
  if (Monolithic_CT) Monolithic_CT->show(s, verbLevel);
}

void MEDDLY::operation::showAllComputeTables(output &s, int verbLevel)
{
  if (Monolithic_CT) {
    Monolithic_CT->show(s, verbLevel);
    return;
  }
  for (int i=0; i<list_size; i++) 
    if (op_list[i]) {
      op_list[i]->showComputeTable(s, verbLevel);
    }
}

void MEDDLY::operation::showComputeTable(output &s, int verbLevel) const
{
  if (CT) CT->show(s, verbLevel);
}

// ******************************************************************
// *                    unary_operation  methods                    *
// ******************************************************************

MEDDLY::unary_operation::unary_operation(const unary_opname* code, int kl, 
  int al, expert_forest* arg, expert_forest* res) : operation(code, kl, al)
{
  argF = arg;
  resultType = FOREST;
  resF = res;

  registerInForest(argF);
  registerInForest(resF);

  setAnswerForest(resF);
}

MEDDLY::unary_operation::unary_operation(const unary_opname* code, int kl, 
  int al, expert_forest* arg, opnd_type res) : operation(code, kl, al)
{
  argF = arg;
  resultType = res;
  resF = 0;

  registerInForest(argF);

  setAnswerForest(0);
}

MEDDLY::unary_operation::~unary_operation()
{
  unregisterInForest(argF);
  unregisterInForest(resF);
}

void MEDDLY::unary_operation::computeDDEdge(const dd_edge &arg, dd_edge &res)
{
  throw error(error::TYPE_MISMATCH);
}

void MEDDLY::unary_operation::compute(const dd_edge &arg, long &res)
{
  throw error(error::TYPE_MISMATCH);
}

void MEDDLY::unary_operation::compute(const dd_edge &arg, double &res)
{
  throw error(error::TYPE_MISMATCH);
}

void MEDDLY::unary_operation::compute(const dd_edge &arg, ct_object &c)
{
  throw error(error::TYPE_MISMATCH);
}

// ******************************************************************
// *                    binary_operation methods                    *
// ******************************************************************

MEDDLY::binary_operation::binary_operation(const binary_opname* op, int kl, 
  int al, expert_forest* arg1, expert_forest* arg2, expert_forest* res)
: operation(op, kl, al)
{
  arg1F = arg1;
  arg2F = arg2;
  resF = res;

  registerInForest(arg1F);
  registerInForest(arg2F);
  registerInForest(resF);

  setAnswerForest(resF);
  can_commute = false;
}

MEDDLY::binary_operation::~binary_operation()
{
  unregisterInForest(arg1F);
  unregisterInForest(arg2F);
  unregisterInForest(resF);
}

MEDDLY::node_handle 
MEDDLY::binary_operation::compute(node_handle a, node_handle b)
{
  throw error(error::WRONG_NUMBER);
}

MEDDLY::node_handle 
MEDDLY::binary_operation::compute(int k, node_handle a, node_handle b)
{
  throw error(error::WRONG_NUMBER);
}

void MEDDLY::binary_operation::compute(int av, node_handle ap,
  int bv, node_handle bp, int &cv, node_handle &cp)
{
  throw error(error::WRONG_NUMBER);
}

void MEDDLY::binary_operation::compute(float av, node_handle ap,
  float bv, node_handle bp, float &cv, node_handle &cp)
{
  throw error(error::WRONG_NUMBER);
}

// ******************************************************************
// *                 specialized_operation  methods                 *
// ******************************************************************

MEDDLY::
specialized_operation::
specialized_operation(const specialized_opname* op, int kl, int al) 
 : operation(op, kl, al)
{
}

MEDDLY::specialized_operation::~specialized_operation()
{
}

void MEDDLY::specialized_operation::compute(const dd_edge &arg, dd_edge &res)
{
  throw error(error::TYPE_MISMATCH);
}

void MEDDLY::specialized_operation::compute(const dd_edge &ar1, 
  const dd_edge &ar2, dd_edge &res)
{
  throw error(error::TYPE_MISMATCH);
}

void MEDDLY::specialized_operation::compute(double* y, const double* x)
{
  throw error(error::TYPE_MISMATCH);
}


