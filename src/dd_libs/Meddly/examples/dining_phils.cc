
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
  State space generation for Dining Philosphers (N=2).

  The model has 2 philosophers and 2 forks.

  Each philosopher can be in state {I, W, L, R, E} where
  
  I:    idle philosopher
  WB:   philosopher is waiting for both forks
  HL:   philosopher has left fork
  HR:   philosopher has right fork
  E:    philosopher is eating

  Each fork can be in state {A, NA} where
  
  A:    fork is available
  NA:   fork is not available

  Philosphers can move from one state to another as:
  
  I -> WB
 
  The synchronization between philosopher 1 and the forks:

  WB1 ->  HR1
  A1  ->  NA1

  WB1 ->  HL1
  A2  ->  NA2

  HR1 ->  E1
  A2  ->  NA2

  HL1 ->  E1
  A1  ->  NA1

  E1  ->  I1
  NA1 ->  A1
  NA2 ->  A2

  The synchronization between philosopher 2 and the forks:

  WB2 ->  HR2
  A2  ->  NA2

  WB2 ->  HL2
  A1  ->  NA1

  HR2 ->  E2
  A1  ->  NA1

  HL2 ->  E2
  A2  ->  NA2

  E2  ->  I2
  NA1 ->  A1
  NA2 ->  A2

  Initially, all forks are in state "A" and all philosophers are in state "I".
  How many reachable states?
  Exceptions?
*/

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>

#include "../config.h"
#if HAVE_LIBGMP
  #include <gmp.h>
#endif

#include "meddly.h"
#include "meddly_expert.h"
#include "timer.h"
#include "loggers.h"


using namespace MEDDLY;

// #define NAME_VARIABLES
// #define SHOW_MXD
// #define SHOW_MDD

// Specify the variable order, top down:
enum varorder {
  fpfp,       // fork, phil, fork, phil, ...
  pfpf,       // phil, fork, phil, fork, ...
  ppff,       // phil, ..., phil, fork, ..., fork
  ffpp        // fork, ..., fork, phil, ..., phil
};

struct switches {
  bool pessimistic;
  bool exact;
  char method;
  // bool chaining;
  bool printReachableStates;
  varorder vord;

public:
  switches() {
    pessimistic = false;
    exact = false;
    method = 'b';
    // chaining = true;
    printReachableStates = false;
    vord = fpfp;
  }
};

class model2var {
  public:
    model2var(varorder vo, int phils);

    inline int philVar(int p) const {
      switch (vord) {

        case ffpp:    return p+1;

        case ppff:    return nPhils+p+1;

        case pfpf:    return 2*p +2;  // 0th philosopher at 2nd variable

        case fpfp:
        default:      return 2*p +1;  // 0th philosopher at 1st variable
      }
    }

    inline int forkVar(int p) const {
      switch (vord) {

        case ffpp:    return nPhils+p+1;

        case ppff:    return p+1;

        case pfpf:    return 2*p +1;  // 0th fork at 1st variable

        case fpfp:
        default:      return 2*p +2;  // 0th fork at 2nd variable
      }

    }

    inline int leftVar(int p) const {
      return forkVar( (p + nPhils - 1) % nPhils );
    }
    inline int rightVar(int p) const {
      return forkVar(p);
    }

    inline int numPhils() const {
      return nPhils;
    }

    inline int numLevels() const {
      return 2*nPhils;
    }
  
  private:
    varorder vord;
    int nPhils;
};


model2var::model2var(varorder vo, int phils) : vord(vo)
{
  nPhils = phils;
}

class philsModel {
  public:
    philsModel(int nPhils, const model2var& m2v, forest* mxd);
    ~philsModel();

    // event builders
    inline void setPhilosopher(int phil) {
      ph = M2V.philVar(phil);
      rf = M2V.rightVar(phil);
      lf = M2V.leftVar(phil);
    };

