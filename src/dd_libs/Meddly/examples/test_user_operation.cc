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


/*! \file test_user_operation.cc
    
    Implementing a user-defined operation using Meddly's expert-interface.

    Operation: AndSum(a, b)
    Truth Table:
      a   b   AndSum(a,b)
      0   y   0
      x   0   0
      x   y   x+y
*/


#include <vector>
#include "meddly.h"
#include "meddly_expert.h"

#define MAX(A, B) ((A > B)? A: B)
#define MIN(A, B) ((A < B)? A: B)

using namespace std;
using namespace MEDDLY;

expert_forest* f = 0;

// Returns a dd_edge representing AndSum(e1, e2)
dd_edge AndSum(dd_edge& e1, dd_edge& e2);

// Helper function for AndSum(e1, e2).
// Returns a MDD node representing AndSum(n1, n2).
int AndSum(int n1, int n2);

// Print all the elements (minterms) in e in lexicographic order.
void printElements(FILE* strm, dd_edge& e);

int main(int argc, char* argv[])
{
  // MEDDLY::intialize();
  initialize();

  // 4 levels in the decision diagrams (excl. terminals).
  // Each level is of size 2 (i.e. binary).
  const int nLevels = 4;
  int sizes[nLevels] = {2, 2, 2, 2};

  // Build a domain for the levels defined above.
  domain *d = 0;
  if (0 == (d = createDomainBottomUp(sizes, nLevels))) {
    fprintf(stderr, "Couldn't create domain\n");
    return 1;
  }

  // Create a forest to store MTMDDs. Terminal values being integers.
  forest *f = 0;
  if (0 == (f = d->createForest(false, forest::INTEGER, forest::MULTI_TERMINAL))) {
    fprintf(stderr, "Couldn't create forest\n");
    return 1;
  }

  // Array for storing a minterm (i.e. an MDD element).
  int elem[nLevels+1];
  int* elems[1];
  elems[0] = elem;

  // Value associated with the minterm.
  int terms[1];

  // Create a set representing: (0, ?, ?, ?) --> {1}
  // i.e. {
  //       (0, 0, 0, 0) --> 1,
  //       (0, 0, 0, 1) --> 1,
  //       (0, 0, 1, 0) --> 1,
  //       (0, 0, 1, 1) --> 1,
  //       (0, 1, 0, 0) --> 1,
  //       (0, 1, 0, 1) --> 1,
  //       (0, 1, 1, 0) --> 1,
  //       (0, 1, 1, 1) --> 1}

  elem[0] = 0;    // Level 0 represents the terminal level in MEDDLY.
                  // Setting this is unnecessary, since forest::createEdge()
                  // uses a separate array for assigning terminal values
                  // to each minterm.
  elem[1] = -1;   // Node that "-1" is a short-cut for "don't change".
                  // Refer to forest::createEdge() for further information.
  elem[2] = -1;
  elem[3] = -1;
  elem[4] = 0;
  terms[0] = 1;

  dd_edge e1(f);
  f->createEdge(elems, terms, 1, e1);   // Note that both the minterms and the
                                        // corresponding terminal values are
                                        // provided to forest::createEdge().
  fprintf(stdout, "MDD A:\n");
  e1.show(stdout, 2);
  fprintf(stdout, "\n");
  fprintf(stdout, "Elements in MDD A:\n");
  printElements(stdout, e1);
  fprintf(stdout, "\n");

  // Create a set representing: (?, ?, ?, 1) --> {1}
  // i.e. {
  //       (0, 0, 0, 1) --> 2,
  //       (0, 0, 1, 1) --> 2,
  //       (0, 1, 0, 1) --> 2,
  //       (0, 1, 1, 1) --> 2,
  //       (1, 0, 0, 1) --> 2,
  //       (1, 0, 1, 1) --> 2,
  //       (1, 1, 0, 1) --> 2,
  //       (1, 1, 1, 1) --> 2}

  elem[0] = 0;
  elem[1] = 1;
  elem[2] = -1;
  elem[3] = -1;
  elem[4] = -1;
  terms[0] = 2;

  dd_edge e2(f);
  f->createEdge(elems, terms, 1, e2);
  fprintf(stdout, "MDD B:\n");
  e2.show(stdout, 2);
  fprintf(stdout, "\n");
  fprintf(stdout, "Elements in MDD B:\n");
  printElements(stdout, e2);
  fprintf(stdout, "\n");

  // e3 = e1 + e2
  dd_edge e3(e1);
  e3 += e2;
  fprintf(stdout, "Sum(A, B):\n");
  e3.show(stdout, 2);
  fprintf(stdout, "\n");
  fprintf(stdout, "Elements in Sum(A, B):\n");
  printElements(stdout, e3);
  fprintf(stdout, "\n");

  // e4 = e1 x e2
  dd_edge e4(e1);
  e4 *= e2;
  fprintf(stdout, "Multiply(A,B):\n");
  e4.show(stdout, 2);
  fprintf(stdout, "\n");
  fprintf(stdout, "Elements in Multiply(A, B):\n");
  printElements(stdout, e4);
  fprintf(stdout, "\n");

  // e5 = AndSum(e1, e2)
  dd_edge e5 = AndSum(e1, e2);
  fprintf(stdout, "AndSum(A,B):\n");
  e5.show(stdout, 2);
  fprintf(stdout, "\n");
  fprintf(stdout, "Elements in AndSum(A, B):\n");
  printElements(stdout, e5);
  fprintf(stdout, "\n");

  // e6 = e4 > 1
  dd_edge e6(f);
  int termOne = static_cast<expert_forest*>(f)->getTerminalNode(1);
  e6.set(termOne, 0, static_cast<expert_forest*>(f)->getNodeLevel(termOne));
  // e3 = e1 + e2
  // e4 = e1 x e2;
  // e6 = e4 > 1;
  // e6 *= e3
  apply(GREATER_THAN, e4, e6, e6);
  e6 *= e3;
  fprintf(stdout, "(A + B) x ((A + B) > 1):\n");
  e6.show(stdout, 2);
  fprintf(stdout, "\n");
  fprintf(stdout, "Elements in (A + B) x ((A + B) > 1):\n");
  printElements(stdout, e6);
  fprintf(stdout, "\n");

  // Cleanup
  cleanup();
#if 0
  destroyForest(f);
  destroyDomain(d);
#endif

  return 0;
}


