
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

#include <cstdlib>
#include <vector>
#include <algorithm>

#include "mtmdd.h"
#include "../unique_table.h"

MEDDLY::mtmdd_forest
::mtmdd_forest(int dsl, domain* d, range_type t, const policies &p,int* level_reduction_rule)
 : mt_forest(dsl, d, false, t, p,level_reduction_rule)
{
  // anything to construct?
}

void MEDDLY::mtmdd_forest::swapAdjacentVariables(int level)
{
  MEDDLY_DCASSERT(level >= 1);
  MEDDLY_DCASSERT(level < getNumVariables());

  MEDDLY_DCASSERT(level >= 1);
  MEDDLY_DCASSERT(level < getNumVariables());

  int hvar = getVarByLevel(level + 1);  // The variable at the higher level
  int lvar = getVarByLevel(level);    // The variable at the lower level
  int hsize = getVariableSize(hvar);  // The size of the variable at the higher level
  int lsize = getVariableSize(lvar);  // The size of the variable at the lower level

  int hnum = unique->getNumEntries(hvar); // The number of nodes associated with the variable at the higher level
  node_handle* hnodes = new node_handle[hnum];
  unique->getItems(hvar, hnodes, hnum);

  int lnum = unique->getNumEntries(lvar); // The number of nodes associated with the variable at the lower level
  node_handle* lnodes = new node_handle[lnum];
  unique->getItems(lvar, lnodes, lnum);

  //	printf("Before: Level %d : %d, Level %d : %d\n",
  //			level+1, hnum,
  //			level, lnum);

  int num = 0;
  // Renumber the level of nodes for the variable to be moved down
  for (int i = 0; i < hnum; i++) {
    unpacked_node* nr = unpacked_node::useUnpackedNode();
    nr->initFromNode(this, hnodes[i], true);

    MEDDLY_DCASSERT(nr->getLevel() == level + 1);
    MEDDLY_DCASSERT(nr->getSize() == hsize);

    for (int j = 0; j < hsize; j++) {
      if (isLevelAbove(getNodeLevel(nr->d(j)), level - 1)) {
        // Remove the nodes corresponding to functions that
        // are independent of the variable to be moved up
        hnodes[num++] = hnodes[i];
        break;
      }
    }
    unpacked_node::recycle(nr);

    setNodeLevel(hnodes[i], level);
  }
  hnum = num;

  // Renumber the level of nodes for the variable to be moved up
  for (int i = 0; i < lnum; i++) {
    setNodeLevel(lnodes[i], level + 1);
  }

  // Update the variable order
  std::const_pointer_cast<variable_order>(var_order)->exchange(hvar, lvar);

  node_handle** children = new node_handle*[hsize];
  for (int i = 0; i < hsize; i++) {
    children[i] = new node_handle[lsize];
  }

  // Process the rest of nodes for the variable to be moved down
  for (int i = 0; i < hnum; i++) {
    MEDDLY_DCASSERT(isActiveNode(hnodes[i]));

    unpacked_node* high_nr = unpacked_node::useUnpackedNode();
    high_nr->initFromNode(this, hnodes[i], true);

    unpacked_node* high_nb = unpacked_node::newFull(this, level + 1, lsize);
    for (int j = 0; j < hsize; j++) {
      if (isLevelAbove(level, getNodeLevel(high_nr->d(j)))) {
        for (int k = 0; k < lsize; k++) {
          children[j][k] = high_nr->d(j);
        }
      }
      else {
        unpacked_node* nr = unpacked_node::useUnpackedNode();
        nr->initFromNode(this, high_nr->d(j), true);

        MEDDLY_DCASSERT(nr->getSize() == lsize);
        for (int k = 0; k < lsize; k++) {
          children[j][k] = nr->d(k);
        }
        unpacked_node::recycle(nr);
      }
    }

    for (int j = 0; j < lsize; j++) {
      unpacked_node* low_nb = unpacked_node::newFull(this, level, hsize);
      for (int k = 0; k < hsize; k++) {
        low_nb->d_ref(k) = linkNode(children[k][j]);
      }
      high_nb->d_ref(j) = createReducedNode(-1, low_nb);
    }

    unpacked_node::recycle(high_nr);

    // The reduced node of high_nb must be at level+1
    // Assume the reduced node is at level
    // Then high_nodes[i] corresponds to a function that
    // is independent of the variable to be moved up
    // This is a contradiction
    modifyReducedNodeInPlace(high_nb, hnodes[i]);
  }

  for (int i = 0; i < hsize; i++) {
    delete[] children[i];
  }
  delete[] children;

  delete[] hnodes;
  delete[] lnodes;

  //	printf("After: Level %d : %d, Level %d : %d\n",
  //			level+1, unique->getNumEntries(lvar),
  //			level, unique->getNumEntries(hvar));
  //	printf("#Node: %d\n", getCurrentNumNodes());
}