    void Idle2WaitBoth(dd_edge &e);
    void WaitBoth2HaveRight(dd_edge &e);
    void WaitBoth2HaveLeft(dd_edge &e);
    void HaveRight2Eat(dd_edge &e);
    void HaveLeft2Eat(dd_edge &e);
    void Eat2Idle(dd_edge &e);

    // build everything for a given phil
    void eventsForPhil(int phil, dd_edge &e);

  private:
    inline void setMinterm(int* m, int c) {
      for (int i = 1; i<sz; i++) m[i] = c;
    }

  private:
    int* from;
    int* to;
    int nPhils;
    int sz;
    forest* mxd;

    int ph;
    int rf;
    int lf;

    const model2var& M2V;
};


philsModel::philsModel(int nP, const model2var& m2v, forest* _mxd)
: M2V(m2v)
{
  nPhils = nP;
  mxd = _mxd;

  sz = nPhils * 2 + 1;

  from = new int[sz];
  to = new int[sz];

  from[0] = to[0] = 0;

  setMinterm(from, DONT_CARE);
  setMinterm(to, DONT_CHANGE);

  // we always set these arrays back when we're done :^)
}

philsModel::~philsModel()
{
  delete[] from;
  delete[] to;
}

void philsModel::Idle2WaitBoth(dd_edge &e)
{
  /* I(ph) -> WB(ph) */
  from[ph] = 0;
  to[ph] = 1;
  mxd->createEdge(&from, &to, 1, e);

  from[ph] = DONT_CARE;
  to[ph] = DONT_CHANGE;
}

void philsModel::WaitBoth2HaveRight(dd_edge &e)
{
  /* WB(ph) -> HR(ph), A(rf) -> NA(rf) */
  from[ph] = 1;
  from[rf] = 0;
  to[ph] = 3;
  to[rf] = 1;
  mxd->createEdge(&from, &to, 1, e);

  from[ph] = DONT_CARE;
  from[rf] = DONT_CARE;
  to[ph] = DONT_CHANGE;
  to[rf] = DONT_CHANGE;
}

void philsModel::WaitBoth2HaveLeft(dd_edge &e)
{
  /* WB(ph) -> HR(ph), A(lf) -> NA(lf) */
  from[lf] = 0;
  from[ph] = 1;
  to[lf] = 1;
  to[ph] = 2;
  mxd->createEdge(&from, &to, 1, e);

  from[lf] = DONT_CARE;
  from[ph] = DONT_CARE;
  to[lf] = DONT_CHANGE;
  to[ph] = DONT_CHANGE;
}

void philsModel::HaveRight2Eat(dd_edge &e)
{
  /* HR(ph) -> E(ph), A(lf) -> NA(lf) */
  from[lf] = 0;
  from[ph] = 3;
  to[lf] = 1;
  to[ph] = 4;
  mxd->createEdge(&from, &to, 1, e);

  from[lf] = DONT_CARE;
  from[ph] = DONT_CARE;
  to[lf] = DONT_CHANGE;
  to[ph] = DONT_CHANGE;
}

void philsModel::HaveLeft2Eat(dd_edge &e)
{
  /* HL(ph) -> E(ph), A(rf) -> NA(rf) */
  from[ph] = 2;
  from[rf] = 0;
  to[ph] = 4;
  to[rf] = 1;
  mxd->createEdge(&from, &to, 1, e);

  from[ph] = DONT_CARE;
  from[rf] = DONT_CARE;
  to[ph] = DONT_CHANGE;
  to[rf] = DONT_CHANGE;
}

void philsModel::Eat2Idle(dd_edge &e)
{
  /* E(ph) -> I(ph), NA(rf) -> A(rf), NA(lf) -> A(lf) */
  from[lf] = 1;
  from[ph] = 4;
  from[rf] = 1;
  to[lf] = 0;
  to[ph] = 0;
  to[rf] = 0;
  mxd->createEdge(&from, &to, 1, e);

  from[lf] = DONT_CARE;
  from[ph] = DONT_CARE;
  from[rf] = DONT_CARE;
  to[lf] = DONT_CHANGE;
  to[ph] = DONT_CHANGE;
  to[rf] = DONT_CHANGE;
}