// < op1, <op2, result> >
std::map<int, std::map<int, int> > cache;


void printCache(FILE* strm, expert_forest* f)
{
  fprintf(strm, "Cache Table:\n");
  for (map<int, map<int, int> >::iterator i = cache.begin();
      i != cache.end(); ++i) {
    map<int, int>& j = i->second;
    for (map<int, int>::iterator jIter = j.begin(); jIter != j.end(); ++jIter) {
      fprintf(strm, "[(%d, %d) --> %d]\n",
          i->first, jIter->first, jIter->second);
    }
  }
  fprintf(strm, "\n");
}


void clearCache(expert_forest* f)
{
  printCache(stdout, f);
  for (map<int, map<int, int> >::iterator i = cache.begin();
      i != cache.end(); ++i) {
    f->unlinkNode(i->first);
    map<int, int>& j = i->second;
    for (map<int, int>::iterator jIter = j.begin(); jIter != j.end(); ++jIter) {
      f->unlinkNode(jIter->first);
      f->unlinkNode(jIter->second);
    }
  }
  cache.clear();
}


void saveInCache(expert_forest* f, int nodeA, int nodeB, int nodeC)
{
  f->linkNode(nodeA);
  f->linkNode(nodeB);
  f->linkNode(nodeC);
  map<int, map<int, int> >::iterator iter = cache.find(nodeA);
  if (iter == cache.end()) {
    static map<int, int> temp;
    cache[nodeA] = temp;
  }
  (cache[nodeA])[nodeB] = nodeC;
}


bool findInCache(expert_forest* f, int nodeA, int nodeB, int& result)
{
  map<int, map<int, int> >::iterator i = cache.find(nodeA);
  if (i == cache.end()) return false;
  map<int, int>::iterator j = (i->second).find(nodeA);
  if (j == (i->second).end()) return false;
  result = j->second;
  return true;
}


dd_edge AndSum(dd_edge& e1, dd_edge& e2)
{
  assert(e1.getForest() == e2.getForest());
  // f is used by the recursive AndSum(int, int).
  f = static_cast<expert_forest*>(e1.getForest());
  int resultNode = AndSum(e1.getNode(), e2.getNode());
  clearCache(f);
  dd_edge result(f);
  result.set(resultNode, 0, f->getNodeLevel(resultNode));
  return result;
}