void MEDDLY::mtmdd_forest::moveDownVariable(int high, int low)
{
  MEDDLY_DCASSERT(low<high);
  MEDDLY_DCASSERT(low>=1);
  MEDDLY_DCASSERT(high<=getNumVariables());

  removeAllComputeTableEntries();

  for(int level=high-1; level>=low; level--) {
    swapAdjacentVariables(level);
  }
}

void MEDDLY::mtmdd_forest::moveUpVariable(int low, int high)
{
  MEDDLY_DCASSERT(low<high);
  MEDDLY_DCASSERT(low>=1);
  MEDDLY_DCASSERT(high<=getNumVariables());

  removeAllComputeTableEntries();

  for(int level=low; level<high; level++) {
    swapAdjacentVariables(level);
  }
}

void MEDDLY::mtmdd_forest::dynamicReorderVariables(int top, int bottom)
{
  MEDDLY_DCASSERT(top > bottom);
  MEDDLY_DCASSERT(top <= getNumVariables());
  MEDDLY_DCASSERT(bottom >= 1);

  removeAllComputeTableEntries();

  std::vector<int> vars;
  vars.reserve(top - bottom + 1);
  for (int i = bottom; i <= top; i++) {
    vars.push_back(getVarByLevel(i));
  }

  for (unsigned int i = 0; i < vars.size(); i++) {
    int max = i;
    unsigned max_num = unique->getNumEntries(vars[max]);
    for (unsigned int j = i + 1; j < vars.size(); j++){
      if (unique->getNumEntries(vars[j]) > max_num) {
        max = j;
        max_num = unique->getNumEntries(vars[j]);
      }
    }

    int temp = vars[max];
    vars[max] = vars[i];
    vars[i] = temp;

    sifting(vars[i], top, bottom);
  }
}