void philsModel::eventsForPhil(int phil, dd_edge &e)
{
  setPhilosopher(phil);
  dd_edge temp(mxd);
  Idle2WaitBoth(e);
  WaitBoth2HaveRight(temp);
  e += temp;
  WaitBoth2HaveLeft(temp);
  e += temp;
  HaveRight2Eat(temp);
  e += temp;
  HaveLeft2Eat(temp);
  e += temp;
  Eat2Idle(temp);
  e += temp;
}


void printStats(const char* who, const forest* f)
{
  printf("%s stats:\n", who);
  const expert_forest* ef = (expert_forest*) f;
  FILE_output mout(stdout);
  ef->reportStats(mout, "\t",
    expert_forest::HUMAN_READABLE_MEMORY  |
    expert_forest::BASIC_STATS | expert_forest::EXTRA_STATS |
    expert_forest::STORAGE_STATS | expert_forest::HOLE_MANAGER_STATS
  );
}

variable** initializeVariables(const model2var &M2V)
{
  // set bounds for each variable
  // forks: 2 states, philosophers: 5 states
  variable** vars = (variable**) malloc((1+M2V.numLevels()) * sizeof(void*));
  for (int i=0; i<=M2V.numLevels(); i++) {
    vars[i] = 0;
  }

#ifdef NAME_VARIABLES
  char buffer[32];
#endif
  
  for (int i=0; i<M2V.numPhils(); i++) {

    /*
        Create ith fork
    */

    int v = M2V.forkVar(i);

    if (vars[v]) {
      fprintf(stderr, "Error: forkVar(%d) = %d index in use\n", i, v);
      exit(1);
    }

    char* name = 0;
#ifdef NAME_VARIABLES
    buffer[0] = 0;
    snprintf(buffer, 32, "fork%d", i);
    name = strdup(buffer);
#endif
    vars[v] = createVariable(2, name);

    /*
        Create ith philosopher
    */

    v = M2V.philVar(i);

    if (vars[v]) {
      fprintf(stderr, "Error: philVar(%d) = %d index in use\n", i, v);
      exit(1);
    }

    name = 0;
#ifdef NAME_VARIABLES
    buffer[0] = 0;
    snprintf(buffer, 32, "phil%d", i);
    name = strdup(buffer);
#endif
    vars[v] = createVariable(5, name);

  } // for i

  return vars;
}


int* initializeInitialState(int nLevels)
{
  // initial state -- all levels at 0
  int *initialState = (int *) malloc((nLevels + 1) * sizeof(int));
  memset(initialState, 0, (nLevels + 1) * sizeof(int));
  return initialState;
}



// Test Index Set
void testIndexSet(const dd_edge& mdd, dd_edge& indexSet)
{
  apply(CONVERT_TO_INDEX_SET, mdd, indexSet);

  FILE_output mout(stdout);
#if 1
  indexSet.show(mout, 3);
#else
  indexSet.show(mout, 1);
#endif
}

