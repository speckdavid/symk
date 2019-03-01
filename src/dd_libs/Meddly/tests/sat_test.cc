
// $Id: check_xA.cc 260 2011-07-20 19:05:35Z asminer $

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
    Tests basic logic operations on binary MDDs, using 3-SAT.

    Note: most of thest tests were automatically generated :^)
*/

#include <cstdlib>
#include "meddly.h"

using namespace MEDDLY;

void clauses2MDD(forest* f, const int** C, dd_edge &M, int verb)
{
  // build array of constraints, by level
  const domain* d = f->getDomain();
  dd_edge** ce = new dd_edge* [d->getNumVariables()+1];
  for (int i=0; i<=d->getNumVariables(); i++) ce[i] = 0;

  // Encode clauses, by "top" level
  if (verb>1) printf("Encoding clauses ");
  if (verb>2) printf("\n");
  for (; C[0]; C++) {
    if (verb>2) {
      printf("\tClause: ");
    }
    dd_edge cl(f);
    f->createEdge(false, cl);
    bool vals[2];
    for (const int* term = C[0]; term[0]; term++) {
      int vh;
      if (term[0]>0) {
        vals[0] = 0;
        vals[1] = 1;
        vh = term[0];
      } else {
        vals[0] = 1;
        vals[1] = 0;
        vh = -term[0];
      }
      dd_edge var(f);
      f->createEdgeForVar(vh, false, vals, var);
      if (verb>2) {
        printf("%d ", term[0]);
      }
      cl += var;
    } // for term
    // donw with clause; add to appropriate level
    int k = cl.getLevel();
    if (0==ce[k]) {
      ce[k] = new dd_edge(cl);
    } else {
      (*ce[k]) *= cl;
    }
    if (verb>2) printf("\n");
    else if (verb>1) printf(".");
    fflush(stdout);
  } // for C

  if (verb>1) printf("\nMerging clauses by level\n\t");
  f->createEdge(true, M);
  for (int i=1; i<=d->getNumVariables(); i++) {
    if (ce[i]) {
      M *= (*ce[i]);
      delete ce[i];
    }
    if (verb) {
      if (i>1) printf(", ");
      printf("%d", i);
      fflush(stdout);
    }
  }
  if (verb) printf("\n");
  delete[] ce;
}

forest* makeBinaryForest(int N)
{
  int* b = new int[N];
  for (int i=0; i<N; i++) b[i] = 2;
  domain* d = createDomainBottomUp(b, N);
  delete[] b;
  return d->createForest(false, forest::BOOLEAN, forest::MULTI_TERMINAL);
}

void check_solution(const char* name, forest* f, dd_edge& M, const int* sol)
{
  long ns;
  apply(CARDINALITY, M, ns);
  printf("%s has %ld solutions\n", name, ns);
  if (ns!=1) exit(2);  

  bool ok;
  f->evaluate(M, sol, ok);
  if (!ok) {
    printf("Solution doesn't match\n");
    exit(3);
  }
}

void tiny_test()
{
  const int c_01[] = {1, 2, 3, 0};
  const int c_02[] = {1, 2, -3, 0};
  const int c_03[] = {1, -2, 3, 0};
  const int c_04[] = {1, -2, -3, 0};
  const int c_05[] = {-1, 2, 3, 0};
  const int c_06[] = {-1, 2, -3, 0};
  const int c_07[] = {-1, -2, 3, 0};
  const int* clauses[] = { c_01, c_02, c_03, c_04, c_05, c_06, c_07, 0 };
  const int sol[] = { -1, 1, 1, 1 };

  forest* f = makeBinaryForest(3);
  dd_edge M(f);

  clauses2MDD(f, clauses, M, 2);
  check_solution("Tiny test", f, M, sol);
  destroyForest(f);
}

void test_10()
{
  const int sol[] = { -1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0}; 

  const int c_01[] = {-10  , -7  , 2 , 0};
  const int c_02[] = {10  , -1  , -9 , 0};
  const int c_03[] = {5  , 3  , -6 , 0};
  const int c_04[] = {-5  , 6  , 3 , 0}; 
  const int c_05[] = {6  , -1  , 4 , 0}; 
  const int c_06[] = {-1  , 5  , -2 , 0}; 
  const int c_07[] = {-9  , 6  , -3 , 0}; 
  const int c_08[] = {-5  , -4  , -10 , 0}; 
  const int c_09[] = {7  , 2  , 4 , 0}; 
  const int c_10[] = {6  , 2  , 8 , 0}; 
  const int c_11[] = {-4  , -1  , 2 , 0}; 
  const int c_12[] = {-7  , 9  , 4 , 0}; 
  const int c_13[] = {10  , 5  , 9 , 0}; 
  const int c_14[] = {-3  , 1  , -4 , 0}; 
  const int c_15[] = {-2  , -4  , 1 , 0}; 
  const int c_16[] = {10  , 4  , -9 , 0}; 
  const int c_17[] = {-2  , 9  , 6 , 0}; 
  const int c_18[] = {-2  , 3  , -4 , 0}; 
  const int c_19[] = {-8  , 4  , 2 , 0}; 
  const int c_20[] = {-4  , -8  , -3 , 0}; 
  const int c_21[] = {-9  , 7  , 10 , 0}; 
  const int c_22[] = {-10  , 4  , 7 , 0}; 
  const int c_23[] = {8  , -3  , 9 , 0}; 
  const int c_24[] = {-2  , 8  , -3 , 0}; 
  const int c_25[] = {6  , 4  , 7 , 0}; 
  const int c_26[] = {2  , -6  , -10 , 0}; 
  const int c_27[] = {-9  , -5  , 7 , 0}; 
  const int c_28[] = {5  , -7  , 6 , 0}; 
  const int c_29[] = {-8  , -9  , -10 , 0}; 
  const int c_30[] = {-4  , 6  , 10 , 0}; 
  const int c_31[] = {3  , 1  , -6 , 0}; 
  const int c_32[] = {-8  , 5  , 7 , 0}; 
  const int c_33[] = {8  , 1  , 4 , 0}; 
  const int c_34[] = {-7  , -10  , 1 , 0}; 
  const int c_35[] = {9  , -8  , -2 , 0}; 
  const int c_36[] = {-6  , -4  , -5 , 0}; 
  const int c_37[] = {-1  , -7  , -6 , 0}; 
  const int c_38[] = {-3  , -2  , 9 , 0}; 
  const int c_39[] = {-2  , -8  , 6 , 0}; 
  const int c_40[] = {-10  , 3  , -7 , 0}; 
  const int c_41[] = {8  , -9  , 5 , 0}; 
  const int c_42[] = {-4  , 2  , 3 , 0};

  const int* clauses[] = {  c_01, c_02, c_03, c_04, c_05, c_06, c_07, 
                            c_08, c_09, c_10, c_11, c_12, c_13, c_14,
                            c_15, c_16, c_17, c_18, c_19, c_20, c_21,
                            c_22, c_23, c_24, c_25, c_26, c_27, c_28,
                            c_29, c_30, c_31, c_32, c_33, c_34, c_35,
                            c_36, c_37, c_38, c_39, c_40, c_41, c_42,
                            0 };

  forest* f = makeBinaryForest(10);
  dd_edge M(f);

  clauses2MDD(f, clauses, M, 2);
  check_solution("Test 10", f, M, sol);
  destroyForest(f);
}

