
// $Id$

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

#include "meddly.h"
#include "simple_model.h"

// #define DEBUG_EVENTS
// #define VERBOSE
// #define TEST_EVTIMES

inline int MAX(int a, int b) {
  return (a>b) ? a : b;
}

void buildNextStateFunction(const char* const* events, int nEvents,
  MEDDLY::forest* mxd, MEDDLY::dd_edge &nsf)
{
  using namespace MEDDLY;
#ifdef VERBOSE
  fprintf(stderr, "Building next-state function\n");
#endif

  // set up auxiliary mtmxd forest and edges
  domain* d = mxd->useDomain();

  int nVars = d->getNumVariables();
  int maxBound = d->getVariableBound(1, false);
  for (int i=2; i<=nVars; i++) {
    maxBound = MAX(maxBound, d->getVariableBound(i, false));
  }
  maxBound++;
  int* minterm = new int[nVars+1];
  int* mtprime = new int[nVars+1];
  dd_edge** varP  = new dd_edge*[nVars+1];
  varP[0] = 0;
  dd_edge** inc   = new dd_edge*[nVars+1];
  inc[0] = 0;
  dd_edge** dec   = new dd_edge*[nVars+1];
  dec[0] = 0;

#ifdef TEST_EVTIMES
  forest* mtmxd = d->createForest(
    true, forest::REAL, forest::EVTIMES
  );
  float* temp = new float[maxBound];
#else
  forest* mtmxd = d->createForest(
    true, forest::INTEGER, forest::MULTI_TERMINAL
  );
  int* temp = new int[maxBound];
#endif


  //  Create edge for each variable xi'
  for (int i=1; i<=nVars; i++) {
    varP[i] = new dd_edge(mtmxd);
    mtmxd->createEdgeForVar(i, true, varP[i][0]);
  }

  // Create edge for each function xi+1
  for (int i=0; i<maxBound; i++) temp[i] = i+1;
  for (int i=1; i<=nVars; i++) {
    inc[i] = new dd_edge(mtmxd);
    mtmxd->createEdgeForVar(i, false, temp, inc[i][0]);
  }

  // Create edge for each function xi-1
  for (int i=0; i<maxBound; i++) temp[i] = i-1;
  for (int i=1; i<=nVars; i++) {
    dec[i] = new dd_edge(mtmxd);
    mtmxd->createEdgeForVar(i, false, temp, dec[i][0]);
  }

  mxd->createEdge(false, nsf);

  for (int e=0; e<nEvents; e++) {
    const char* ev = events[e];
#ifdef VERBOSE
    fprintf(stderr, "Event %5d", e);
#endif

    dd_edge nsf_ev(mxd);
    dd_edge term(mxd);

    //
    // build mask for this event
    //
    for (int i=1; i<=nVars; i++) {
      if ('.' == ev[i]) {
        minterm[i] = DONT_CARE;
        mtprime[i] = DONT_CHANGE;
      } else {
        minterm[i] = DONT_CARE;
        mtprime[i] = DONT_CARE;
      }
    }
    mxd->createEdge(&minterm, &mtprime, 1, nsf_ev);
#ifdef DEBUG_EVENTS
    printf("Initial nsf for event %d\n", e);
    nsf_ev.show(stdout, 2);
#endif
#ifdef VERBOSE
    fprintf(stderr, " : ");
#endif
    
    //
    // 'and' with the "do care" levels
    //
    for (int i=1; i<=nVars; i++) {
      dd_edge docare(mtmxd);
#ifdef VERBOSE
      fputc(ev[i], stderr);
#endif
      if ('.' == ev[i]) continue;
      switch (ev[i]) {
        case '+':   apply(EQUAL, varP[i][0], inc[i][0], docare);
                    break;

        case '-':   apply(EQUAL, varP[i][0], dec[i][0], docare);
                    break;

        default:    throw 1;
      } // switch
      apply(COPY, docare, term);
#ifdef DEBUG_EVENTS
      printf("Term for event %d, level %d\n", e, i);
      term.show(stdout, 2);
#endif
      nsf_ev *= term;
    } // for i

    //
    //  union with overall
    //
#ifdef DEBUG_EVENTS
    printf("Complete nsf for event %d:\n", e);
    nsf_ev.show(stdout, 2);
#endif
#ifdef VERBOSE
    fputc(' ', stderr);
#endif
    nsf += nsf_ev;

#ifdef VERBOSE
    fputc('\n', stderr);
#endif
  } // for e

  // cleanup
  delete[] mtprime;
  delete[] minterm;
  delete[] temp;
  for (int i=1; i<=nVars; i++) {
    delete varP[i];
    delete inc[i];
    delete dec[i];
  }
  delete[] varP;
  delete[] inc;
  delete[] dec;
  destroyForest(mtmxd);

#ifdef DEBUG_EVENTS
  printf("Complete NSF:\n");
  nsf.show(stdout, 2);
#endif
}