domain* runWithOptions(int nPhilosophers, const switches &sw, forest::logger* LOG)
{
  timer start;
  FILE_output meddlyout(stdout);

  // Number of levels in domain (excluding terminals)
  int nLevels = nPhilosophers * 2;


  const char* order_description = 0;
  switch (sw.vord) {
    case pfpf:  order_description = "phil, fork, phil, fork, ...";
                break;
    case fpfp:  order_description = "fork, phil, fork, phil, ...";
                break;
    case ppff:  order_description = "phil, ..., phil, fork, ..., fork";
                break;
    case ffpp:  order_description = "fork, ..., fork, phil, ..., phil";
                break;
    default:    order_description = "unknown order?";
  }
  printf("Using variable order (from top down): %s\n", order_description);
  model2var M2V(sw.vord, nPhilosophers);

  // Set up arrays bounds based on nPhilosophers
  variable** vars = initializeVariables(M2V);

  printf("Initiailzing forests\n");

  // Create a domain and set up the state variables.
  domain *d = createDomain(vars, M2V.numLevels());
  assert(d != NULL);

  // Set up MDD options
  forest::policies pmdd(false);
  if (sw.pessimistic) pmdd.setPessimistic();
  else                pmdd.setOptimistic();

  // Create an MDD forest in this domain (to store states)
  forest* mdd =
    d->createForest(false, forest::BOOLEAN, forest::MULTI_TERMINAL, pmdd);
  assert(mdd != NULL);
  mdd->setLogger(LOG, "MDD");

  // Set up MXD options
  forest::policies pmxd(true);
  if (sw.pessimistic) pmdd.setPessimistic();
  else                pmdd.setOptimistic();

  // Create a MXD forest in domain (to store transition diagrams)
  forest* mxd = 
    d->createForest(true, forest::BOOLEAN, forest::MULTI_TERMINAL, pmxd);
  assert(mxd != NULL);
  mxd->setLogger(LOG, "MxD");

  // Set up initial state array based on nPhilosophers
  if (LOG) LOG->newPhase(mdd, "Building initial state");
  int *initSt = initializeInitialState(nLevels);
  int *addrInitSt[1] = { initSt };
  dd_edge initialStates(mdd);
  mdd->createEdge(reinterpret_cast<int**>(addrInitSt), 1, initialStates);

  if (LOG) LOG->newPhase(mxd, "Building next-state function");
  printf("Building next-state function for %d dining philosophers\n", 
          nPhilosophers);
  fflush(stdout);
  start.note_time();

  // Create a matrix diagram to represent the next-state function
  // Next-State function is computed by performing a union of next-state
  // functions that each represent how a philosopher's state can change.
  philsModel model(nPhilosophers, M2V, mxd);
  dd_edge nsf(mxd);
  satpregen_opname::pregen_relation* ensf = 0;
  specialized_operation* sat = 0;

  if ('s' == sw.method) {
    ensf = new satpregen_opname::pregen_relation(mdd, mxd, mdd, 6*nPhilosophers);
  }
  if ('k' == sw.method) {
    ensf = new satpregen_opname::pregen_relation(mdd, mxd, mdd);
  }

  if (ensf) {
    dd_edge temp(mxd);
    for (int i = 0; i < nPhilosophers; i++) {
      model.setPhilosopher(i);
      model.Idle2WaitBoth(temp);
      ensf->addToRelation(temp);
      model.WaitBoth2HaveRight(temp);
      ensf->addToRelation(temp);
      model.WaitBoth2HaveLeft(temp);
      ensf->addToRelation(temp);
      model.HaveRight2Eat(temp);
      ensf->addToRelation(temp);
      model.HaveLeft2Eat(temp);
      ensf->addToRelation(temp);
      model.Eat2Idle(temp);
      ensf->addToRelation(temp);
    }
    ensf->finalize();
  } else {
    dd_edge phil(mxd);
    for (int i = 0; i < nPhilosophers; i++) {
      model.eventsForPhil(i, phil);
      nsf += phil;
    }
  }
  start.note_time();
  printf("Next-state function construction took %.4e seconds\n",
          start.get_last_interval()/1000000.0);
  if (!ensf) {
    printf("Next-state function MxD has\n\t%d nodes\n\t\%d edges\n",
      nsf.getNodeCount(), nsf.getEdgeCount());
  }

  //
  // Show stats for nsf construction
  //
  printStats("MxD", mxd);

#ifdef SHOW_MXD
  printf("Next-State Function:\n");
  nsf.show(meddlyout, 2);
#endif


  //
  // Build reachable states
  //
  if (LOG) LOG->newPhase(mdd, "Building reachability set");
  dd_edge reachableStates(initialStates);
  start.note_time();

  switch (sw.method) {
    case 'b':
        printf("Building reachability set using traditional algorithm\n");
        fflush(stdout);
        apply(REACHABLE_STATES_BFS, initialStates, nsf, reachableStates);
        break;

    case 'm':
        printf("Building reachability set using saturation, monolithic relation\n");
        fflush(stdout);
        apply(REACHABLE_STATES_DFS, initialStates, nsf, reachableStates);
        break;

    case 'k':
    case 's':
        printf("Building reachability set using saturation, relation");
        if ('k'==sw.method) printf(" by levels\n");
        else                printf(" by events\n");
        fflush(stdout);
        if (0==SATURATION_FORWARD) {
          throw error(error::UNKNOWN_OPERATION);
        }
        sat = SATURATION_FORWARD->buildOperation(ensf);
        if (0==sat) {
          throw error(error::INVALID_OPERATION);
        }
        sat->compute(initialStates, reachableStates);
        break;

    default:
        printf("Error - unknown method\n");
        exit(2);
  };
  start.note_time();
  printf("Done\n");

  printf("Reachability set construction took %.4e seconds\n",
          start.get_last_interval()/1000000.0);
  fflush(stdout);
  printf("#Nodes: %d\n", reachableStates.getNodeCount());
  printf("#Edges: %d\n", reachableStates.getEdgeCount());

#ifdef SHOW_MDD
  printf("Reachability set:\n");
  reachableStates.show(meddlyout, 2);
#endif

  // Show stats for rs construction
  printStats("MDD", mdd);
  
  operation::showAllComputeTables(meddlyout, 3);

  double c;
  apply(CARDINALITY, reachableStates, c);
  printf("Approximately %e reachable states\n", c);
  fflush(stdout);

#if HAVE_LIBGMP
  if (sw.exact) {
    mpz_t nrs;
    mpz_init(nrs);
    apply(CARDINALITY, reachableStates, nrs);
    printf("Exactly ");
    mpz_out_str(0, 10, nrs);
    printf(" reachable states\n");
    fflush(stdout);
    mpz_clear(nrs);
  }
#endif

  if (sw.printReachableStates) {
    // Create a EV+MDD forest in this domain (to store index set)
    forest* evplusmdd =
      d->createForest(false, forest::INTEGER, forest::INDEX_SET);
    assert(evplusmdd != NULL);

    // Test Convert MDD to Index Set EV+MDD
    dd_edge indexSet(evplusmdd);
    testIndexSet(reachableStates, indexSet);
    int* element = (int *) malloc((nLevels + 1) * sizeof(int));

    double cardinality = indexSet.getCardinality();
    for (int index = 0; index < int(cardinality); index++)
    {
      evplusmdd->getElement(indexSet, index, element);
      printf("Element at index %d: [ ", index);
      for (int i = nLevels; i > 0; i--)
      {
        printf("%d ", element[i]);
      }
      printf("]\n");
    }
  }

  if (false) {
    start.note_time();
    unsigned counter = 0;
    for (enumerator iter(reachableStates); 
        iter; ++iter, ++counter)
    {
      const int* element = iter.getAssignments();
      assert(element != 0);
#if 1
      printf("%d: [", counter);
      for (int i = 2*nPhilosophers; i > 0; --i)
      {
        printf("%d ", element[i]);
      }
      printf("]\n");
#endif
    }
    start.note_time();
    printf("Iterator traversal time (%0.4e elements): %0.4e seconds\n",
        double(counter), start.get_last_interval()/double(1000000.0));
  }

  /*
    Next will be cleanup.
    Log that.
  */
  if (LOG) {
    LOG->newPhase(mdd, "Cleanup");
    LOG->newPhase(mxd, "Cleanup");
  }

  return d;
}