void test_20()
{
  const int sol[] = { -1, 1, 0, 0, 0, 1, 0, 1, 0, 1, 1, 1, 1, 0, 0, 0, 
                      1, 0, 0, 1, 1 };

  const int c_1[] = {-10 , 20 , -13 , 0};
  const int c_2[] = {2 , 15 , -4 , 0};
  const int c_3[] = {-17 , 13 , 9 , 0};
  const int c_4[] = {12 , 15 , -13 , 0};
  const int c_5[] = {-14 , -3 , -17 , 0};
  const int c_6[] = {-13 , 16 , 9 , 0};
  const int c_7[] = {-20 , 10 , 8 , 0};
  const int c_8[] = {17 , 5 , -16 , 0};
  const int c_9[] = {19 , 4 , 14 , 0};
  const int c_10[] = {19 , -11 , -3 , 0};
  const int c_11[] = {9 , 4 , 18 , 0};
  const int c_12[] = {18 , 12 , 11 , 0};
  const int c_13[] = {4 , 7 , 14 , 0};
  const int c_14[] = {-12 , -2 , -10 , 0};
  const int c_15[] = {-2 , 6 , -4 , 0};
  const int c_16[] = {-9 , 2 , 7 , 0};
  const int c_17[] = {-2 , -5 , 15 , 0};
  const int c_18[] = {13 , 19 , 4 , 0};
  const int c_19[] = {-9 , 8 , -2 , 0};
  const int c_20[] = {18 , 3 , 10 , 0};
  const int c_21[] = {-6 , -19 , -14 , 0};
  const int c_22[] = {-7 , -11 , 12 , 0};
  const int c_23[] = {-13 , -4 , -14 , 0};
  const int c_24[] = {-6 , 19 , 14 , 0};
  const int c_25[] = {-5 , -15 , 3 , 0};
  const int c_26[] = {16 , 20 , 2 , 0};
  const int c_27[] = {19 , -5 , 17 , 0};
  const int c_28[] = {9 , -3 , -18 , 0};
  const int c_29[] = {7 , 8 , 14 , 0};
  const int c_30[] = {-14 , -6 , -17 , 0};
  const int c_31[] = {-14 , -20 , -5 , 0};
  const int c_32[] = {-4 , 9 , -8 , 0};
  const int c_33[] = {-12 , 10 , 1 , 0};
  const int c_34[] = {-14 , -2 , -19 , 0};
  const int c_35[] = {15 , -2 , 16 , 0};
  const int c_36[] = {-4 , -15 , -20 , 0};
  const int c_37[] = {-20 , 4 , 9 , 0};
  const int c_38[] = {5 , 14 , -9 , 0};
  const int c_39[] = {-4 , 16 , -2 , 0};
  const int c_40[] = {2 , -16 , 10 , 0};
  const int c_41[] = {11 , 2 , 8 , 0};
  const int c_42[] = {-7 , -13 , 19 , 0};
  const int c_43[] = {-18 , -13 , 11 , 0};
  const int c_44[] = {-18 , 10 , -17 , 0};
  const int c_45[] = {1 , -16 , 10 , 0};
  const int c_46[] = {5 , 17 , 10 , 0};
  const int c_47[] = {4 , 11 , 12 , 0};
  const int c_48[] = {-14 , 4 , -11 , 0};
  const int c_49[] = {-15 , 8 , -14 , 0};
  const int c_50[] = {6 , -2 , 16 , 0};
  const int c_51[] = {-18 , -7 , -10 , 0};
  const int c_52[] = {18 , -15 , -12 , 0};
  const int c_53[] = {13 , 10 , -9 , 0};
  const int c_54[] = {-6 , -1 , 13 , 0};
  const int c_55[] = {-2 , -1 , -16 , 0};
  const int c_56[] = {-17 , -11 , -7 , 0};
  const int c_57[] = {11 , 5 , -17 , 0};
  const int c_58[] = {4 , 17 , 9 , 0};
  const int c_59[] = {12 , -8 , 1 , 0};
  const int c_60[] = {5 , -1 , -9 , 0};
  const int c_61[] = {17 , 5 , 1 , 0};
  const int c_62[] = {-11 , -9 , 1 , 0};
  const int c_63[] = {17 , -4 , 2 , 0};
  const int c_64[] = {1 , -8 , -5 , 0};
  const int c_65[] = {3 , 20 , -12 , 0};
  const int c_66[] = {-13 , 1 , -3 , 0};
  const int c_67[] = {-16 , -6 , -12 , 0};
  const int c_68[] = {-5 , 2 , -8 , 0};
  const int c_69[] = {9 , 13 , -18 , 0};
  const int c_70[] = {-7 , -3 , 17 , 0};
  const int c_71[] = {4 , 16 , 14 , 0};
  const int c_72[] = {12 , -11 , -8 , 0};
  const int c_73[] = {-12 , 16 , 2 , 0};
  const int c_74[] = {-3 , 18 , -10 , 0};
  const int c_75[] = {11 , 12 , -3 , 0};
  const int c_76[] = {-13 , 15 , 18 , 0};
  const int c_77[] = {16 , -8 , -9 , 0};
  const int c_78[] = {-10 , 12 , 15 , 0};
  const int c_79[] = {16 , 14 , -1 , 0};
  const int c_80[] = {-1 , 8 , -3 , 0};
  const int c_81[] = {3 , -11 , -15 , 0};
  const int c_82[] = {-8 , -12 , 6 , 0};
  const int c_83[] = {-8 , 18 , 7 , 0};
  const int c_84[] = {-12 , -8 , 14 , 0};

  const int* clauses[] = {
      c_1, c_2, c_3, c_4, c_5, c_6, c_7, c_8, c_9, c_10, 
      c_11, c_12, c_13, c_14, c_15, c_16, c_17, c_18, c_19, c_20, 
      c_21, c_22, c_23, c_24, c_25, c_26, c_27, c_28, c_29, c_30, 
      c_31, c_32, c_33, c_34, c_35, c_36, c_37, c_38, c_39, c_40, 
      c_41, c_42, c_43, c_44, c_45, c_46, c_47, c_48, c_49, c_50, 
      c_51, c_52, c_53, c_54, c_55, c_56, c_57, c_58, c_59, c_60, 
      c_61, c_62, c_63, c_64, c_65, c_66, c_67, c_68, c_69, c_70, 
      c_71, c_72, c_73, c_74, c_75, c_76, c_77, c_78, c_79, c_80, 
      c_81, c_82, c_83, c_84, 
      0 };

  forest* f = makeBinaryForest(20);
  dd_edge M(f);

  clauses2MDD(f, clauses, M, 2);
  check_solution("Test 20", f, M, sol);
  destroyForest(f);
}

