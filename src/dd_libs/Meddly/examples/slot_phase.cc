
// $Id: slot_phase.cc 722 2016-10-02 01:16:29Z cjiang1209 $

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

#include <cstdio>
#include <cstdlib>
#include <string.h>
#include <fstream>
#include <vector>
#include <algorithm>
#include <iostream>

#include "meddly.h"
#include "meddly_expert.h"
#include "simple_model.h"
#include "timer.h"
#include "loggers.h"

using namespace MEDDLY;
using namespace std;

FILE_output meddlyout(stdout);

int usage(const char* who)
{
  /* Strip leading directory, if any: */
  const char* name = who;
  for (const char* ptr=who; *ptr; ptr++) {
    if ('/' == *ptr) name = ptr+1;
  }
  printf("\nUsage: %s nnnn [options]\n\n", name);
  printf("\tnnnn: number of parts\n\n");
  printf("\t-bfs: use traditional iterations\n\n");
  printf("\t-dfs: use fastest saturation (currently, -msat)\n");
  printf("\t-esat: use saturation by events\n");
  printf("\t-ksat: use saturation by levels\n");
  printf("\t-msat: use monolithic saturation (default)\n\n");
  printf("\t-exp: use explicit (very slow)\n");
  printf("\t--batch b: specify explicit batch size\n\n");
  printf("\t -l lfile: Write logging information to specified file\n\n");
  return 1;
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



inline char* newEvent(int N)
{
  char* ev = new char[N*8+1];
  for (int i=N*8; i; i--) ev[i] = '.';
  ev[0] = '_';
  return ev;
}

char* Get(int i, int N)
{
  char* t = newEvent(N);
  char* tloc = t+8*i;
  tloc[6] = '-';
  tloc[8] = '-';
  tloc[3] = '+';
  tloc[5] = '+';
  return t;
}

char* Free(int i, int N)
{
  char* t = newEvent(N);
  char* tloc = t+8*i;
  char* t_rt = i ? t+(8*i-8) : t+(8*N-8);
  tloc[5] = '-';
  tloc[6] = '+';
  t_rt[3] = '-';
  t_rt[2] = '+';
  return t;
}

char* Put(int i, int N)
{
  char* t = newEvent(N);
  char* tloc = t+8*i;
  tloc[3] = '+';
  tloc[7] = '+';
  tloc[4] = '-';
  tloc[6] = '-';
  return t;
}

char* Used(int i, int N)
{
  char* t = newEvent(N);
  char* tloc = t+8*i;
  char* t_rt = i ? t+(8*i-8) : t+(8*N-8);
  tloc[7] = '-';
  tloc[6] = '+';
  t_rt[3] = '-';
  t_rt[1] = '+';
  return t;
}

char* Other(int i, int N)
{
  char* t = newEvent(N);
  char* tloc = t+8*i;
  tloc[1] = '-';
  tloc[4] = '+';
  return t;
}

char* Owner(int i, int N)
{
  char* t = newEvent(N);
  char* tloc = t+8*i;
  tloc[1] = '-';
  tloc[2] = '+';
  return t;
}

char* Write(int i, int N)
{
  char* t = newEvent(N);
  char* tloc = t+8*i;
  tloc[2] = '-';
  tloc[4] = '+';
  return t;
}

char* Go(int i, int N)
{
  char* t = newEvent(N);
  char* tloc = t+8*i;
  tloc[2] = '-';
  tloc[8] = '+';
  return t;
}

void show_node(const dd_edge& e)
{
  static_cast<expert_forest*>(e.getForest())->removeAllComputeTableEntries();

  cout << "# Nodes: " << e.getForest()->getCurrentNumNodes() << endl;
  cout << "# Peak Nodes: " << e.getForest()->getPeakNumNodes() << endl;
//  FILE_output out(stdout);
//  e.show(out, 2);
  cout << "# States: " << e.getCardinality() << endl;
}

double reorder_time = 0;

void execute_phase(const dd_edge& initial, const dd_edge& nsf, dd_edge& result, bool reorder)
{
  timer start;

  if (reorder) {
    start.note_time();

    cout << "Reordering..." << endl;

    initial.getForest()->resetPeakNumNodes();

    expert_forest* relation = static_cast<expert_forest*>(nsf.getForest());
    int* rel_level2var = new int[relation->getNumVariables() + 1];
    relation->getVariableOrder(rel_level2var);

    expert_forest* state = static_cast<expert_forest*>(initial.getForest());

    state->reorderVariables(rel_level2var);
    delete[] rel_level2var;

    start.note_time();
    cout << "Time: "
        << static_cast<double>(start.get_last_interval()) / 1000000.0 << " s"
        << endl;
    show_node(initial);

    reorder_time += static_cast<double>(start.get_last_interval()) / 1000000.0;
  }

  cout << "Computing the reachable states..." << endl;

  initial.getForest()->resetPeakNumNodes();

  start.note_time();
  apply(REACHABLE_STATES_DFS, initial, nsf, result);
  start.note_time();

  cout << "Time: "
      << static_cast<double>(start.get_last_interval()) / 1000000.0 << " s"
      << endl;
}

void runWithArgs(int N, char method, int batchsize, forest::logger* LOG)
{
  int num_phases = 3;
  bool reorder = true;

  if (N % num_phases != 0) {
    exit(0);
  }

  timer start;

  printf("+-------------------------------------------------+\n");
  printf("|   Initializing Slotted ring model for N = %-4d  |\n", N);
  printf("+-------------------------------------------------+\n");
  fflush(stdout);

  char** events = new char*[8 * N];
  char** fill = events;
  for (int i = 0; i < N; i++) {
    fill[0] = Other(i, N);
    fill[1] = Owner(i, N);
    fill[2] = Write(i, N);
    fill[3] = Go(i, N);
    fill[4] = Get(i, N);
    fill[5] = Put(i, N);
    fill[6] = Used(i, N);
    fill[7] = Free(i, N);
    fill += 8;
  }

  // Initialize domain
  int* sizes = new int[N * 8];
  for (int i = N * 8 - 1; i >= 0; i--) {
    sizes[i] = 2;
  }
  domain* d = createDomainBottomUp(sizes, N * 8);
  delete[] sizes;

  // Initialize forests
  forest::policies p(false);
  p.setSinkDown();
  forest* mdd = d->createForest(0, forest::BOOLEAN, forest::MULTI_TERMINAL);

  std::vector<forest*> mxds;
  for (int i = 0; i < num_phases; i++) {
    forest* mxd = d->createForest(1, forest::BOOLEAN, forest::MULTI_TERMINAL);
    mxds.push_back(mxd);
  }

//  forest* mxd = d->createForest(1, forest::BOOLEAN, forest::MULTI_TERMINAL);
  if (LOG) {
    mdd->setLogger(LOG, "MDD");
    //mxd->setLogger(LOG, "MxD");
  }

  //
  // Build initial state
  //
  if (LOG) LOG->newPhase(mdd, "Building initial state");
  int* initial = new int[1 + N * 8];
  for (int i = N * 8; i; i--) initial[i] = 0;
  int* initLocal = initial;
  for (int i = 0; i < N; i++) {
    initLocal[3] = initLocal[5] = 1;
    initLocal += 8;
  }
  dd_edge init_state(mdd);
  mdd->createEdge(&initial, 1, init_state);

  //
  // Build next-state function
  //

  std::vector<dd_edge> nsfs;

  start.note_time();

  char** itr = events;
  for (int i = 0; i < num_phases; i++) {
    if (LOG) LOG->newPhase(mxds[i], "Building next-state function");

    dd_edge nsf(mxds[i]);
    buildNextStateFunction(itr, N / num_phases * 8, mxds[i], nsf, 2);
    nsfs.push_back(nsf);
    show_node(nsf);

    itr += (N / num_phases) * 8;
  }

  if (reorder) {
    int* level2var = new int[N * 8 + 1];
    for (int i = 0; i < N * 8 + 1; i++) {
      level2var[i] = i;
    }
    std::rotate(level2var + 1, level2var + ((N - 1) * 8 + 1), level2var + (N * 8 + 1));

    for (auto& nsf : nsfs) {
      static_cast<expert_forest*>(nsf.getForest())->reorderVariables(level2var);
      std::rotate(level2var + 1, level2var + ((N / num_phases) * 8 + 1), level2var + (N * 8 + 1));
    }

    static_cast<expert_forest*>(nsfs.back().getForest())->getVariableOrder(level2var);
    static_cast<expert_forest*>(mdd)->reorderVariables(level2var);

    delete[] level2var;
  }

  for (int i = 0; i < 8 * N; i++) {
    delete[] events[i];
  }
  delete[] events;

  //        buildNextStateFunction(events, 8*N, mxd, nsf, 2);
  start.note_time();
  printf("Next-state function construction took %.4e seconds\n",
      start.get_last_interval() / 1000000.0);
  //    printStats("MxD", mxd);

  //
  // Build reachable states
  //
  if (LOG) LOG->newPhase(mdd, "Building reachability set");

  int phase = 0;

  start.note_time();

  // Fixed point
  int fp = 0;
  while (fp != num_phases) {
    cout << "-------------------------------------------------------------------" << endl;
    cout << "Phase " << phase << endl;

    dd_edge result = init_state;
    execute_phase(init_state, nsfs[phase % num_phases], result, reorder);
    phase++;

    if (result != init_state) {
      fp = 0;
      init_state = result;
    }
    else {
      fp++;
    }

    show_node(init_state);
  }

  start.note_time();
  printf("Done\n");
  printf("Reachability set construction took %.4e seconds\n",
          start.get_last_interval()/1000000.0);
  printf("Reordering took %.4e seconds\n", reorder_time);
  fflush(stdout);

#ifdef SHOW_STATES
  int count = 0;
  for (enumerator i(reachable); i; ++i, ++count) {
    const int* element = i.getAssignments();
    printf("State %4d: [%d", count, element[1]);
    for (int j=2; j<=8*N; j++) {
      printf(", %d", element[j]);
    } // for j
    printf("]\n");
  }  // for i
#endif

  printStats("MDD", mdd);
  fflush(stdout);

  double c;
  apply(CARDINALITY, init_state, c);
  
  printf("Approx. %g reachable states\n", c);
  // or, don't, and let cleanup() take care of it?

  if (LOG) {
    LOG->newPhase(mdd, "Cleanup");
    MEDDLY::destroyDomain(d);
  }
}

int main(int argc, const char** argv)
{
  int N = -1;
  char method = 'm';
  int batchsize = 256;
  const char* lfile = 0;

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
    if (strcmp("-exp", argv[i])==0) {
      method = 'e';
      continue;
    }
    if (strcmp("-l", argv[i])==0) {
      lfile = argv[i+1];
      i++;
      continue;
    }
    if (strcmp("--batch", argv[i])==0) {
      i++;
      if (argv[i]) batchsize = atoi(argv[i]);
      continue;
    }
    N = atoi(argv[i]);
  }

  if (N<0) return usage(argv[0]);

  MEDDLY::initialize();

  //
  // Set up logger, if any
  //

  std::ofstream log;
  forest::logger* LOG = 0;
  if (lfile) {
    log.open(lfile, std::ofstream::out);
    if (!log) {
      printf("Couldn't open %s for writing, no logging\n", lfile);
    } else {
      LOG = new simple_logger(log);
      LOG->recordNodeCounts();
      char comment[80];
      snprintf(comment, 80, "Automatically generated by slot (N=%d)", N);
      LOG->addComment(comment);
    }
  }

  try {
    runWithArgs(N, method, batchsize, LOG);
    delete LOG;
    MEDDLY::cleanup();
    return 0;
  }
  catch (MEDDLY::error e) {
    printf("Caught MEDDLY error: %s\n", e.getName());
    return 1;
  }

}