int usage(const char* who)
{
  /* Strip leading directory, if any: */
  const char* name = who;
  for (const char* ptr=who; *ptr; ptr++) {
    if ('/' == *ptr) name = ptr+1;
  }

  printf("\nUsage: %s [options]\n\n", name);

  printf("\t-n<#phils>: set number of philosophers\n\n");

  printf("\t-exact:     display the exact number of states\n");
  printf("\t-cs<cache>: set cache size (0 for library default)\n");
  printf("\t-pess:      use pessimistic node deletion (lower mem usage)\n\n");

  printf("\t-bfs:       use traditional iterations (default)\n\n");
  printf("\t-dfs:       use fastest saturation (currently, -msat)\n");
  printf("\t-esat:      use saturation by events\n");
  printf("\t-ksat:      use saturation by levels\n");
  printf("\t-msat:      use monolithic saturation\n");
  printf("\t -l lfile:  Write logging information to specified file\n\n");
  printf("\t-ofpfp:     (default) Variable order: fork, phil, fork, phil, ...\n");
  printf("\t-opfpf:     Variable order: phil, fork, phil, fork, ...\n");
  printf("\t-oppff:     Variable order: phil, ..., phil, fork, ..., fork\n");
  printf("\t-offpp:     Variable order: fork, ..., fork, phil, ..., phil\n");
  printf("\n");
  return 0;
}


