
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

  Rubik's Rings puzzle.

  Two intersecting rings, with 34 total balls of 3 colors.

*/

#include "meddly.h"
#include "meddly_expert.h"
#include "timer.h"
#include <cstdio>
#include <stdarg.h>

// #define DEBUG_MOVE_BALL
// #define DEBUG_MAKE_ROTATION
// #define DEBUG_BUILD_NSF
// #define ALL_ROTATIONS
#define SHOW_ORDER

using namespace MEDDLY;

const int YY = 0;
const int RR = 1;
const int BB = 2;
const int COLORS = 3;

/// Size of the left ring
// const int leftring = 18;
int leftring;

/// Size of the right ring
// const int rightring = 18;
int rightring;

/// Level number for the "left ring" slots.
int* leftvar = 0;

/// Initial states for the "left ring" slots.
int* leftinit = 0;

/// Level number for the "right ring" slots.
int* rightvar = 0;

/// Initial states for the "right ring" slots.
int* rightinit = 0;

/// Total number of slots
int LEVELS;



inline void bailout(const char* fmt, ...)
{
  va_list argptr;
  va_start(argptr, fmt);
  vprintf(fmt, argptr);
  va_end(argptr);
  exit(1);
}


void sanityChecks()
{
  if (YY<0 || YY >= COLORS) {
    bailout("Illegal value for YY: %d\n", YY);
  }
  if (RR<0 || RR >= COLORS) {
    bailout("Illegal value for RR: %d\n", RR);
  }
  if (BB<0 || BB >= COLORS) {
    bailout("Illegal value for BB: %d\n", BB);
  }
  if (leftvar[0] != rightvar[0]) {
    bailout("Error: exchange position 0 must match in level arrays\n");
  }
  if (leftvar[6] != rightvar[6]) {
    bailout("Error: exchange position 6 must match in level arrays\n");
  }
  if (leftinit[0] != rightinit[0]) {
    bailout("Error: exchange position 0 must match in init arrays\n");
  }
  if (leftinit[6] != rightinit[6]) {
    bailout("Error: exchange position 6 must match in init arrays\n");
  }
  for (int i=0; i<leftring; i++) {
    if (leftvar[i]<1 || leftvar[i] > LEVELS) {
      bailout("Illegal value for leftvar[%d]: %d\n", i, leftvar[i]);
    }
    if ((leftinit[i] != YY) && (leftinit[i] != RR) && (leftinit[i] != BB)) {
      bailout("Illegal value for leftinit[%d]: %d\n", i, leftinit[i]);
    }
  }
  for (int i=0; i<rightring; i++) {
    if (rightvar[i]<1 || rightvar[i] > LEVELS) {
      bailout("Illegal value for rightvar[%d]: %d\n", i, rightvar[i]);
    }
    if ((rightinit[i] != YY) && (rightinit[i] != RR) && (rightinit[i] != BB)) {
      bailout("Illegal value for rightinit[%d]: %d\n", i, leftinit[i]);
    }
  }
#ifdef SHOW_ORDER
  printf("leftvar = {%d", leftvar[0]);
  for (int i=1; i<leftring; i++) printf(", %d", leftvar[i]);
  printf("}\n");
  printf("rightvar = {%d", rightvar[0]);
  for (int i=1; i<rightring; i++) printf(", %d", rightvar[i]);
  printf("}\n");
#endif
}


/**
    Use the following order:
      left ring is decreasing down to 1.
      right ring is increasing (except for second exchange slots).
*/
void decinc_order(int p_red, int p_blue)
{
  leftring = 7 + p_red;
  rightring = 7 + p_blue;

  LEVELS = leftring + rightring - 2;

  leftvar = new int[leftring];
  leftinit = new int[leftring];
  rightvar = new int[rightring];
  rightinit = new int[rightring];

  for (int i=leftring; i; i--) leftvar[leftring-i] = i;

  int k = leftring;
  for (int i=0; i<rightring; i++) {
    if (i != 6) {
      rightvar[i] = k;
      k++;
    } else {
      rightvar[6] = leftvar[6];
    }
  }

  int yellow = -2;  // Correct for exchange positions
  int blue = 0;
  int red = 0;

  for (int i=0; i<leftring; i++) {
    if (i<7) {
      leftinit[i] = YY;
      yellow++;
    } else {
      leftinit[i] = RR;
      red++;
    }
  }
  for (int i=0; i<rightring; i++) {
    if (i<7) {
      rightinit[i] = YY;
      yellow++;
    } else {
      rightinit[i] = BB;
      blue++;
    }
  }
  printf("Rubik's rings using:\n\t%2d yellow\n\t%2d red\n\t%2d blue\n",
    yellow, red, blue);

  sanityChecks();
}



