
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



/**
 * build_expression.cc
 *
 * Demonstrates use of
 *   createEdgeForVar(int, bool, dd_edge&), and
 *   createEdgeForVar(int, bool, int*, dd_edge&)
 *   createEdgeForVar(int, bool, float*, dd_edge&)
 */

#include <iostream>
#include <vector>

#include "meddly.h"
#include "timer.h"

using namespace MEDDLY;

#define USE_REALS 1

#if USE_REALS
  typedef float element_type;
#else
  typedef int element_type;
#endif

const int nVariables = 3;
int variableBound = 8;

// Builds the expression y1 + 2*y2 + 3*y3
// Assumes states has atleast 3 levels, each of size 2.
// Assumed variable order: y3 y2 y1 TERMINALS.
// Uses createEdgeForVar(int, bool, dd_edge&)
dd_edge buildExpression(forest* states);

// Similar to buildExpression, but uses a array to define the terminals.
// Uses createEdgeForVar(int, bool, int*/float*, dd_edge&)
dd_edge buildExpressionWithTerms(forest* states);

// Builds the expression y1 + 2*y2 + 3*y3 == y1'
// Assumed variable order: y3 y2 y1 TERMINALS.
// Uses createEdgeForVar(int, bool, dd_edge&)
dd_edge buildTransitionExpression(forest* mtmxd);

// Builds the expression y1 + 1 == y1'
// Assumed variable order: y3 y2 y1 TERMINALS.
// Uses createEdgeForVar(int, bool, dd_edge&)
dd_edge buildIncrY1(forest* mtmxd);

// Builds the expression y2 + 1 == y2'
// Assumed variable order: y3 y2 y1 TERMINALS.
// Uses createEdgeForVar(int, bool, dd_edge&)
dd_edge buildIncrY2(forest* mtmxd);

// Builds the expression y3 + 1 == y3'
// Assumed variable order: y3 y2 y1 TERMINALS.
// Uses createEdgeForVar(int, bool, dd_edge&)
dd_edge buildIncrY3(forest* mtmxd);





int main(int argc, char *argv[])
{
  MEDDLY::initialize();
  
  FILE_output mout(stdout);

  // initialize the variable bounds array to provide to the domain
  int bounds[nVariables];
  for (int i = 0; i < nVariables; ++i)
    bounds[i] = variableBound;

  // Create a domain
  domain *d = createDomainBottomUp(bounds, nVariables);
  assert(d != 0);

  // Create an MXD forest in this domain (to store states)

#if USE_REALS
  forest::range_type range = forest::REAL;
#else
  forest::range_type range = forest::INTEGER;
#endif

  forest* states = d->createForest(false, range, forest::MULTI_TERMINAL);
  assert(states != 0);

#if 1
  dd_edge expr = buildExpression(states);
  // expr.show(mout, 2);
#else
  dd_edge expr = buildExpressionWithTerms(states);
  // expr.show(mout, 2);
#endif

  forest* mtmxd = d->createForest(true, range, forest::MULTI_TERMINAL);
  assert(mtmxd != 0);

  dd_edge incrY1 = buildIncrY1(mtmxd);
  fprintf(stdout, "\nMTMXD for (y1' == y1 + 1):\n\n");
  incrY1.show(mout, 2);
  
  dd_edge postImage(states);
  int element[] = {0, 0, 0, 0, 0};
  int* elements[] = { element };
  element_type terms[] = {element_type(1)};
  states->createEdge((int**)elements, (element_type*)terms, 1, postImage);
  // postImage *= expr;

  fprintf(stdout, "-----------------------------------------------------\n");
  fprintf(stdout, "\nMTMDD for [0 0 0 = 1]:\n\n");
  postImage.show(mout, 2); 

  apply(POST_IMAGE, postImage, incrY1, postImage);
  // postImage *= expr;
  fprintf(stdout, "-----------------------------------------------------\n");
  fprintf(stdout, "\nMTMDD after POST_IMAGE:\n\n");
  postImage.show(mout, 2);

  apply(PRE_IMAGE, postImage, incrY1, postImage);
  // postImage *= expr;
  fprintf(stdout, "-----------------------------------------------------\n");
  fprintf(stdout, "\nMTMDD after PRE_IMAGE:\n\n");
  postImage.show(mout, 2);

  // Cleanup
  MEDDLY::destroyDomain(d);
  MEDDLY::cleanup();

  return 0;
}




dd_edge buildExpression(forest* states)
{
  // Building expression y1 + 2*y2 + 3*y3

  // ---- Building y1 ----

  // y1
  dd_edge y1(states);
  states->createEdgeForVar(1, false, y1);

  // ---- Building 2*y2 ----

  // y2
  dd_edge y2(states);
  states->createEdgeForVar(2, false, y2);

  // constant 2
  dd_edge cons2(states);
#if USE_REALS
  states->createEdge(2.0f, cons2);
#else
  states->createEdge(2, cons2);
#endif

  // 2y2
  y2 *= cons2;

  // ---- Building 3*y3 ----

  // y3
  dd_edge y3(states);
  states->createEdgeForVar(3, false, y3);

  // constant 3
  dd_edge cons3(states);
#if USE_REALS
  states->createEdge(3.0f, cons3);
#else
  states->createEdge(3, cons3);
#endif
  
  // 3y3
  y3 *= cons3;

  // ---- Building y1 + 2*y2 + 3*y3 ----

  y1 = y1 + y2 + y3;
  
  return y1;
}



