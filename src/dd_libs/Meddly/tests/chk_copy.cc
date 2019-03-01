
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
    Tests the cross-product operator.
*/


#include <cstdlib>
#include <time.h>

#include "meddly.h"

#define CHECK_ALL_MTMDDS
#define CHECK_ALL_MTMXDS
#define CHECK_ALL_INTMDDS
#define CHECK_ALL_REALMDDS
#define CHECK_ALL_REALMXDS

const int TERMS = 20;

using namespace MEDDLY;

long seed = -1;

double Random()
{
  const long MODULUS = 2147483647L;
  const long MULTIPLIER = 48271L;
  const long Q = MODULUS / MULTIPLIER;
  const long R = MODULUS % MULTIPLIER;

  long t = MULTIPLIER * (seed % Q) - R * (seed / Q);
  if (t > 0) {
    seed = t;
  } else {
    seed = t + MODULUS;
  }
  return ((double) seed / MODULUS);
}

int Equilikely(int a, int b)
{
  return (a + (int) ((b - a + 1) * Random()));
}

void randomizeMinterm(bool primed, int max, int* mt, int N)
{
  int min = primed ? -2 : -1;
  for (int i=1; i<N; i++) {
    mt[i] = Equilikely(min, max);
  }
#ifdef DEBUG_RANDSET
  printf("Random minterm: [%d", minterm[1]);
  for (int i=2; i<N; i++) printf(", %d", minterm[i]);
  printf("]\n");
#endif
}

void adjustMinterms(int* mtu, int* mtp, int N)
{
  for (int i=1; i<N; i++) {
    if (mtp[i] == -2) mtu[i] = -1;
  }
}

void buildRandomFunc(long s, int terms, dd_edge &out)
{
  seed = s;
  forest* f = out.getForest();
  int Vars = f->getDomain()->getNumVariables();

  int* minterm = new int[Vars+1];
  int* minprime = 0;
  if (f->isForRelations()) minprime = new int[Vars+1];


  out.clear();
  for (int i=0; i<terms; i++) {
    randomizeMinterm(false, 4, minterm, Vars+1);
    if (minprime) {
      randomizeMinterm(true, 4, minprime, Vars+1);
      adjustMinterms(minterm, minprime, Vars+1);
    }
    int i_value = Equilikely(1, 5);
    float f_value = i_value;

    dd_edge temp(out);

    switch (f->getRangeType()) {
      case forest::BOOLEAN:
        if (minprime) f->createEdge(&minterm, &minprime, 1, temp);
        else          f->createEdge(&minterm, 1, temp);
        break;

      case forest::INTEGER:
        if (minprime) f->createEdge(&minterm, &minprime, &i_value, 1, temp);
        else          f->createEdge(&minterm, &i_value, 1, temp);
        break;
        
      case forest::REAL:
        if (minprime) f->createEdge(&minterm, &minprime, &f_value, 1, temp);
        else          f->createEdge(&minterm, &f_value, 1, temp);
        break;
    }

    out += temp;

  } // for i

  // cleanup
  delete[] minprime;
  delete[] minterm;
}

void writeType(const forest* f)
{
  switch (f->getRangeType()) {
    case forest::BOOLEAN:
      printf("bool ");
      break;

    case forest::INTEGER:
      printf("int. ");
      break;

    case forest::REAL:
      printf("real ");
      break;

    default:
      printf("unk. ");
      break;
  }

  switch(f->getReductionRule()) {

    case forest::policies::FULLY_REDUCED:
      printf("FR ");
      break;

    case forest::policies::QUASI_REDUCED:
      printf("QR ");
      break;

    case forest::policies::IDENTITY_REDUCED:
      printf("IR ");
      break;

    default:
      printf("?R ");
      break;
  }
  
  switch (f->getEdgeLabeling()) {
    case forest::MULTI_TERMINAL:
      printf(" mt");
      break;

    case forest::EVPLUS:
      printf("ev+");
      break;

    case forest::INDEX_SET:
      printf("ind");
      break;

    case forest::EVTIMES:
      printf("ev*");
      break;

    default:
      printf(" ??");
      break;
  }

  if (f->isForRelations())  printf("mxd");
  else                      printf("mdd");
}