/**
    Use the following order:
      left ring is increasing up to max.
      right ring is decreasing (except for second exchange slots).
*/
void incdec_order(int p_red, int p_blue)
{
  leftring = 7 + p_red;
  rightring = 7 + p_blue;

  LEVELS = leftring + rightring - 2;

  leftvar = new int[leftring];
  leftinit = new int[leftring];
  rightvar = new int[rightring];
  rightinit = new int[rightring];

  for (int i=0; i<leftring; i++) leftvar[i] = i + rightring-1;

  int k = leftvar[0];
  for (int i=0; i<rightring; i++) {
    if (i != 6) {
      rightvar[i] = k;
      k--;
    } else {
      rightvar[6] = leftvar[6];
    }
  }

  int yellow = -2;  // Correct for exchange positions
  int blue = 0;
  int red = 0;

  for (int i=0; i<leftring; i++) {
    if (i<7) {
      leftinit[i] = YY;
      yellow++;
    } else {
      leftinit[i] = RR;
      red++;
    }
  }
  for (int i=0; i<rightring; i++) {
    if (i<7) {
      rightinit[i] = YY;
      yellow++;
    } else {
      rightinit[i] = BB;
      blue++;
    }
  }
  printf("Rubik's rings using:\n\t%2d yellow\n\t%2d red\n\t%2d blue\n",
    yellow, red, blue);

  sanityChecks();
}



/**
    Builds relation where
      newSlot' == oldSlot
    and for all other levels: 
        if changing[k] is true, then is "don't care";
                                else is "don't change".
*/
void moveBall(int oldSlot, int newSlot, bool changing[], dd_edge &out)
{
  //
  // Build minterm arrays, if needed (save for later)
  // 
  static int** fromterms = 0;
  static int** toterms = 0;

  if (0==fromterms) {
    fromterms = new int*[COLORS];
    for (int i=0; i<COLORS; i++) {
      fromterms[i] = new int[1+LEVELS];
    }
  }
  if (0==toterms) {
    toterms = new int*[COLORS];
    for (int i=0; i<COLORS; i++) {
      toterms[i] = new int[1+LEVELS];
    }
  }

  //
  // Build the from and to arrays
  //
  for (int i=0; i<COLORS; i++) {
    for (int k=LEVELS; k; k--) {
      fromterms[i][k] = DONT_CARE;  
      toterms[i][k] = changing[k] ? DONT_CARE : DONT_CHANGE;
    }
    fromterms[i][oldSlot] = i;
    toterms[i][newSlot] = i;
  }

  //
  // And, build the edge
  //
  forest* f = out.getForest();
  f->createEdge(fromterms, toterms, COLORS, out);

#ifdef DEBUG_MOVE_BALL
  fprintf(stderr, "moveBall (%d, %d):\n", oldSlot, newSlot);
  out.show(stderr, 2);
  fflush(stderr);
#endif
}


inline const char* arrayName(const int* foo)
{
  if (foo == leftvar) return "leftvar";
  if (foo == rightvar) return "rightvar";
  return "???";
}

/// Build a ring rotation by R
void makeRotation(const int* vars, int nv, int r, dd_edge &out)
{
  bool changing[1+LEVELS];
  for (int k=LEVELS; k; k--) changing[k] = false;
  for (int i=0; i<nv; i++) changing[vars[i]] = true;

  moveBall(vars[0], vars[r], changing, out);

  for (int i=1; i<nv; i++) {
    dd_edge temp(out);
   
    moveBall(vars[i], vars[(i+r)%nv], changing, temp);

    out *= temp;
  } // for i

#ifdef DEBUG_MAKE_ROTATION
  fprintf(stderr, "makeRotation(%s, %d, %d):\n", arrayName(vars), nv, r);
  out.show(stderr, 2);
  fflush(stderr);
#endif
}

/// Build entire NSF
void buildNSF(dd_edge& out)
{
  forest* f = out.getForest();
  f->createEdge(false, out);

  dd_edge rot(out);

  //
  // Rotations for the left ring
  // 
  for (int r=1; r<leftring; r++) {
#ifndef ALL_ROTATIONS
    if (r != 1 && r+1 != leftring) continue;
#endif
    makeRotation(leftvar, leftring, r, rot);
    out += rot;
  }

  //
  // Rotations for the right ring
  //
  for (int r=1; r<rightring; r++) {
#ifndef ALL_ROTATIONS
    if (r != 1 && r+1 != rightring) continue;
#endif
    makeRotation(rightvar, rightring, r, rot);
    out += rot;
  }

#ifdef DEBUG_BUILD_NSF
  fprintf(stderr, "built nsf:\n");
  out.show(stderr, 2);
  fflush(stderr);
#endif
}

void printStats(const char* who, timer& watch, const dd_edge &node)
{
  printf("%s construction took %.4f seconds\n", 
    who, watch.get_last_interval()/1000000.0
  );
  printf("%s has\n\t%d nodes\n\t\%d edges\n", 
    who, node.getNodeCount(), node.getEdgeCount()
  );
  printf("    Stats:\n");
  FILE_output meddlyout(stdout);
  const expert_forest* ef = (expert_forest*) node.getForest();
  ef->reportStats(meddlyout, "\t",
    expert_forest::HUMAN_READABLE_MEMORY  |
    expert_forest::BASIC_STATS | 
    expert_forest::EXTRA_STATS |
    expert_forest::STORAGE_STATS | 
    expert_forest::HOLE_MANAGER_STATS
  );
}