void test_30()
{
  const int sol[] = { -1, 1, 1, 0, 0, 0, 1, 1, 0, 1, 1, 1, 1, 0, 0, 0, 
                          0, 0, 0, 1, 0, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1 };

  const int c_1[] = {21 , 27 , 23 , 0};
  const int c_2[] = {-15 , -24 , -1 , 0};
  const int c_3[] = {7 , 5 , -11 , 0};
  const int c_4[] = {29 , -5 , 19 , 0};
  const int c_5[] = {2 , -17 , 7 , 0};
  const int c_6[] = {19 , 3 , -7 , 0};
  const int c_7[] = {-11 , -25 , -9 , 0};
  const int c_8[] = {-27 , 6 , 24 , 0};
  const int c_9[] = {30 , 1 , 23 , 0};
  const int c_10[] = {7 , 30 , 25 , 0};
  const int c_11[] = {17 , 9 , 3 , 0};
  const int c_12[] = {-15 , 23 , 25 , 0};
  const int c_13[] = {25 , -26 , 6 , 0};
  const int c_14[] = {-23 , 1 , -9 , 0};
  const int c_15[] = {-26 , 18 , -13 , 0};
  const int c_16[] = {-13 , 15 , -23 , 0};
  const int c_17[] = {17 , -10 , 19 , 0};
  const int c_18[] = {25 , -11 , 26 , 0};
  const int c_19[] = {23 , 20 , 17 , 0};
  const int c_20[] = {-20 , 11 , 24 , 0};
  const int c_21[] = {-8 , 13 , -10 , 0};
  const int c_22[] = {-2 , -29 , 22 , 0};
  const int c_23[] = {-13 , 21 , 15 , 0};
  const int c_24[] = {-15 , 17 , -2 , 0};
  const int c_25[] = {4 , -1 , -28 , 0};
  const int c_26[] = {11 , -14 , 4 , 0};
  const int c_27[] = {4 , 19 , 30 , 0};
  const int c_28[] = {-10 , -8 , 22 , 0};
  const int c_29[] = {18 , -22 , 26 , 0};
  const int c_30[] = {11 , -14 , -28 , 0};
  const int c_31[] = {26 , 5 , 17 , 0};
  const int c_32[] = {13 , 1 , 28 , 0};
  const int c_33[] = {24 , -21 , 13 , 0};
  const int c_34[] = {4 , -18 , 22 , 0};
  const int c_35[] = {-29 , -4 , 18 , 0};
  const int c_36[] = {-18 , 20 , 8 , 0};
  const int c_37[] = {-20 , 14 , -9 , 0};
  const int c_38[] = {3 , 17 , -15 , 0};
  const int c_39[] = {25 , -23 , -15 , 0};
  const int c_40[] = {-24 , -5 , -12 , 0};
  const int c_41[] = {-4 , 16 , 15 , 0};
  const int c_42[] = {-27 , -17 , 5 , 0};
  const int c_43[] = {9 , 1 , 7 , 0};
  const int c_44[] = {15 , -29 , 10 , 0};
  const int c_45[] = {3 , -1 , 21 , 0};
  const int c_46[] = {-18 , 26 , -3 , 0};
  const int c_47[] = {-4 , -23 , 15 , 0};
  const int c_48[] = {25 , 7 , 20 , 0};
  const int c_49[] = {-8 , 5 , -6 , 0};
  const int c_50[] = {13 , 4 , -18 , 0};
  const int c_51[] = {-1 , -20 , 10 , 0};
  const int c_52[] = {-29 , -10 , 2 , 0};
  const int c_53[] = {2 , 28 , -15 , 0};
  const int c_54[] = {-18 , 30 , 25 , 0};
  const int c_55[] = {-11 , -30 , -8 , 0};
  const int c_56[] = {12 , 8 , 16 , 0};
  const int c_57[] = {9 , -10 , -6 , 0};
  const int c_58[] = {-12 , -11 , -28 , 0};
  const int c_59[] = {-27 , -5 , -22 , 0};
  const int c_60[] = {13 , 19 , -1 , 0};
  const int c_61[] = {-18 , -7 , 17 , 0};
  const int c_62[] = {-1 , 12 , 2 , 0};
  const int c_63[] = {8 , -27 , 10 , 0};
  const int c_64[] = {4 , -3 , -19 , 0};
  const int c_65[] = {-29 , -22 , 2 , 0};
  const int c_66[] = {1 , 5 , -10 , 0};
  const int c_67[] = {-14 , -23 , -28 , 0};
  const int c_68[] = {4 , 5 , -16 , 0};
  const int c_69[] = {-18 , 4 , 11 , 0};
  const int c_70[] = {24 , -29 , 13 , 0};
  const int c_71[] = {-17 , -12 , -5 , 0};
  const int c_72[] = {23 , 15 , -19 , 0};
  const int c_73[] = {-13 , -15 , 6 , 0};
  const int c_74[] = {24 , -30 , 2 , 0};
  const int c_75[] = {12 , 27 , 8 , 0};
  const int c_76[] = {-5 , 10 , -3 , 0};
  const int c_77[] = {19 , 8 , -17 , 0};
  const int c_78[] = {-17 , -20 , 15 , 0};
  const int c_79[] = {11 , -24 , -29 , 0};
  const int c_80[] = {29 , -16 , -24 , 0};
  const int c_81[] = {-26 , 2 , 28 , 0};
  const int c_82[] = {-2 , -22 , 7 , 0};
  const int c_83[] = {17 , -22 , -15 , 0};
  const int c_84[] = {-6 , 27 , -7 , 0};
  const int c_85[] = {-15 , 4 , -12 , 0};
  const int c_86[] = {8 , -24 , -17 , 0};
  const int c_87[] = {27 , -10 , -25 , 0};
  const int c_88[] = {29 , -30 , 21 , 0};
  const int c_89[] = {20 , 8 , 1 , 0};
  const int c_90[] = {3 , -23 , 22 , 0};
  const int c_91[] = {10 , 1 , 27 , 0};
  const int c_92[] = {29 , 15 , 16 , 0};
  const int c_93[] = {-7 , 11 , 17 , 0};
  const int c_94[] = {5 , 22 , 21 , 0};
  const int c_95[] = {-30 , -8 , -23 , 0};
  const int c_96[] = {-11 , -25 , 18 , 0};
  const int c_97[] = {-29 , 23 , -17 , 0};
  const int c_98[] = {15 , -17 , -23 , 0};
  const int c_99[] = {7 , -1 , 27 , 0};
  const int c_100[] = {-1 , 13 , 26 , 0};
  const int c_101[] = {-28 , -22 , 4 , 0};
  const int c_102[] = {28 , 25 , -5 , 0};
  const int c_103[] = {18 , -22 , 30 , 0};
  const int c_104[] = {30 , -18 , 8 , 0};
  const int c_105[] = {-9 , 19 , -26 , 0};
  const int c_106[] = {-1 , -2 , -15 , 0};
  const int c_107[] = {22 , 6 , -2 , 0};
  const int c_108[] = {-27 , 24 , 10 , 0};
  const int c_109[] = {-6 , 22 , 2 , 0};
  const int c_110[] = {17 , 16 , 9 , 0};
  const int c_111[] = {-24 , -26 , -5 , 0};
  const int c_112[] = {24 , -23 , -15 , 0};
  const int c_113[] = {-19 , -3 , 21 , 0};
  const int c_114[] = {-22 , 10 , 2 , 0};
  const int c_115[] = {-28 , -8 , 5 , 0};
  const int c_116[] = {10 , 7 , -26 , 0};
  const int c_117[] = {-9 , 16 , 23 , 0};
  const int c_118[] = {-14 , -10 , -30 , 0};
  const int c_119[] = {-26 , -10 , -14 , 0};
  const int c_120[] = {-10 , 6 , -22 , 0};
  const int c_121[] = {-3 , -27 , -9 , 0};
  const int c_122[] = {-21 , 22 , -7 , 0};
  const int c_123[] = {8 , 22 , 6 , 0};
  const int c_124[] = {30 , -17 , -20 , 0};
  const int c_125[] = {-30 , 27 , -14 , 0};
  const int c_126[] = {15 , 10 , -8 , 0};

  const int* clauses[] = {
      c_1, c_2, c_3, c_4, c_5, c_6, c_7, c_8, c_9, c_10, 
      c_11, c_12, c_13, c_14, c_15, c_16, c_17, c_18, c_19, c_20, 
      c_21, c_22, c_23, c_24, c_25, c_26, c_27, c_28, c_29, c_30, 
      c_31, c_32, c_33, c_34, c_35, c_36, c_37, c_38, c_39, c_40, 
      c_41, c_42, c_43, c_44, c_45, c_46, c_47, c_48, c_49, c_50, 
      c_51, c_52, c_53, c_54, c_55, c_56, c_57, c_58, c_59, c_60, 
      c_61, c_62, c_63, c_64, c_65, c_66, c_67, c_68, c_69, c_70, 
      c_71, c_72, c_73, c_74, c_75, c_76, c_77, c_78, c_79, c_80, 
      c_81, c_82, c_83, c_84, c_85, c_86, c_87, c_88, c_89, c_90, 
      c_91, c_92, c_93, c_94, c_95, c_96, c_97, c_98, c_99, c_100, 
      c_101, c_102, c_103, c_104, c_105, c_106, c_107, c_108, c_109, c_110, 
      c_111, c_112, c_113, c_114, c_115, c_116, c_117, c_118, c_119, c_120, 
      c_121, c_122, c_123, c_124, c_125, c_126, 
      0 };

  forest* f = makeBinaryForest(30);
  dd_edge M(f);

  clauses2MDD(f, clauses, M, 2);
  check_solution("Test 30", f, M, sol);
  destroyForest(f);
}