dd_edge buildExpressionWithTerms(forest* states)
{
  // Building expression y1 + 2*y2 + 3*y3

#if USE_REALS
  float terms[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
#else
  int terms[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
#endif

  // ---- Building y1 ----

  // y1
  dd_edge y1(states);
  states->createEdgeForVar(1, false, terms, y1);

  // ---- Building 2*y2 ----

  // y2
  dd_edge y2(states);
  states->createEdgeForVar(2, false, terms, y2);

  // constant 2
  dd_edge cons2(states);
#if USE_REALS
  states->createEdge(2.0f, cons2);
#else
  states->createEdge(2, cons2);
#endif

  // 2y2
  y2 *= cons2;

  // ---- Building 3*y3 ----

  // y3
  dd_edge y3(states);
  states->createEdgeForVar(3, false, terms, y3);

  // constant 3
  dd_edge cons3(states);
#if USE_REALS
  states->createEdge(3.0f, cons3);
#else
  states->createEdge(3, cons3);
#endif
  
  // 3y3
  y3 *= cons3;

  // ---- Building y1 + 2*y2 + 3*y3 ----

  y1 = y1 + y2 + y3;

  return y1;
}




dd_edge buildMultiplierForVariable(forest* mtmxd, int level)
{
  assert(mtmxd != 0);
  int nLevels = mtmxd->getDomain()->getNumVariables() + 1;
  std::vector<int> from(nLevels, -2);
  std::vector<int> to(nLevels, -2);
  int* fromArray[] = {&from[0]};
  int* toArray[] = {&to[0]};
  element_type terms[] = {element_type(1)};

  from[0] = 0;
  to[0] = 0;
  from[level] = -1;
  to[level] = -1;

  dd_edge multiplier(mtmxd);
  mtmxd->createEdge(fromArray, toArray, (element_type*)terms, 1, multiplier);

  return multiplier;
}


dd_edge buildIncrVariable(forest* mtmxd, int level)
{
  // y in comments represents level

  // Building expression y + 1 == y'

  // ---- Building y ----

  // y
  dd_edge y(mtmxd);
  mtmxd->createEdgeForVar(level, false, y);

  // constant 1
  dd_edge cons1(mtmxd);
#if USE_REALS
  mtmxd->createEdge(1.0f, cons1);
#else
  mtmxd->createEdge(1, cons1);
#endif

  // y + 1
  y += cons1;

  // ---- Building y' ----

  dd_edge yPrime(mtmxd);
  mtmxd->createEdgeForVar(level, true, yPrime);

  // ---- Building y + 1 == y' ----
  apply(EQUAL, yPrime, y, yPrime);

  // Make the rest of the transitions into "don't change"
  dd_edge multiplier = buildMultiplierForVariable(mtmxd, level);

  yPrime *= multiplier;

  return yPrime;
}


dd_edge buildIncrY1(forest* mtmxd)
{
  // Building expression y1 + 1 == y1'
  return buildIncrVariable(mtmxd, 1);
}


dd_edge buildIncrY2(forest* mtmxd)
{
  // Building expression y2 + 1 == y2'
  return buildIncrVariable(mtmxd, 2);
}


dd_edge buildIncrY3(forest* mtmxd)
{
  // Building expression y3 + 1 == y3'
  return buildIncrVariable(mtmxd, 3);
}


dd_edge buildTransitionExpression(forest* mtmxd)
{
  // Building expression y1 + 2*y2 + 3*y3 == y1'

  // ---- Building y1 ----

  // y1
  dd_edge y1(mtmxd);
  mtmxd->createEdgeForVar(1, false, y1);

  // ---- Building 2*y2 ----

  // y2
  dd_edge y2(mtmxd);
  mtmxd->createEdgeForVar(2, false, y2);

  // constant 2
  dd_edge cons2(mtmxd);
#if USE_REALS
  mtmxd->createEdge(2.0f, cons2);
#else
  mtmxd->createEdge(2, cons2);
#endif

  // 2y2
  y2 *= cons2;

  // ---- Building 3*y3 ----

  // y3
  dd_edge y3(mtmxd);
  mtmxd->createEdgeForVar(3, false, y3);

  // constant 3
  dd_edge cons3(mtmxd);
#if USE_REALS
  mtmxd->createEdge(3.0f, cons3);
#else
  mtmxd->createEdge(3, cons3);
#endif

  // 3y3
  y3 *= cons3;

  // ---- Building y1 + 2*y2 + 3*y3 ----

  y1 = y1 + y2 + y3;

  // ---- Building y1' ----
  dd_edge y1Prime(mtmxd);
  mtmxd->createEdgeForVar(1, true, y1Prime);

  // y1Prime.show(mout, 2);

  apply(EQUAL, y1Prime, y1, y1Prime);

  // y1Prime.show(mout, 2);

  dd_edge multiplier = buildMultiplierForVariable(mtmxd, 1);
  y1Prime *= multiplier;

  // y1Prime.show(mout, 2);

  return y1Prime;
}