void testCopy(forest* srcF, forest* destF)
{
  printf("\t");
  writeType(srcF);
  printf(" -> ");
  writeType(destF);
  printf("      ");
  fflush(stdout);

  dd_edge srcE(srcF);
  dd_edge destE(destF);

  try {
    for (int t=1; t<=TERMS; t++) {

      long save_seed = seed;
      buildRandomFunc(save_seed, t, srcE);
      buildRandomFunc(save_seed, t, destE);

      if (srcF->getRangeType() == forest::BOOLEAN) {
        if (destF->getRangeType() == forest::INTEGER) {
          // convert destE to boolean
          dd_edge zero(destF);
          destF->createEdge(int(0), zero);
          apply(NOT_EQUAL, destE, zero, destE);
        }
        if (destF->getRangeType() == forest::REAL) {
          // convert destE to boolean
          dd_edge zero(destF);
          destF->createEdge(float(0), zero);
          apply(NOT_EQUAL, destE, zero, destE);
        }
      }

      dd_edge copyE(destF);
      apply(COPY, srcE, copyE);

      if (copyE != destE) {
        FILE_output meddlyout(stdout);

        printf("failed!\n\n");
  
        printf("Source (first forest):\n");
        srcE.show(meddlyout, 3);
  
        printf("Destination (should get):\n");
        destE.show(meddlyout, 3);

        printf("Copy (built from source):\n");
        copyE.show(meddlyout, 3);
        exit(1);
      }

    } // for t

    printf("OK\n");
  }
  catch (MEDDLY::error e) {
    printf("%s\n", e.getName());
  }

}



int processArgs(int argc, const char** argv)
{
  if (argc>2) {
    /* Strip leading directory, if any: */
    const char* name = argv[0];
    for (const char* ptr=name; *ptr; ptr++) {
    if ('/' == *ptr) name = ptr+1;
  }
    printf("Usage: %s <seed>\n", name);
    return 0;
  }
  if (argc>1) {
    seed = atol(argv[1]);
  }
  if (seed < 1) {
    seed = time(0);
  }
  
  printf("Using rng seed %ld\n", seed);
  return 1;
}

void addMDDforests(domain* D, forest** list, int &i, 
  forest::edge_labeling ev, forest::range_type type)
{
  forest::policies fr(false);
  fr.setFullyReduced();
  forest::policies qr(false);
  qr.setQuasiReduced();

  list[i++] = D->createForest(0, type, ev, fr);
  list[i++] = D->createForest(0, type, ev, qr);
}

void addMXDforests(domain* D, forest** list, int &i, 
  forest::edge_labeling ev, forest::range_type type)
{
  forest::policies ir(true);
  ir.setIdentityReduced();
  forest::policies fr(true);
  fr.setFullyReduced();
  forest::policies qr(true);
  qr.setQuasiReduced();

  list[i++] = D->createForest(1, type, ev, ir);
  list[i++] = D->createForest(1, type, ev, fr);
  list[i++] = D->createForest(1, type, ev, qr);
}

int makeMTMDDforests(domain* D, forest** list)
{
  int i = 0;
  addMDDforests(D, list, i, forest::MULTI_TERMINAL, forest::BOOLEAN);
  addMDDforests(D, list, i, forest::MULTI_TERMINAL, forest::INTEGER);
  addMDDforests(D, list, i, forest::MULTI_TERMINAL, forest::REAL);
  return i;
}

int makeMTMXDforests(domain* D, forest** list)
{
  int i = 0;
  addMXDforests(D, list, i, forest::MULTI_TERMINAL, forest::BOOLEAN);
  addMXDforests(D, list, i, forest::MULTI_TERMINAL, forest::INTEGER);
  addMXDforests(D, list, i, forest::MULTI_TERMINAL, forest::REAL);
  return i;
}

int makeIntegerMDDforests(domain* D, forest** list)
{
  int i = 0;
  addMDDforests(D, list, i, forest::MULTI_TERMINAL, forest::INTEGER);
  addMDDforests(D, list, i, forest::EVPLUS, forest::INTEGER);
  return i;
}

void addRealMDDforests(int& i, domain* D, forest** list)
{
  addMDDforests(D, list, i, forest::MULTI_TERMINAL, forest::REAL);
  // TBD - these are not supported yet
//  addMDDforests(D, list, i, forest::EVPLUS, forest::REAL);
//  addMDDforests(D, list, i, forest::EVTIMES, forest::REAL);
  // TBD
}