double factorial(int n)
{
  double product = 1.0;
  for (int i=2; i<=n; i++) {
    product *= i;
  }
  return product;
}

double theoretical()
{
  //
  // How many of each ball do we have?
  //
  int balls[COLORS];
  int total = 0;
  for (int i=0; i<COLORS; i++) balls[i] = 0;

  for (int i=0; i<leftring; i++) {
    balls[leftinit[i]]++;
    total++;
  }
  for (int i=0; i<rightring; i++) {
    // is this the exchange position?  could be more efficient, but I don't care here
    bool exchange = false;
    for (int j=0; j<leftring; j++) {
      if (rightvar[i] != leftvar[j]) continue;
      exchange = true;
      break;
    }
    if (exchange) continue;
    balls[rightinit[i]]++;
    total++;
  }

  double denom = factorial(balls[0]);
  for (int i=1; i<COLORS; i++) {
    denom *= factorial(balls[i]);
  }
  return factorial(total) / denom;
}

int usage(const char* who)
{
  /* Strip leading directory, if any: */
  const char* name = who;
  for (const char* ptr=who; *ptr; ptr++) {
    if ('/' == *ptr) name = ptr+1;
  }
  printf("Usage: %s <options>\n\n", name);
  printf("\t -r n:  Specify the number of red balls (default 11)\n");
  printf("\t -b n:  Specify the number of blue balls (default 11)\n\n");
  printf("\t -bfs:  Traditional iteration\n");
  printf("\t -dfs:  Saturation (default) \n\n");
  printf("\t -opt:  Optimistic node deletion (default)\n");
  printf("\t-pess:  Pessimistic node deletion \n\n");
  printf("\t  -di:  Variable order should be 'decrement-increment'\n");
  printf("\t  -id:  Variable order should be 'increment-decrement' (default)\n");
  return 1;
}

int main(int argc, const char** argv)
{
  MEDDLY::initialize();

  int red = 11;
  int blue = 11;
  forest::policies p(false);
  p.setOptimistic();
  bool decinc = false;
  bool saturate = true;
  for (int i=1; i<argc; i++) {
    if (strcmp("-bfs", argv[i])==0) {
        saturate = false;
        continue;
    }
    if (strcmp("-dfs", argv[i])==0) {
        saturate = true;
        continue;
    }
    if (strcmp("-opt", argv[i])==0) {
        p.setOptimistic();
        continue;
    }
    if (strcmp("-pess", argv[i])==0) {
        p.setPessimistic();
        continue;
    }
    if (strcmp("-r", argv[i])==0) {
        i++;
        red = atoi(argv[i]);
        if (red>=0) continue;
    }
    if (strcmp("-b", argv[i])==0) {
        i++;
        blue = atoi(argv[i]);
        if (blue>=0) continue;
    }
    if (strcmp("-di", argv[i])==0) {
      decinc = true;
      continue;
    }
    if (strcmp("-id", argv[i])==0) {
      decinc = false;
      continue;
    }
    return usage(argv[0]);
  }

  if (decinc) decinc_order(red, blue);
  else        incdec_order(red, blue);

  //
  // Build the domain
  //
  int scratch[1+LEVELS];
  for (int k=0; k<LEVELS; k++) scratch[k] = COLORS;
  domain* D = createDomainBottomUp(scratch, LEVELS);
 
  //
  // Build NSF for possible "1-step" moves
  //
  timer watch;
  forest* mxd = D->createForest(true, forest::BOOLEAN, forest::MULTI_TERMINAL);
  dd_edge nsf(mxd);

  watch.note_time();
  buildNSF(nsf);
  watch.note_time();

  printStats("Next-state function", watch, nsf);

  //
  // Build initial configuration
  //
  forest* mdd = D->createForest(false, forest::BOOLEAN, forest::MULTI_TERMINAL, p);
  dd_edge initial(mdd);
  for (int i=0; i<leftring; i++)  scratch[leftvar[i]]   = leftinit[i];
  for (int i=0; i<rightring; i++) scratch[rightvar[i]]  = rightinit[i];
  const int* foo = scratch;
  mdd->createEdge(&foo, 1, initial);

  //
  // Saturate!
  //
  printf("Building reachability set using %s\n",
    saturate ? "saturation" : "traditional algorithm");
  fflush(stdout);
  dd_edge reachable(mdd);

  watch.note_time();
  if (saturate) apply(REACHABLE_STATES_DFS, initial, nsf, reachable);
  else          apply(REACHABLE_STATES_BFS, initial, nsf, reachable);
  watch.note_time();

  printStats("Reachability set", watch, reachable);

  //
  // Determine cardinality
  //
  double c;
  apply(CARDINALITY, reachable, c);
  printf("Counted (approx) %g reachable states\n", c);

  double th = theoretical();
  printf("Theory: (approx) %g reachable states\n", th);

  //
  // Cleanup
  //
  FILE_output meddlyout(stdout);
  operation::showAllComputeTables(meddlyout, 3);
  MEDDLY::cleanup();
  return 0;
}