void MEDDLY::mtmdd_forest::sifting(int var, int top, int bottom)
{
  int level = getLevelByVar(var);

  MEDDLY_DCASSERT(level <= top && level >= bottom);

  long num = getCurrentNumNodes();
  if(level <= (top + bottom) / 2) {
    // Move to the bottom
    while(level > bottom) {
      //			int low_var = getVarByLevel(level - 1);
      //			int old_sum = unique->getNumEntries(var) + unique->getNumEntries(low_var);
      swapAdjacentVariables(level - 1);
      //			int new_sum = unique->getNumEntries(var) + unique->getNumEntries(low_var);
      //			change += (new_sum - old_sum);
      level--;
      //
      //			if(change < min) {
      //				min_level = level;
      //				min = change;
      //			}
    }

    int change = 0;
    int min_level = bottom;

    MEDDLY_DCASSERT(level == bottom);
    // Move to the top
    while(level < top) {
      int high_var = getVarByLevel(level + 1);
      size_t old_sum = unique->getNumEntries(var) + unique->getNumEntries(high_var);
      swapAdjacentVariables(level);
      size_t new_sum = unique->getNumEntries(var) + unique->getNumEntries(high_var);
      change += (new_sum - old_sum);
      level++;

      if(change <= 0) {
        min_level = level;
        change = 0;
      }
    }

    MEDDLY_DCASSERT(level == top);
    while(level > min_level) {
      swapAdjacentVariables(level - 1);
      level--;
    }
  }
  else {
    // Move to the top
    while(level < top) {
      //			int high_var = getVarByLevel(level + 1);
      //			int old_sum = unique->getNumEntries(var) + unique->getNumEntries(high_var);
      swapAdjacentVariables(level);
      //			int new_sum = unique->getNumEntries(var) + unique->getNumEntries(high_var);
      //			change += (new_sum - old_sum);
      level++;
      //
      //			if(change < min) {
      //				min_level = level;
      //				min = change;
      //			}
    }

    int change = 0;
    int min_level = top;
    int min = change;

    MEDDLY_DCASSERT(level == top);
    // Move to the bottom
    while(level > bottom) {
      int low_var = getVarByLevel(level - 1);
      size_t old_sum = unique->getNumEntries(var) + unique->getNumEntries(low_var);
      swapAdjacentVariables(level - 1);
      size_t new_sum = unique->getNumEntries(var) + unique->getNumEntries(low_var);
      change += (new_sum - old_sum);
      level--;

      if(change <= min) {
        min_level = level;
        min = change;
      }
    }

    MEDDLY_DCASSERT(level == bottom);
    MEDDLY_DCASSERT(min <= 0);
    while(level < min_level) {
      swapAdjacentVariables(level);
      level++;
    }
  }

  MEDDLY_DCASSERT(getCurrentNumNodes() <= num);
  if(getCurrentNumNodes() > num) {
    printf("Error: %ld > %ld\n", getCurrentNumNodes(), num);
  }
}

// ******************************************************************
// *                                                                *
// *              mtmdd_forest::mtmdd_iterator methods              *
// *                                                                *
// ******************************************************************

MEDDLY::mtmdd_forest::mtmdd_iterator::mtmdd_iterator(const expert_forest *F)
 : mt_iterator(F)
{
}

MEDDLY::mtmdd_forest::mtmdd_iterator::~mtmdd_iterator()
{
}

bool MEDDLY::mtmdd_forest::mtmdd_iterator::start(const dd_edge &e)
{
  if (F != e.getForest()) {
    throw error(error::FOREST_MISMATCH);
  }
  return first(maxLevel, e.getNode());
}

bool MEDDLY::mtmdd_forest::mtmdd_iterator::next()
{
  MEDDLY_DCASSERT(F);
  MEDDLY_DCASSERT(!F->isForRelations());
  MEDDLY_DCASSERT(index);
  MEDDLY_DCASSERT(nzp);
  MEDDLY_DCASSERT(path);

  int k;
  node_handle down = 0;
  for (k=1; k<=maxLevel; k++) {
    nzp[k]++;
    if (nzp[k] < path[k].getNNZs()) {
      index[k] = path[k].i(nzp[k]);
      down = path[k].d(nzp[k]);
      MEDDLY_DCASSERT(down);
      break;
    }
  }
  level_change = k;
  if (k>maxLevel) {
    return false;
  }

  return first(k-1, down);
}

bool MEDDLY::mtmdd_forest::mtmdd_iterator::first(int k, node_handle down)
{
  MEDDLY_DCASSERT(F);
  MEDDLY_DCASSERT(!F->isForRelations());
  MEDDLY_DCASSERT(index);
  MEDDLY_DCASSERT(nzp);
  MEDDLY_DCASSERT(path);

  if (0==down) return false;

  for ( ; k; k--) {
    MEDDLY_DCASSERT(down);
    int kdn = F->getNodeLevel(down);
    MEDDLY_DCASSERT(kdn <= k);
    if (kdn < k)  path[k].initRedundant(F, k, down, false);
    else          path[k].initFromNode(F, down, false);
    nzp[k] = 0;
    index[k] = path[k].i(0);
    down = path[k].d(0);
  }
  // save the terminal value
  index[0] = down;
  return true;
}