void test_40()
{
  const int sol[] = { -1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 
                          1, 0, 0, 0, 0, 1, 0, 1, 1, 0, 1, 1, 0, 1, 0, 
                          1, 0, 1, 0, 1, 0, 1, 1, 1, 0 };

  const int c_1[] = {-32 , 36 , 27 , 0};
  const int c_2[] = {-20 , 6 , -33 , 0};
  const int c_3[] = {34 , 16 , -24 , 0};
  const int c_4[] = {31 , 30 , -36 , 0};
  const int c_5[] = {-21 , 12 , -6 , 0};
  const int c_6[] = {16 , -2 , 10 , 0};
  const int c_7[] = {-9 , 37 , -10 , 0};
  const int c_8[] = {-7 , -24 , -8 , 0};
  const int c_9[] = {4 , 5 , 25 , 0};
  const int c_10[] = {27 , 26 , 38 , 0};
  const int c_11[] = {-13 , -17 , 18 , 0};
  const int c_12[] = {4 , 17 , 5 , 0};
  const int c_13[] = {30 , 39 , -38 , 0};
  const int c_14[] = {-17 , -32 , 11 , 0};
  const int c_15[] = {-16 , 33 , -2 , 0};
  const int c_16[] = {14 , 1 , -18 , 0};
  const int c_17[] = {29 , -16 , 31 , 0};
  const int c_18[] = {-17 , -27 , 22 , 0};
  const int c_19[] = {-38 , 31 , -14 , 0};
  const int c_20[] = {-31 , 37 , 17 , 0};
  const int c_21[] = {27 , -11 , -23 , 0};
  const int c_22[] = {-35 , 25 , -3 , 0};
  const int c_23[] = {-20 , 23 , -2 , 0};
  const int c_24[] = {7 , -12 , -39 , 0};
  const int c_25[] = {24 , 20 , 10 , 0};
  const int c_26[] = {-38 , 23 , -8 , 0};
  const int c_27[] = {9 , 38 , -31 , 0};
  const int c_28[] = {19 , 40 , 14 , 0};
  const int c_29[] = {-27 , -3 , -23 , 0};
  const int c_30[] = {17 , 14 , -15 , 0};
  const int c_31[] = {21 , 30 , -8 , 0};
  const int c_32[] = {24 , -3 , -13 , 0};
  const int c_33[] = {-17 , -9 , 7 , 0};
  const int c_34[] = {16 , -23 , -1 , 0};
  const int c_35[] = {-18 , -1 , 35 , 0};
  const int c_36[] = {24 , -11 , -30 , 0};
  const int c_37[] = {22 , -7 , 21 , 0};
  const int c_38[] = {21 , -14 , 13 , 0};
  const int c_39[] = {-21 , -26 , -32 , 0};
  const int c_40[] = {36 , 33 , -15 , 0};
  const int c_41[] = {2 , 25 , 6 , 0};
  const int c_42[] = {-4 , -17 , -19 , 0};
  const int c_43[] = {35 , 37 , -5 , 0};
  const int c_44[] = {-29 , -9 , -31 , 0};
  const int c_45[] = {23 , 7 , 40 , 0};
  const int c_46[] = {10 , -7 , -14 , 0};
  const int c_47[] = {40 , 31 , 16 , 0};
  const int c_48[] = {30 , -33 , -10 , 0};
  const int c_49[] = {35 , 12 , 15 , 0};
  const int c_50[] = {-35 , -6 , -7 , 0};
  const int c_51[] = {34 , -35 , 29 , 0};
  const int c_52[] = {-2 , -16 , -13 , 0};
  const int c_53[] = {28 , 8 , -21 , 0};
  const int c_54[] = {29 , -5 , -13 , 0};
  const int c_55[] = {-10 , -17 , -3 , 0};
  const int c_56[] = {-20 , -25 , -6 , 0};
  const int c_57[] = {23 , 16 , -7 , 0};
  const int c_58[] = {-21 , -3 , -25 , 0};
  const int c_59[] = {-27 , -28 , 39 , 0};
  const int c_60[] = {18 , -26 , -32 , 0};
  const int c_61[] = {4 , -25 , 19 , 0};
  const int c_62[] = {-40 , 8 , -29 , 0};
  const int c_63[] = {-28 , 25 , 40 , 0};
  const int c_64[] = {-36 , -14 , 1 , 0};
  const int c_65[] = {-6 , -5 , -31 , 0};
  const int c_66[] = {19 , -40 , 18 , 0};
  const int c_67[] = {25 , -22 , -23 , 0};
  const int c_68[] = {-7 , 40 , -3 , 0};
  const int c_69[] = {-11 , -13 , 15 , 0};
  const int c_70[] = {-27 , 11 , -8 , 0};
  const int c_71[] = {-28 , -14 , -33 , 0};
  const int c_72[] = {-14 , -6 , -10 , 0};
  const int c_73[] = {-26 , -34 , 35 , 0};
  const int c_74[] = {-36 , -10 , 31 , 0};
  const int c_75[] = {-34 , 20 , 19 , 0};
  const int c_76[] = {8 , 35 , -24 , 0};
  const int c_77[] = {37 , -8 , 7 , 0};
  const int c_78[] = {-26 , -22 , -16 , 0};
  const int c_79[] = {-28 , -11 , 6 , 0};
  const int c_80[] = {-13 , 7 , -23 , 0};
  const int c_81[] = {8 , -23 , -5 , 0};
  const int c_82[] = {-18 , 1 , 26 , 0};
  const int c_83[] = {-4 , -8 , 33 , 0};
  const int c_84[] = {-33 , 35 , 3 , 0};
  const int c_85[] = {-18 , -10 , 37 , 0};
  const int c_86[] = {17 , -11 , 8 , 0};
  const int c_87[] = {-12 , 23 , -29 , 0};
  const int c_88[] = {34 , 38 , -17 , 0};
  const int c_89[] = {16 , -20 , -31 , 0};
  const int c_90[] = {37 , 36 , -27 , 0};
  const int c_91[] = {-40 , -17 , 5 , 0};
  const int c_92[] = {-34 , -23 , 6 , 0};
  const int c_93[] = {-10 , -17 , 21 , 0};
  const int c_94[] = {32 , 14 , 5 , 0};
  const int c_95[] = {35 , 29 , 20 , 0};
  const int c_96[] = {-26 , -25 , 3 , 0};
  const int c_97[] = {-9 , -35 , 14 , 0};
  const int c_98[] = {-29 , 30 , -4 , 0};
  const int c_99[] = {-8 , 27 , -38 , 0};
  const int c_100[] = {24 , 9 , 40 , 0};
  const int c_101[] = {-21 , -4 , -5 , 0};
  const int c_102[] = {-17 , 26 , -14 , 0};
  const int c_103[] = {39 , -5 , 32 , 0};
  const int c_104[] = {13 , -18 , -5 , 0};
  const int c_105[] = {16 , -2 , 21 , 0};
  const int c_106[] = {25 , -23 , 37 , 0};
  const int c_107[] = {-35 , 9 , 11 , 0};
  const int c_108[] = {31 , 13 , -29 , 0};
  const int c_109[] = {9 , -30 , 12 , 0};
  const int c_110[] = {27 , -22 , 39 , 0};
  const int c_111[] = {21 , -3 , -32 , 0};
  const int c_112[] = {-37 , 24 , 21 , 0};
  const int c_113[] = {-38 , 27 , 6 , 0};
  const int c_114[] = {17 , -37 , -12 , 0};
  const int c_115[] = {13 , 8 , 7 , 0};
  const int c_116[] = {35 , -18 , -8 , 0};
  const int c_117[] = {-17 , -35 , 16 , 0};
  const int c_118[] = {-20 , -34 , 8 , 0};
  const int c_119[] = {-22 , 33 , -14 , 0};
  const int c_120[] = {-23 , 14 , 35 , 0};
  const int c_121[] = {-37 , -5 , -22 , 0};
  const int c_122[] = {-40 , -19 , -13 , 0};
  const int c_123[] = {-29 , -32 , 5 , 0};
  const int c_124[] = {-20 , 13 , -35 , 0};
  const int c_125[] = {34 , -19 , -24 , 0};
  const int c_126[] = {-12 , 35 , 14 , 0};
  const int c_127[] = {10 , -15 , -29 , 0};
  const int c_128[] = {21 , -4 , 13 , 0};
  const int c_129[] = {6 , -3 , -17 , 0};
  const int c_130[] = {-22 , 36 , -2 , 0};
  const int c_131[] = {-9 , -31 , 22 , 0};
  const int c_132[] = {9 , -4 , 30 , 0};
  const int c_133[] = {-5 , -10 , -31 , 0};
  const int c_134[] = {23 , 2 , -19 , 0};
  const int c_135[] = {-22 , -8 , -7 , 0};
  const int c_136[] = {-25 , 40 , 15 , 0};
  const int c_137[] = {17 , 2 , 34 , 0};
  const int c_138[] = {-30 , 4 , -2 , 0};
  const int c_139[] = {4 , -25 , 10 , 0};
  const int c_140[] = {10 , -21 , -9 , 0};
  const int c_141[] = {-37 , -20 , 12 , 0};
  const int c_142[] = {25 , -5 , -17 , 0};
  const int c_143[] = {27 , 15 , 25 , 0};
  const int c_144[] = {-34 , -13 , -31 , 0};
  const int c_145[] = {-36 , -27 , 23 , 0};
  const int c_146[] = {13 , 31 , 12 , 0};
  const int c_147[] = {-21 , -10 , -38 , 0};
  const int c_148[] = {-1 , -23 , 36 , 0};
  const int c_149[] = {-37 , 16 , 2 , 0};
  const int c_150[] = {-25 , -32 , -16 , 0};
  const int c_151[] = {33 , -37 , 4 , 0};
  const int c_152[] = {-1 , -31 , 27 , 0};
  const int c_153[] = {6 , 30 , 26 , 0};
  const int c_154[] = {-35 , -27 , -9 , 0};
  const int c_155[] = {27 , 18 , 16 , 0};
  const int c_156[] = {40 , -32 , -39 , 0};
  const int c_157[] = {-22 , 40 , -35 , 0};
  const int c_158[] = {-33 , -34 , 5 , 0};
  const int c_159[] = {-5 , -22 , 13 , 0};
  const int c_160[] = {-14 , 38 , 17 , 0};
  const int c_161[] = {-36 , -9 , 6 , 0};
  const int c_162[] = {17 , -26 , -28 , 0};
  const int c_163[] = {-35 , -15 , -24 , 0};
  const int c_164[] = {18 , 15 , 29 , 0};
  const int c_165[] = {6 , -28 , 1 , 0};
  const int c_166[] = {31 , -40 , 30 , 0};
  const int c_167[] = {33 , 3 , -37 , 0};
  const int c_168[] = {-26 , -1 , -39 , 0};

  const int* clauses[] = {
      c_1, c_2, c_3, c_4, c_5, c_6, c_7, c_8, c_9, c_10, 
      c_11, c_12, c_13, c_14, c_15, c_16, c_17, c_18, c_19, c_20, 
      c_21, c_22, c_23, c_24, c_25, c_26, c_27, c_28, c_29, c_30, 
      c_31, c_32, c_33, c_34, c_35, c_36, c_37, c_38, c_39, c_40, 
      c_41, c_42, c_43, c_44, c_45, c_46, c_47, c_48, c_49, c_50, 
      c_51, c_52, c_53, c_54, c_55, c_56, c_57, c_58, c_59, c_60, 
      c_61, c_62, c_63, c_64, c_65, c_66, c_67, c_68, c_69, c_70, 
      c_71, c_72, c_73, c_74, c_75, c_76, c_77, c_78, c_79, c_80, 
      c_81, c_82, c_83, c_84, c_85, c_86, c_87, c_88, c_89, c_90, 
      c_91, c_92, c_93, c_94, c_95, c_96, c_97, c_98, c_99, c_100, 
      c_101, c_102, c_103, c_104, c_105, c_106, c_107, c_108, c_109, c_110, 
      c_111, c_112, c_113, c_114, c_115, c_116, c_117, c_118, c_119, c_120, 
      c_121, c_122, c_123, c_124, c_125, c_126, c_127, c_128, c_129, c_130, 
      c_131, c_132, c_133, c_134, c_135, c_136, c_137, c_138, c_139, c_140, 
      c_141, c_142, c_143, c_144, c_145, c_146, c_147, c_148, c_149, c_150, 
      c_151, c_152, c_153, c_154, c_155, c_156, c_157, c_158, c_159, c_160, 
      c_161, c_162, c_163, c_164, c_165, c_166, c_167, c_168, 
      0 };

  forest* f = makeBinaryForest(40);
  dd_edge M(f);

  clauses2MDD(f, clauses, M, 2);
  check_solution("Test 40", f, M, sol);
  destroyForest(f);
}