int AndSum(int n1, int n2)
{
  // Construct result such that:
  //    If n1[i] == 0 OR n2[i] == 0, result[i] = 0.
  //    Otherwise, result[i] = n1[i] + n2[i],

  // Terminal condition for recursion
  if (0 == n1 || 0 == n2) return 0;
  if (f->isTerminalNode(n1) && f->isTerminalNode(n2)) {
    // n1 and n2 are terminal nodes.
    // Extract the terminal values before adding them.
    // The sum is converted to a terminal node before it is returned.
    return f->getTerminalNode(f->getInteger(n1) + f->getInteger(n2));
  }

  // AndSum(n1, n2) == AndSum(n2, n1).
  // Therefore, to reduce the number of cache table entries and at the same time
  // increase the number of cache hits, sort n1 and n2.
  if (n1 > n2) { int temp = n1; n1 = n2; n2 = temp; }

  // Search in cache table
  int resultNode = 0;
  if (findInCache(f, n1, n2, resultNode)) {
    // Found
    f->linkNode(resultNode);
    return resultNode;
  }

  // n1 and n2 may be on different levels (since were working with
  // Fully Reduced MDDs). Therefore we need to deal with the following
  // situations:
  // (1) n1 is lower than n2.
  // (2) n1 is higher than n2.
  // (3) n1 and n2 are on the same level.

  // The result will be at higher of the two levels.
  int resultLevel = MAX(f->getNodeLevel(n1), f->getNodeLevel(n2));

  // Vector for storing the children of the resulting node.
  vector<int> result;
  vector<int> n1Dptrs;
  vector<int> n2Dptrs;

  // If n1 is higher than n2 or at the same level as n2, we need to expand
  // n1. Therefore, read its downpointers into a vector.
  if (resultLevel == f->getNodeLevel(n1)) {
    assert(f->getDownPtrs(n1, n1Dptrs));
  } else {
    // n1 is below the result-level.
    // An equivalent node at the result-level is simply a node with all its
    // downpointers pointing to n1.
    n1Dptrs.resize(f->getLevelSize(resultLevel), n1);
  }

  // If n2 is higher than n1 or at the same level as n1, we need to expand
  // n2. Therefore, read its downpointers into a vector.
  if (resultLevel == f->getNodeLevel(n2)) {
    assert(f->getDownPtrs(n2, n2Dptrs));
  } else {
    // n2 is below the result-level.
    // An equivalent node at the result-level is simply a node with all its
    // downpointers pointing to n2.
    n2Dptrs.resize(f->getLevelSize(resultLevel), n2);
  }

  // Only need to consider the indexes common to both nodes (since the rest
  // are zeroes).
  result.resize( MIN(n1Dptrs.size(), n2Dptrs.size()) , 0);

  // For each i in result.size(), result[i] = AndSum(n1Dptrs[i], n2Dptrs[i])
  for (unsigned i = 0; i < result.size(); i++) {
    result[i] = AndSum(n1Dptrs[i], n2Dptrs[i]);
  }

  int tempNode = f->createTempNode(resultLevel, result);
  resultNode = f->reduceNode(tempNode);

  // save result to cache.
  saveInCache(f, n1, n2, resultNode);

  // Alternatively, user the cache provided by MEDDLY by building an
  // operation derived from MEDDLY::operation.

  return resultNode;
}


void printElements(FILE* strm, dd_edge& e)
{
  int nLevels = ((e.getForest())->getDomain())->getNumVariables();
  for (enumerator iter(e); iter; ++iter) {
    const int* minterm = iter.getAssignments();
    fprintf(strm, "[");
    for (int i = nLevels; i > 0; i--) {
      fprintf(strm, " %d", minterm[i]);
    }
    switch ((e.getForest())->getRangeType()) {
      case forest::BOOLEAN:
        fprintf(strm, " --> T]\n");
        break;
      case forest::INTEGER:
        {
          int val = 0;
          iter.getValue(val);
          fprintf(strm, " --> %d]\n", val);
        }
        break;
      case forest::REAL:
        {
          float val = 0;
          iter.getValue(val);
          fprintf(strm, " --> %0.3f]\n", val);
        }
        break;
      default:
        fprintf(strm, "Error: invalid range_type\n");
    }
  }
}
