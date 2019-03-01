
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
    Tests for floating-point errors when using EV*MxDs.
*/


#include <cstdlib>
#include <time.h>

#include "meddly.h"

using namespace MEDDLY;

#define CHECK_COPY
#define CHECK_COUNTS

#define ENABLE_IDENTITY_REDUCED
// #define ENABLE_FULLY_REDUCED
// #define ENABLE_QUASI_REDUCED

// #define DEBUG_RANDSET

const int varSize = 4;
const int TERMS = 100;
int terms = 0;
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

void printRandomSet(int* mt, int N)
{
  printf("Random minterm: [%d", mt[1]);
  for (int i=2; i<N; i++) printf(", %d", mt[i]);
  printf("]\n");
}

void randomizeMinterm(bool primed, int max, int* mt, int N)
{
  int min = primed ? -2 : -1;
  for (int i=1; i<N; i++) {
    mt[i] = Equilikely(min, max);
  }
#ifdef DEBUG_RANDSET
  printf("\n");
  printRandomSet(mt, N);
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

  if (f->getRangeType() != forest::BOOLEAN &&
      f->getRangeType() != forest::REAL) {
    printf("Invalid forest: does not store booleans or reals!\n\n");
    exit(1);
  }
  if (!f->isForRelations()) {
    printf("Invalid forest: does not store relations!\n\n");
    exit(1);
  }

  int* minterm = new int[Vars+1];
  int* minprime = new int[Vars+1];

  out.clear();
  for (int i=0; i<terms; i++) {

    randomizeMinterm(false, varSize-1, minterm, Vars+1);
    randomizeMinterm(true, varSize-1, minprime, Vars+1);
    adjustMinterms(minterm, minprime, Vars+1);
    float f_value = float(Equilikely(1, varSize));
    dd_edge temp(out);
    if (f->getRangeType() == forest::BOOLEAN) {
      f->createEdge(&minterm, &minprime, 1, temp);
    } else {
      f->createEdge(&minterm, &minprime, &f_value, 1, temp);
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


void testEVTimesMXD(forest* srcF, forest* destF)
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
    for (int t=1; t<=terms; t++) {

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

        dd_edge diff(destF);
        apply(MINUS, destE, copyE, diff);
        printf("Difference:\n");
        diff.show(meddlyout, 3);

        exit(1);
      }

    } // for t

    printf("OK\n");
  }
  catch (MEDDLY::error e) {
    printf("%s\n", e.getName());
  }

}



void testEV(forest* mxd, forest* mtmxd, forest* evmxd)
{
  dd_edge MXD(mxd);
  dd_edge MTMXD(mtmxd);
  dd_edge EVMXD(evmxd);

  try {

    for (int t=1; t<=terms; t++) {

      long save_seed = seed;
      buildRandomFunc(save_seed, t, MXD);
      buildRandomFunc(save_seed, t, MTMXD);
      buildRandomFunc(save_seed, t, EVMXD);

      if (MTMXD.getNodeCount() < EVMXD.getNodeCount()
        || MTMXD.getEdgeCount(false) < EVMXD.getEdgeCount(false)) {
        printf("failed!\n\n");

        printf("\nCardinality: MxD = %f, MTMxD = %f, EV*MxD = %f", 
            MXD.getCardinality(), 
            MTMXD.getCardinality(), 
            EVMXD.getCardinality());
        printf("\nNode Count: MxD = %d, MTMxD = %d, EV*MxD = %d", 
            MXD.getNodeCount(),
            MTMXD.getNodeCount(),
            EVMXD.getNodeCount());
        printf("\nEdge Count: MxD = %d, MTMxD = %d, EV*MxD = %d", 
            MXD.getEdgeCount(false),
            MTMXD.getEdgeCount(false),
            EVMXD.getEdgeCount(false));

        exit(1);
      }


    } // for t

    printf("\nOK\n");
  }
  catch (MEDDLY::error e) {
    printf("%s\n", e.getName());
  }

}



int processArgs(int argc, const char** argv)
{
  if (argc>3) {
    /* Strip leading directory, if any: */
    const char* name = argv[0];
    for (const char* ptr=name; *ptr; ptr++) {
      if ('/' == *ptr) name = ptr+1;
    }
    printf("Usage: %s <seed> <#minterms>\n", name);
    return 0;
  }
  if (argc>2) {
    terms = atol(argv[2]);
  } else {
    terms = TERMS;
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

void addMXDforests(domain* D, forest** list, int &i, 
  forest::edge_labeling ev, forest::range_type type)
{
#ifdef ENABLE_IDENTITY_REDUCED
  forest::policies ir(true);
  ir.setIdentityReduced();
  list[i++] = D->createForest(1, type, ev, ir);
#endif
#ifdef ENABLE_FULLY_REDUCED
  forest::policies fr(true);
  fr.setFullyReduced();
  list[i++] = D->createForest(1, type, ev, fr);
#endif
#ifdef ENABLE_QUASI_REDUCED
  forest::policies qr(true);
  qr.setQuasiReduced();
  list[i++] = D->createForest(1, type, ev, qr);
#endif
}

void addRealMXDforests(int& i, domain* D, forest** list)
{
  addMXDforests(D, list, i, forest::EVTIMES, forest::REAL);
  // addMXDforests(D, list, i, forest::MULTI_TERMINAL, forest::REAL);
  // TBD - these are not supported yet
//  addMXDforests(D, list, i, forest::EVPLUS, forest::REAL);
  // TBD
}

int makeRealMXDforests(domain* D, forest** list)
{
  int i = 0;
  addRealMXDforests(i, D, list);
  return i;
}

void addIRMXDforest(domain* D, forest** list, int &i, 
  forest::edge_labeling ev, forest::range_type type)
{
  forest::policies ir(true);
  ir.setIdentityReduced();
  list[i++] = D->createForest(1, type, ev, ir);
}

int makeIRMXDforests(domain* D, forest** list)
{
  int i = 0;
  addIRMXDforest(D, list, i, forest::MULTI_TERMINAL, forest::BOOLEAN);
  addIRMXDforest(D, list, i, forest::MULTI_TERMINAL, forest::REAL);
  addIRMXDforest(D, list, i, forest::EVTIMES, forest::REAL);
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

  int vars[] = {varSize, varSize, varSize, varSize, varSize, varSize};
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
  // MT and EV part 1
  // (real only)
  //

#ifdef CHECK_COPY
  printf("Checking all possible copies for Real MXD forests\n");
  slen = makeRealMXDforests(myd, srcs);
  dlen = makeRealMXDforests(myd, dests);

  for (int i=0; i<slen; i++)
    for (int j=0; j<dlen; j++)
      testEVTimesMXD(srcs[i], dests[j]);

  clearForests(srcs, slen);
  clearForests(dests, dlen);
  printf("\n");
#endif

  //
  // Boolean and EV part 2
  // (real only)
  //

#ifdef CHECK_COUNTS
  printf("Checking for errors when building Real MXD forests\n");
  slen = makeIRMXDforests(myd, srcs);
  assert(3 == slen);

  testEV(srcs[0], srcs[1], srcs[2]);

  clearForests(srcs, slen);
  printf("\n");
#endif

  
  cleanup();
  return 0;
}