int main(int argc, char *argv[])
{
  int nPhilosophers = 0; // number of philosophers
  switches sw;
  int cacheSize = 0;
  const char* lfile = 0;

  for (int i=1; i<argc; i++) {
    const char* cmd = argv[i];
    if (strcmp(cmd, "-pess") == 0) {
      sw.pessimistic = true;
      continue;
    }
    if (strncmp(cmd, "-cs", 3) == 0) {
      cacheSize = strtol(&cmd[3], NULL, 10);
      if (cacheSize < 1) {
        return 1+usage(argv[0]);
      }
      continue;
    }
    if (strcmp(cmd, "-exact") == 0) {
      sw.exact = true;
      continue;
    }
    if (strcmp(cmd, "-print") == 0) {
      sw.printReachableStates = true;
      continue;
    }
    if (strncmp(cmd, "-n", 2) == 0) {
      nPhilosophers = strtol(cmd+2, NULL, 10);
      if (nPhilosophers < 1) {
        return 1+usage(argv[0]);
      }
      continue;
    }

    if (strcmp(cmd, "-bfs") == 0) {
      sw.method = 'b';
      continue;
    }

    if (strcmp(cmd, "-dfs") == 0) {
      sw.method = 'm';
      continue;
    }

    if (strcmp(cmd, "-msat") == 0) {
      sw.method = 'm';
      continue;
    }
    if (strcmp(cmd, "-esat") == 0) {
      sw.method = 's';
      continue;
    }
    if (strcmp(cmd, "-ksat") == 0) {
      sw.method = 'k';
      continue;
    }
    if (strcmp("-l", argv[i])==0) {
      lfile = argv[i+1];
      i++;
      continue;
    }

    if (strcmp(cmd, "-ofpfp") == 0) {
      sw.vord = fpfp;
      continue;
    }
    if (strcmp(cmd, "-opfpf") == 0) {
      sw.vord = pfpf;
      continue;
    }
    if (strcmp(cmd, "-oppff") == 0) {
      sw.vord = ppff;
      continue;
    }
    if (strcmp(cmd, "-offpp") == 0) {
      sw.vord = ffpp;
      continue;
    }

    return 1+usage(argv[0]);
  }

  while (nPhilosophers < 2) {
    printf("Enter the number of philosophers (at least 2): ");
    scanf("%d", &nPhilosophers);
  }

  // Initialize MEDDLY

  initializer_list* L = defaultInitializerList(0);
  if (cacheSize > 0) {
    ct_initializer::setMaxSize(cacheSize);
  }
  MEDDLY::initialize(L);

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
      LOG->recordTimeStamps();
      char comment[80];
      snprintf(comment, 80, "Automatically generated by dining_phils (N=%d)", 
        nPhilosophers);
      LOG->addComment(comment);
    }
  }

  try {
    domain* d = runWithOptions(nPhilosophers, sw, LOG);
    if (LOG) {
      MEDDLY::destroyDomain(d);
      delete LOG;
    }
    MEDDLY::cleanup();
    printf("\n\nDONE\n");
    return 0;
  }
  catch (error e) {
    printf("Caught MEDDLY error: %s\n", e.getName());
    return 1;
  }
}