void test_50()
{
  const int sol[] = { -1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 
                          1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 0, 
                          1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 
                          1, 0, 1, 0, 1 };

  const int c_1[] = {11 , 43 , 28 , 0};
  const int c_2[] = {37 , 15 , -26 , 0};
  const int c_3[] = {35 , 38 , 5 , 0};
  const int c_4[] = {12 , -36 , 13 , 0};
  const int c_5[] = {-34 , -17 , 23 , 0};
  const int c_6[] = {-19 , 3 , 43 , 0};
  const int c_7[] = {25 , 47 , 40 , 0};
  const int c_8[] = {-12 , -13 , 47 , 0};
  const int c_9[] = {38 , 1 , -39 , 0};
  const int c_10[] = {-46 , 12 , 48 , 0};
  const int c_11[] = {-9 , 3 , 7 , 0};
  const int c_12[] = {36 , 42 , 20 , 0};
  const int c_13[] = {-15 , -19 , 50 , 0};
  const int c_14[] = {48 , -20 , -47 , 0};
  const int c_15[] = {33 , -44 , 6 , 0};
  const int c_16[] = {-50 , -2 , -38 , 0};
  const int c_17[] = {-19 , 23 , -36 , 0};
  const int c_18[] = {19 , -42 , 40 , 0};
  const int c_19[] = {22 , -43 , 32 , 0};
  const int c_20[] = {-45 , -46 , -50 , 0};
  const int c_21[] = {22 , -46 , 37 , 0};
  const int c_22[] = {22 , -13 , 37 , 0};
  const int c_23[] = {25 , -1 , -50 , 0};
  const int c_24[] = {16 , -19 , -21 , 0};
  const int c_25[] = {9 , 49 , -4 , 0};
  const int c_26[] = {28 , -32 , 12 , 0};
  const int c_27[] = {-39 , 31 , 5 , 0};
  const int c_28[] = {45 , 34 , 7 , 0};
  const int c_29[] = {-5 , 37 , -49 , 0};
  const int c_30[] = {-34 , -7 , 18 , 0};
  const int c_31[] = {-6 , -38 , -3 , 0};
  const int c_32[] = {-1 , -47 , 45 , 0};
  const int c_33[] = {34 , -24 , -9 , 0};
  const int c_34[] = {-26 , 10 , -28 , 0};
  const int c_35[] = {37 , 15 , 29 , 0};
  const int c_36[] = {-13 , -32 , -38 , 0};
  const int c_37[] = {-4 , 45 , 35 , 0};
  const int c_38[] = {-47 , -49 , -50 , 0};
  const int c_39[] = {-10 , -45 , 28 , 0};
  const int c_40[] = {37 , 21 , -25 , 0};
  const int c_41[] = {-27 , -29 , 47 , 0};
  const int c_42[] = {16 , -36 , 37 , 0};
  const int c_43[] = {-30 , -8 , -26 , 0};
  const int c_44[] = {-46 , -24 , -11 , 0};
  const int c_45[] = {9 , 50 , -5 , 0};
  const int c_46[] = {-37 , 2 , 3 , 0};
  const int c_47[] = {44 , -48 , -31 , 0};
  const int c_48[] = {-37 , -49 , -45 , 0};
  const int c_49[] = {18 , 21 , 14 , 0};
  const int c_50[] = {16 , -35 , -24 , 0};
  const int c_51[] = {-32 , -13 , 16 , 0};
  const int c_52[] = {-1 , 20 , -41 , 0};
  const int c_53[] = {14 , -33 , 31 , 0};
  const int c_54[] = {-15 , 40 , -25 , 0};
  const int c_55[] = {-24 , -14 , 46 , 0};
  const int c_56[] = {-40 , -22 , 28 , 0};
  const int c_57[] = {6 , 17 , 41 , 0};
  const int c_58[] = {8 , 6 , 44 , 0};
  const int c_59[] = {-10 , -12 , -37 , 0};
  const int c_60[] = {27 , -4 , 35 , 0};
  const int c_61[] = {46 , 8 , -38 , 0};
  const int c_62[] = {-47 , -5 , 45 , 0};
  const int c_63[] = {20 , 32 , 6 , 0};
  const int c_64[] = {45 , -1 , 50 , 0};
  const int c_65[] = {-39 , -17 , 45 , 0};
  const int c_66[] = {-16 , -9 , -35 , 0};
  const int c_67[] = {47 , 17 , 26 , 0};
  const int c_68[] = {33 , 26 , -11 , 0};
  const int c_69[] = {-43 , -3 , -33 , 0};
  const int c_70[] = {12 , 24 , -43 , 0};
  const int c_71[] = {-45 , -28 , -6 , 0};
  const int c_72[] = {21 , 46 , 43 , 0};
  const int c_73[] = {-3 , -26 , 38 , 0};
  const int c_74[] = {-24 , 4 , -43 , 0};
  const int c_75[] = {-32 , 12 , 29 , 0};
  const int c_76[] = {-39 , 26 , 50 , 0};
  const int c_77[] = {43 , 17 , 25 , 0};
  const int c_78[] = {33 , -16 , -23 , 0};
  const int c_79[] = {-29 , -28 , -24 , 0};
  const int c_80[] = {44 , 39 , 9 , 0};
  const int c_81[] = {45 , -46 , -30 , 0};
  const int c_82[] = {-21 , -36 , -39 , 0};
  const int c_83[] = {-44 , 37 , -36 , 0};
  const int c_84[] = {-20 , -6 , -24 , 0};
  const int c_85[] = {44 , 32 , -42 , 0};
  const int c_86[] = {-25 , 5 , -1 , 0};
  const int c_87[] = {-17 , 24 , 26 , 0};
  const int c_88[] = {-43 , 18 , 8 , 0};
  const int c_89[] = {9 , 2 , 1 , 0};
  const int c_90[] = {-44 , -29 , 1 , 0};
  const int c_91[] = {-31 , 6 , 2 , 0};
  const int c_92[] = {34 , -5 , 38 , 0};
  const int c_93[] = {46 , -40 , -20 , 0};
  const int c_94[] = {50 , -2 , 29 , 0};
  const int c_95[] = {-32 , 35 , -27 , 0};
  const int c_96[] = {5 , 36 , 27 , 0};
  const int c_97[] = {-20 , -23 , 14 , 0};
  const int c_98[] = {47 , -7 , 44 , 0};
  const int c_99[] = {16 , 28 , -49 , 0};
  const int c_100[] = {-21 , 49 , -43 , 0};
  const int c_101[] = {-40 , 37 , 36 , 0};
  const int c_102[] = {-9 , 15 , 3 , 0};
  const int c_103[] = {7 , -2 , 48 , 0};
  const int c_104[] = {-1 , 7 , -35 , 0};
  const int c_105[] = {41 , 27 , 32 , 0};
  const int c_106[] = {-40 , 14 , -29 , 0};
  const int c_107[] = {28 , -11 , -27 , 0};
  const int c_108[] = {-7 , -42 , -33 , 0};
  const int c_109[] = {16 , 44 , 45 , 0};
  const int c_110[] = {-14 , -11 , -26 , 0};
  const int c_111[] = {27 , 12 , 37 , 0};
  const int c_112[] = {-35 , 2 , -46 , 0};
  const int c_113[] = {45 , -2 , -32 , 0};
  const int c_114[] = {2 , 14 , 27 , 0};
  const int c_115[] = {-26 , 27 , 39 , 0};
  const int c_116[] = {-30 , -28 , 34 , 0};
  const int c_117[] = {12 , -39 , 23 , 0};
  const int c_118[] = {-17 , 26 , 2 , 0};
  const int c_119[] = {38 , -42 , -4 , 0};
  const int c_120[] = {-40 , -34 , -5 , 0};
  const int c_121[] = {-33 , 18 , -24 , 0};
  const int c_122[] = {43 , 44 , 19 , 0};
  const int c_123[] = {-16 , -45 , 26 , 0};
  const int c_124[] = {5 , -13 , 45 , 0};
  const int c_125[] = {-5 , -21 , 44 , 0};
  const int c_126[] = {-33 , 10 , 47 , 0};
  const int c_127[] = {38 , 23 , -42 , 0};
  const int c_128[] = {45 , -9 , -27 , 0};
  const int c_129[] = {27 , -10 , 33 , 0};
  const int c_130[] = {31 , 24 , 8 , 0};
  const int c_131[] = {42 , 33 , 41 , 0};
  const int c_132[] = {-19 , -1 , -5 , 0};
  const int c_133[] = {11 , 49 , 33 , 0};
  const int c_134[] = {-45 , 23 , -44 , 0};
  const int c_135[] = {-43 , -3 , 21 , 0};
  const int c_136[] = {11 , -44 , 47 , 0};
  const int c_137[] = {10 , 24 , -33 , 0};
  const int c_138[] = {37 , 30 , 25 , 0};
  const int c_139[] = {48 , 28 , 37 , 0};
  const int c_140[] = {17 , -18 , 33 , 0};
  const int c_141[] = {7 , 13 , -19 , 0};
  const int c_142[] = {-37 , 44 , 16 , 0};
  const int c_143[] = {-1 , 3 , -33 , 0};
  const int c_144[] = {-3 , 21 , -23 , 0};
  const int c_145[] = {-22 , 17 , 15 , 0};
  const int c_146[] = {38 , 6 , 47 , 0};
  const int c_147[] = {42 , 16 , 28 , 0};
  const int c_148[] = {-25 , 42 , -14 , 0};
  const int c_149[] = {15 , -28 , 22 , 0};
  const int c_150[] = {40 , 1 , -19 , 0};
  const int c_151[] = {23 , 48 , 10 , 0};
  const int c_152[] = {-31 , 6 , -18 , 0};
  const int c_153[] = {19 , -26 , 44 , 0};
  const int c_154[] = {40 , 30 , 37 , 0};
  const int c_155[] = {30 , -12 , 37 , 0};
  const int c_156[] = {37 , 3 , 35 , 0};
  const int c_157[] = {26 , 14 , -35 , 0};
  const int c_158[] = {3 , 50 , 7 , 0};
  const int c_159[] = {-49 , 18 , -21 , 0};
  const int c_160[] = {34 , -33 , -40 , 0};
  const int c_161[] = {-24 , -25 , -48 , 0};
  const int c_162[] = {6 , -42 , 17 , 0};
  const int c_163[] = {42 , -7 , -50 , 0};
  const int c_164[] = {45 , -32 , 7 , 0};
  const int c_165[] = {-10 , 36 , -50 , 0};
  const int c_166[] = {17 , -38 , 21 , 0};
  const int c_167[] = {34 , -44 , -18 , 0};
  const int c_168[] = {43 , 50 , 26 , 0};
  const int c_169[] = {39 , 27 , -33 , 0};
  const int c_170[] = {-9 , 5 , -32 , 0};
  const int c_171[] = {-25 , -44 , 26 , 0};
  const int c_172[] = {14 , -47 , -24 , 0};
  const int c_173[] = {-37 , -27 , 19 , 0};
  const int c_174[] = {-33 , -3 , 20 , 0};
  const int c_175[] = {-41 , -31 , 37 , 0};
  const int c_176[] = {-32 , -25 , 24 , 0};
  const int c_177[] = {-2 , 14 , 1 , 0};
  const int c_178[] = {-8 , -46 , 39 , 0};
  const int c_179[] = {22 , 15 , -3 , 0};
  const int c_180[] = {46 , -1 , 8 , 0};
  const int c_181[] = {23 , -8 , 40 , 0};
  const int c_182[] = {11 , 40 , 14 , 0};
  const int c_183[] = {-49 , 3 , -2 , 0};
  const int c_184[] = {-2 , -10 , 45 , 0};
  const int c_185[] = {36 , 26 , 40 , 0};
  const int c_186[] = {40 , -49 , -30 , 0};
  const int c_187[] = {-49 , 28 , -44 , 0};
  const int c_188[] = {13 , -48 , -15 , 0};
  const int c_189[] = {49 , -9 , 31 , 0};
  const int c_190[] = {-30 , -39 , -23 , 0};
  const int c_191[] = {-1 , -21 , 29 , 0};
  const int c_192[] = {-41 , -48 , 20 , 0};
  const int c_193[] = {45 , -46 , -35 , 0};
  const int c_194[] = {8 , -32 , -49 , 0};
  const int c_195[] = {22 , 11 , -12 , 0};
  const int c_196[] = {37 , -34 , 28 , 0};
  const int c_197[] = {-3 , -38 , -16 , 0};
  const int c_198[] = {-33 , -37 , 42 , 0};
  const int c_199[] = {9 , -3 , -6 , 0};
  const int c_200[] = {13 , -29 , 20 , 0};
  const int c_201[] = {5 , -43 , -4 , 0};
  const int c_202[] = {-5 , -45 , -16 , 0};
  const int c_203[] = {42 , -8 , -39 , 0};
  const int c_204[] = {-34 , -3 , 39 , 0};
  const int c_205[] = {25 , -7 , -50 , 0};
  const int c_206[] = {25 , -28 , -12 , 0};
  const int c_207[] = {12 , 45 , -44 , 0};
  const int c_208[] = {-36 , -44 , -38 , 0};
  const int c_209[] = {26 , 45 , -25 , 0};
  const int c_210[] = {9 , 41 , 22 , 0};

  const int* clauses[] = {
      c_1, c_2, c_3, c_4, c_5, c_6, c_7, c_8, c_9, c_10, 
      c_11, c_12, c_13, c_14, c_15, c_16, c_17, c_18, c_19, c_20, 
      c_21, c_22, c_23, c_24, c_25, c_26, c_27, c_28, c_29, c_30, 
      c_31, c_32, c_33, c_34, c_35, c_36, c_37, c_38, c_39, c_40, 
      c_41, c_42, c_43, c_44, c_45, c_46, c_47, c_48, c_49, c_50, 
      c_51, c_52, c_53, c_54, c_55, c_56, c_57, c_58, c_59, c_60, 
      c_61, c_62, c_63, c_64, c_65, c_66, c_67, c_68, c_69, c_70, 
      c_71, c_72, c_73, c_74, c_75, c_76, c_77, c_78, c_79, c_80, 
      c_81, c_82, c_83, c_84, c_85, c_86, c_87, c_88, c_89, c_90, 
      c_91, c_92, c_93, c_94, c_95, c_96, c_97, c_98, c_99, c_100, 
      c_101, c_102, c_103, c_104, c_105, c_106, c_107, c_108, c_109, c_110, 
      c_111, c_112, c_113, c_114, c_115, c_116, c_117, c_118, c_119, c_120, 
      c_121, c_122, c_123, c_124, c_125, c_126, c_127, c_128, c_129, c_130, 
      c_131, c_132, c_133, c_134, c_135, c_136, c_137, c_138, c_139, c_140, 
      c_141, c_142, c_143, c_144, c_145, c_146, c_147, c_148, c_149, c_150, 
      c_151, c_152, c_153, c_154, c_155, c_156, c_157, c_158, c_159, c_160, 
      c_161, c_162, c_163, c_164, c_165, c_166, c_167, c_168, c_169, c_170, 
      c_171, c_172, c_173, c_174, c_175, c_176, c_177, c_178, c_179, c_180, 
      c_181, c_182, c_183, c_184, c_185, c_186, c_187, c_188, c_189, c_190, 
      c_191, c_192, c_193, c_194, c_195, c_196, c_197, c_198, c_199, c_200, 
      c_201, c_202, c_203, c_204, c_205, c_206, c_207, c_208, c_209, c_210, 
      
      0 };

  forest* f = makeBinaryForest(50);
  dd_edge M(f);

  clauses2MDD(f, clauses, M, 2);
  check_solution("Test 50", f, M, sol);
  destroyForest(f);
}


int main()
{
  initialize(); // initialize MEDDLY

  tiny_test();
  test_10();
  test_20();
  test_30();
  test_40();
  test_50();

  cleanup();  // MEDDLY cleanup
  return 0;
}

