
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



// This file demonstrates the construction of MDDs from a truth table.
//
// Data:
// Assuming that there are three questions (A, B and C) whose
// answers lead us to a value of either True (T) or False (F)
// as given by the following truth table.
//
// A  B  C  Result
// 0  0  0  T
// 0  0  1  F
// 0  1  0  F
// 0  1  1  T
// 1  0  0  T
// 1  0  1  T
// 1  1  0  F
// 1  1  1  T
// 2  0  0  F
// 2  0  1  F
// 2  1  0  F
// 2  1  1  T
//
// Note the range for each: A [0..2], B[0,1], C[0, 1]
//
// Constructing MDD for the truth table:
// 1. There are 6 True entries in the above truth table.
// 2. Construct a array of size 6 where each element of the array
//    represents a True entry. For example,
//    Entries[0] = {0, 0, 0}
//    Entries[1] = {0, 1, 1}
//    Entries[2] = {1, 0, 0}
//    Entries[3] = {1, 0, 1}
//    Entries[4] = {1, 1, 1}
//    Entries[5] = {2, 1, 1}
// 3. Create an MDD representation for each element and then
//    performing a union of those MDDs.
//

#include <iostream>
#include <string.h>
#include "meddly.h"
#include "meddly_expert.h"

using namespace MEDDLY;

int main(int argv, char *argc[])
{
  ostream_output meddlyout(std::cout);
  initialize();
  std::cout << getLibraryInfo();

  // Create a domain
  domain* d = createDomain();
  assert(d != 0);

  // We have three variables A, B and C.
  // A = bottom level, C = top level
  const int N = 3;
  int bounds[N];
  bounds[0] = 3;
  bounds[1] = 2;
  bounds[2] = 2;

  // Create variable in the above domain
  d->createVariablesBottomUp(bounds, N);
  std::cout << "Created domain with "
    << d->getNumVariables()
    << " variables\n";

  // Create a forest in the above domain
  forest* mdd = d->createForest(
      false,                    // this is not a relation
      forest::BOOLEAN,          // terminals are either true or false
      forest::MULTI_TERMINAL    // disables edge-labeling
      );
  assert(mdd != 0);

  // Display forest properties
  std::cout << "Created forest in this domain with:"
    << "\n  Relation:\tfalse"
    << "\n  Range Type:\tBOOLEAN"
    << "\n  Edge Label:\tMULTI_TERMINAL"
    << "\n";

  // Create an element to insert in the MDD
  // Note that this is of size (N + 1), since [0] is a special level handle
  // dedicated to terminal nodes.
  int* elementList[1];
  elementList[0] = new int[N + 1];

  // First element
  elementList[0][1] = 0; elementList[0][2] = 0; elementList[0][3] = 0;
  dd_edge first(mdd);
  mdd->createEdge(elementList, 1, first);
  std::cout << "\nCreated element [0 0 0]: "
    << "node: " << first.getNode()
    << ", level: " << first.getLevel()
    << "\n";
  first.show(meddlyout, 2);

  // Second element
  elementList[0][1] = 0; elementList[0][2] = 1; elementList[0][3] = 1;
  dd_edge second(mdd);
  mdd->createEdge(elementList, 1, second);
  std::cout << "\nCreated element [0 1 1]: "
    << "node: " << second.getNode()
    << ", level: " << second.getLevel()
    << "\n";
  second.show(meddlyout, 2);

  // Third, Fourth, Fifth and Sixth elements are created at once
  int* eList[4];
  for (int i = 0; i < 4; ++i) { eList[i] = new int[N + 1]; }
  eList[0][1] = 1; eList[0][2] = 0; eList[0][3] = 0;
  eList[1][1] = 1; eList[1][2] = 0; eList[1][3] = 1;
  eList[2][1] = 1; eList[2][2] = 1; eList[2][3] = 1;
  eList[3][1] = 2; eList[3][2] = 1; eList[3][3] = 1;
  dd_edge theRest(mdd);
  mdd->createEdge(eList, 4, theRest);
  std::cout << "\nCreated elements [1 0 0], [1 0 1], [1 1 1], [2 1 1]: "
    << "node: " << theRest.getNode()
    << ", level: " << theRest.getLevel()
    << "\n";
  theRest.show(meddlyout, 2);

  // Add them all together (using union)
  // 1. Get a handle to the compute manager (through which operations
  //    are performed).
  // 2. Call compute manager with the operation code for UNION

  // do all = union(first, second)
  std::cout << "\nUnion("
    << first.getNode() << ", " << second.getNode() << "): ";
  dd_edge all(mdd);
  apply(UNION, first, second, all);
  std::cout << all.getNode() << "\n";
  all.show(meddlyout, 2);

  // do all = union(all, theRest)
  // -- note that all is over-written with the result of the union
  std::cout << "\nUnion("
    << all.getNode() << ", " << theRest.getNode() << "): ";
  apply(UNION, all, theRest, all);
  std::cout << all.getNode() << "\n";
  all.show(meddlyout, 2);

  // intersect = intersection(theRest, all)
  std::cout << "\nIntersection("
    << theRest.getNode() << ", " << all.getNode() << "): ";
  dd_edge intersect(mdd);
  apply(INTERSECTION, theRest, all, intersect);
  std::cout << intersect.getNode() << "\n";
  intersect.show(meddlyout, 2);

  // create edge reprenting terminal node TRUE
  dd_edge one(mdd);
  mdd->createEdge(true, one);
  std::cout << "\nTerminal node, TRUE: " << one.getNode() << "\n";
  one.show(meddlyout, 2);

  // diff = difference(one, all)
  std::cout << "\nUsing difference to compute complement, "
    << "Difference(" << one.getNode() << ", " << all.getNode() << "): ";
  dd_edge diff(mdd);
  apply(DIFFERENCE, one, all, diff);
  std::cout << diff.getNode() << "\n";
  diff.show(meddlyout, 2);

  operation::showMonolithicComputeTable(meddlyout, true);
  cleanup();
  return 0;
}