void addRealMXDforests(int& i, domain* D, forest** list)
{
  addMXDforests(D, list, i, forest::EVTIMES, forest::REAL);
  addMXDforests(D, list, i, forest::MULTI_TERMINAL, forest::REAL);
  // TBD - these are not supported yet
//  addMXDforests(D, list, i, forest::EVPLUS, forest::REAL);
  // TBD
}

int makeRealMDDforests(domain* D, forest** list)
{
  int i = 0;
  addRealMDDforests(i, D, list);
  return i;
}

int makeRealMXDforests(domain* D, forest** list)
{
  int i = 0;
  addRealMXDforests(i, D, list);
  return i;
}

void clearForests(forest** list, int N)
{
  for (int i=0; i<N; i++) {
    destroyForest(list[i]);
    list[i] = 0;
  }
}

int main(int argc, const char** argv)
{
  if (!processArgs(argc, argv)) return 1;

  initialize();

  int vars[] = {5, 5, 5, 5, 5, 5};
  domain* myd = createDomainBottomUp(vars, 6);
  assert(myd);

  //
  // For later - arrays of forests (!)
  //

  forest* srcs[9];
  forest* dests[9];
  int slen = 9;
  int dlen = 9;
  for (int i=0; i<9; i++) {
    srcs[i] = 0;
    dests[i] = 0;
  }

  //
  // MTMDD tests, build all possible types of forests
  //

#ifdef CHECK_ALL_MTMDDS
  printf("Checking all possible copies between MTMDD forests\n");
  slen = makeMTMDDforests(myd, srcs);
  dlen = makeMTMDDforests(myd, dests);

  for (int i=0; i<slen; i++)
    for (int j=0; j<dlen; j++)
      testCopy(srcs[i], dests[j]);

  clearForests(srcs, slen);
  clearForests(dests, dlen);
  printf("\n");
#endif

  //
  // MTMXD tests, build all possible types
  //

#ifdef CHECK_ALL_MTMXDS
  printf("Checking all possible copies between MTMXD forests\n");
  slen = makeMTMXDforests(myd, srcs);
  dlen = makeMTMXDforests(myd, dests);

  for (int i=0; i<slen; i++)
    for (int j=0; j<dlen; j++)
      testCopy(srcs[i], dests[j]);

  clearForests(srcs, slen);
  clearForests(dests, dlen);
  printf("\n");
#endif

  //
  // MT and EV part 1
  // (integer to integer or real, mt or ev)
  //

#ifdef CHECK_ALL_INTMDDS
  printf("Checking all possible copies from integer to integer/real MDD forests\n");
  slen = makeIntegerMDDforests(myd, srcs);
  dlen = makeIntegerMDDforests(myd, dests);
  addRealMDDforests(dlen, myd, dests); 

  for (int i=0; i<slen; i++)
    for (int j=0; j<dlen; j++)
      testCopy(srcs[i], dests[j]);

  clearForests(srcs, slen);
  clearForests(dests, dlen);
  printf("\n");
#endif

  //
  // MT and EV part 2
  // (real only)
  //

#ifdef CHECK_ALL_REALMDDS
  printf("Checking all possible copies for real MDD forests\n");
  slen = makeRealMDDforests(myd, srcs);
  dlen = makeRealMDDforests(myd, dests);

  for (int i=0; i<slen; i++)
    for (int j=0; j<dlen; j++)
      testCopy(srcs[i], dests[j]);

  clearForests(srcs, slen);
  clearForests(dests, dlen);
  printf("\n");
#endif

  //
  // MT and EV part 3
  // (real only)
  //

#ifdef CHECK_ALL_REALMXDS
  printf("Checking all possible copies for real MXD forests\n");
  slen = makeRealMXDforests(myd, srcs);
  dlen = makeRealMXDforests(myd, dests);

  for (int i=0; i<slen; i++)
    for (int j=0; j<dlen; j++)
      testCopy(srcs[i], dests[j]);

  clearForests(srcs, slen);
  clearForests(dests, dlen);
  printf("\n");
#endif

  
  cleanup();
  return 0;
}


