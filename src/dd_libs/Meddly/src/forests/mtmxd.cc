
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

#include <vector>
#include <unordered_map>

#include "mtmxd.h"
#include "../unique_table.h"

MEDDLY::mtmxd_forest
::mtmxd_forest(int dsl, domain* d, range_type t, const policies &p,int* level_reduction_rule)
 : mt_forest(dsl, d, true, t, p,level_reduction_rule)
{
  // anything to construct?
}

// ******************************************************************
// *                                                                *
// *              mtmxd_forest::mtmxd_iterator methods              *
// *                                                                *
// ******************************************************************

MEDDLY::mtmxd_forest::mtmxd_iterator::mtmxd_iterator(const expert_forest *F)
 : mt_iterator(F)
{
}

void MEDDLY::mtmxd_forest::swapAdjacentVariables(int level)
{
  if (isVarSwap()) {
    swapAdjacentVariablesByVarSwap(level);
  }
  else if (isLevelSwap()) {
    swapAdjacentVariablesByLevelSwap(level);
  }
}

void MEDDLY::mtmxd_forest::swapAdjacentVariablesByVarSwap(int level)
{
  // Swap VarHigh and VarLow
  MEDDLY_DCASSERT(level >= 1);
  MEDDLY_DCASSERT(level < getNumVariables());

  int hvar = getVarByLevel(level+1);
  int lvar = getVarByLevel(level);
  int hsize = getVariableSize(hvar);
  int lsize = getVariableSize(lvar);

  // Renumber the level of nodes for VarHigh
  int hnum = unique->getNumEntries(hvar);
  node_handle* hnodes = new node_handle[hnum];
  unique->getItems(hvar, hnodes, hnum);
  for (int i = 0; i < hnum; i++) {
    setNodeLevel(hnodes[i], level);
  }

  // Renumber the level of nodes for VarHigh'
  int phnum = unique->getNumEntries(-hvar);
  node_handle* phnodes = new node_handle[phnum];
  unique->getItems(-hvar, phnodes, phnum);
  for (int i = 0; i < phnum; i++) {
    setNodeLevel(phnodes[i], -level);
    // Protect nodes from being destroyed
    linkNode(phnodes[i]);
  }

  // Renumber the level of nodes for VarLow
  int lnum = unique->getNumEntries(lvar);
  node_handle* lnodes = new node_handle[lnum];
  unique->getItems(lvar, lnodes, lnum);
  for (int i = 0; i < lnum; i++) {
    setNodeLevel(lnodes[i], level+1);
  }
  delete[] lnodes;

  // Renumber the level of nodes for VarLow'
  int plnum = unique->getNumEntries(-lvar);
  node_handle* plnodes = new node_handle[plnum];
  unique->getItems(-lvar, plnodes, plnum);
  for (int i = 0; i < plnum; i++) {
    setNodeLevel(plnodes[i], -(level+1));
  }
  delete[] plnodes;

  //	printf("Before: Level %d : %d, Level %d : %d, Level %d : %d, Level %d : %d\n",
  //			level+1, num_high, -(level+1), num_phigh,
  //			level, num_low, -level, num_plow);

  // Update the variable order
  std::const_pointer_cast<variable_order>(var_order)->exchange(hvar, lvar);

  std::vector<node_handle> t;
  std::unordered_map<node_handle, node_handle> dup;

  // Reconstruct nodes for VarHigh
  for (int i = 0; i < hnum; i++) {
    node_handle node = swapAdjacentVariablesOf(hnodes[i]);
    if (hnodes[i] == node) {
      // VarLow is DONT_CHANGE in the MxD
      unlinkNode(node);
    }
    else if (getNodeInCount(node) > 1) {
      MEDDLY_DCASSERT(getNodeLevel(node) == -(level+1));

      // Duplication conflict
      dup.emplace(node, hnodes[i]);
//      std::cout << "UPDATE: " << node << " -> " << hnodes[i] << std::endl;
    }
    else {
      // Newly created node
      swapNodes(hnodes[i], node);
      unlinkNode(node);
      if (getNodeLevel(hnodes[i]) == (level+1)) {
        t.push_back(hnodes[i]);
      }
    }
  }
  delete[] hnodes;

  if (!dup.empty()) {
    for (const auto& n : t) {
      MEDDLY_DCASSERT(getNodeLevel(n) == (level+1));

      unpacked_node* nr = unpacked_node::useUnpackedNode();
      nr->initFromNode(this, n, true);
      bool update = false;
      for (int i = 0; i < hsize; i++) {
        if (dup.find(nr->d(i)) != dup.end()) {
          update = true;
          break;
        }
      }

      if (update) {
        unpacked_node* nb = unpacked_node::newFull(this, level + 1, hsize);
        for (int i = 0; i < hsize; i++) {
          auto search = dup.find(nr->d(i));
          nb->d_ref(i) = linkNode(search == dup.end() ? nr->d(i) : search->second);
        }
        node_handle node = createReducedNode(-1, nb);
        MEDDLY_DCASSERT(getNodeInCount(node) == 1 && getNodeLevel(node) == level + 1);
        swapNodes(n, node);
        unlinkNode(node);
      }

      unpacked_node::recycle(nr);
    }

    for (const auto& it : dup) {
      MEDDLY_DCASSERT(getNodeInCount(it.first) == 1);
      swapNodes(it.first, it.second);
      unlinkNode(it.first);
    }

    dup.clear();
  }

  {
    int j = 0;
    for (int i = 0; i < phnum; i++) {
      // Revoke the protection
      unlinkNode(phnodes[i]);
      if (isActiveNode(phnodes[i])) {
        phnodes[j++] = phnodes[i];
      }
    }
    phnum = j;
  }

  // Reconstruct nodes for VarHigh'
  for (int i = 0; i < phnum; i++) {
    MEDDLY_DCASSERT(isActiveNode(phnodes[i]));

    unpacked_node* nr = unpacked_node::useUnpackedNode();
    nr->initFromNode(this, phnodes[i], true);
    bool skip = true;
    for (int j = 0; j < hsize; j++){
      if (!isLevelAbove(-level, getNodeLevel(nr->d(j)))) {
        skip = false;
        break;
      }
    }
    unpacked_node::recycle(nr);

    if (skip) {
      // VarLow is DONT_CARE + DONT_CHANGE in the MxD
      continue;
    }

    node_handle node = swapAdjacentVariablesOf(phnodes[i]);
    MEDDLY_DCASSERT(phnodes[i] != node);

    if (getNodeInCount(node) > 1) {
      MEDDLY_DCASSERT(getNodeLevel(node) == -(level+1));

      // Duplication conflict
      dup.emplace(node, phnodes[i]);
//      std::cout << "UPDATE: " << node << " -> " << phnodes[i] << std::endl;
    }
    else {
      // Newly created node
      swapNodes(phnodes[i], node);
      unlinkNode(node);
      if (getNodeLevel(phnodes[i]) == (level+1)) {
        t.push_back(phnodes[i]);
      }
    }
  }
  delete[] phnodes;

  // XXX: Duplicate code
  if (!dup.empty()) {
    for (const auto& n : t) {
      MEDDLY_DCASSERT(getNodeLevel(n)==(level+1));

      unpacked_node* nr = unpacked_node::useUnpackedNode();
      nr->initFromNode(this, n, true);

      bool update = false;
      for (int i = 0; i < hsize; i++) {
        if(dup.find(nr->d(i)) != dup.end()){
          update=true;
          break;
        }
      }

      if (update) {
        unpacked_node* nb = unpacked_node::newFull(this, level + 1, hsize);
        for (int i = 0; i < hsize; i++) {
          auto search = dup.find(nr->d(i));
          nb->d_ref(i) = linkNode(search == dup.end() ? nr->d(i) : search->second);
        }
        node_handle node = createReducedNode(-1, nb);
        MEDDLY_DCASSERT(getNodeInCount(node) == 1 && getNodeLevel(node) == level + 1);
        swapNodes(n, node);
        unlinkNode(node);
      }

      unpacked_node::recycle(nr);
    }

    for (const auto& it : dup) {
      MEDDLY_DCASSERT(getNodeInCount(it.first) == 1);
      swapNodes(it.first, it.second);
      unlinkNode(it.first);
    }

    dup.clear();
  }

  //	printf("After: Level %d : %d,  Level %d : %//
  // Complete adjacent variable swap by swapping two levels 4 times
  // Work for fully-fully reduction only
  //d, Level %d : %d, Level %d : %d\n",
  //			level+1, unique->getNumEntries(var_low),
  //			-(level+1), unique->getNumEntries(-var_low),
  //			level, unique->getNumEntries(var_high),
  //			-level, unique->getNumEntries(-var_high));
  //	printf("#Node: %d\n", getCurrentNumNodes());
}

MEDDLY::node_handle MEDDLY::mtmxd_forest::swapAdjacentVariablesOf(node_handle node)
{
  int level = ABS(getNodeLevel(node));
  int hvar = getVarByLevel(level);
  int lvar = getVarByLevel(level+1);
  int hsize = getVariableSize(hvar);
  int lsize = getVariableSize(lvar);

  // Unprimed high node builder
  unpacked_node* hnb = unpacked_node::newFull(this, level + 1, lsize);
  if (isFullyReduced() || isQuasiReduced()) { 
    for (int m = 0; m < lsize; m++) {
      // Primed high node builder
      unpacked_node* phnb = unpacked_node::newFull(this, -(level + 1), lsize);
      for (int n = 0; n < lsize; n++) {
        // Unprimed low node builder
        unpacked_node* lnb = unpacked_node::newFull(this, level, hsize);
        for (int p = 0; p < hsize; p++) {
          // Primed low node builder
          unpacked_node* plnb = unpacked_node::newFull(this, -level, hsize);
          for (int q = 0; q < hsize; q++){
            node_handle node_p = (getNodeLevel(node) == level ? getDownPtr(node, p) : node);
            node_handle node_pq = (getNodeLevel(node_p) == -(level) ? getDownPtr(node_p, q) : node_p);
            node_handle node_pqm = (getNodeLevel(node_pq) == (level+1) ? getDownPtr(node_pq, m) : node_pq);
            plnb->d_ref(q) = linkNode(getNodeLevel(node_pqm) == -(level+1) ? getDownPtr(node_pqm, n) : node_pqm);
          }
          lnb->d_ref(p) = createReducedNode(p, plnb);
        }
        phnb->d_ref(n) = createReducedNode(-1, lnb);
      }
      hnb->d_ref(m) = createReducedNode(m, phnb);
    }
  }
  else if (isIdentityReduced()) { 
    for (int m = 0; m < lsize; m++) {
      // Primed high node builder
      unpacked_node* phnb = unpacked_node::newFull(this, -(level + 1), lsize);
      for (int n = 0; n < lsize; n++) {
        // Unprimed low node builder
        unpacked_node* lnb = unpacked_node::newFull(this, level, hsize);
        for (int p = 0; p < hsize; p++) {
          // Primed low node builder
          unpacked_node* plnb = unpacked_node::newFull(this, -level, hsize);
          for (int q = 0; q < hsize; q++) {
            node_handle node_p = (getNodeLevel(node) == level ? getDownPtr(node, p) : node);
            if (getNodeLevel(node_p) != -(level) && q != p) {
              plnb->d_ref(q) = linkNode(getTransparentNode());
            }
            else {
              node_handle node_pq = (getNodeLevel(node_p) == -(level) ? getDownPtr(node_p, q) : node_p);
              node_handle node_pqm = (getNodeLevel(node_pq) == (level+1) ? getDownPtr(node_pq, m) : node_pq);
              if (getNodeLevel(node_pqm) != -(level+1) && n != m) {
                plnb->d_ref(q) = linkNode(getTransparentNode());
              }
              else {
                plnb->d_ref(q) = linkNode(getNodeLevel(node_pqm) == -(level+1) ? getDownPtr(node_pqm, n) : node_pqm);
              }
            }
          }
          lnb->d_ref(p) = createReducedNode(p, plnb);
        }
        phnb->d_ref(n) = createReducedNode(-1, lnb);
      }
      hnb->d_ref(m) = createReducedNode(m, phnb);
    }
  }
  else {
    throw error(error::NOT_IMPLEMENTED);
  }

  return createReducedNode(-1, hnb);
}

//
// Complete adjacent variable swap by swapping two levels four times
// Work for fully-fully and quasi-quasi reductions only
//
void MEDDLY::mtmxd_forest::swapAdjacentVariablesByLevelSwap(int level)
{
  // Swap VarHigh and VarLow
  MEDDLY_DCASSERT(level >= 1);
  MEDDLY_DCASSERT(level < getNumVariables());

  if(!isFullyReduced() && !isQuasiReduced()){
    throw error(error::INVALID_OPERATION);
  }

  // x > x' > y > y'
  swapAdjacentLevels(level);
  // x > y > x' > y'
  swapAdjacentLevels(-(level+1));
  // y > x > x' > y'
  swapAdjacentLevels(-level);
  // y > x > y' > x'
  swapAdjacentLevels(level);
  // y > y' > x > x'
}

void MEDDLY::mtmxd_forest::swapAdjacentLevels(int level)
{
  throw error(error::NOT_IMPLEMENTED);

//  MEDDLY_DCASSERT(ABS(level) >= 1);
//  MEDDLY_DCASSERT(ABS(level) <= getNumVariables());
//
//  int hlevel = (level<0 ? -level : (-level-1));
//  int hvar = getVarByLevel(hlevel);
//  int lvar = getVarByLevel(level);
//  int hsize = getVariableSize(ABS(hvar));
//  int lsize = getVariableSize(ABS(lvar));
//
//  int hnum = unique->getNumEntries(hvar);
//  node_handle* hnodes = new node_handle[hnum];
//  unique->getItems(hvar, hnodes, hnum);
//
//  int lnum = unique->getNumEntries(lvar);
//  node_handle* lnodes = new node_handle[lnum];
//  unique->getItems(lvar, lnodes, lnum);
//
//  //	printf("Before: Level %d : %d, Level %d : %d\n",
//  //			level+1, high_node_size,
//  //			level, low_node_size);
//
//  int num = 0;
//  // Renumber the level of nodes for VarHigh
//  for (int i = 0; i < hnum; i++) {
//    unpacked_node* nr = unpacked_node::useUnpackedNode();
//    nr->initFromNode(this, hnodes[i], true);
//
//    MEDDLY_DCASSERT(nr->getLevel() == hlevel);
//    MEDDLY_DCASSERT(nr->getSize() == hsize);
//
//    for (int j = 0; j < hsize; j++) {
//      if (getNodeLevel(nr->d(j)) == level) {
//        // Remove the nodes corresponding to functions that
//        // are independent of the variable to be moved up
//        hnodes[num++] = hnodes[i];
//        break;
//      }
//    }
//    unpacked_node::recycle(nr);
//
//    setNodeLevel(hnodes[i], level);
//  }
//  hnum = num;
//
//  // Renumber the level of nodes for the variable to be moved up
//  for(int i = 0; i < lnum; i++) {
//    setNodeLevel(lnodes[i], hlevel);
//  }
//  delete[] lnodes;
//
//  // Update the variable order
//  order_var[hvar] = level;
//  order_var[lvar] = hlevel;
//  order_level[hlevel] = lvar;
//  order_level[level] = hvar;
//
////  std::const_pointer_cast<variable_order>(var_order)->exchange(hvar, lvar);
//
//  // Process the rest of nodes for the variable to be moved down
//  for (int i = 0; i < hnum; i++) {
//    unpacked_node* high_nr = unpacked_node::useUnpackedNode();
//    high_nr->initFromNode(this, hnodes[i], true);
//    unpacked_node* high_nb = unpacked_node::newFull(this, hlevel, lsize);
//    for (int j = 0; j < lsize; j++) {
//      unpacked_node* low_nb = unpacked_node::newFull(this, level, hsize);
//      for (int k = 0; k < hsize; k++) {
//        node_handle node_k = high_nr->d(k);
//        node_handle node_kj = (getNodeLevel(node_k) == hlevel ? getDownPtr(node_k, j) : node_k);
//        low_nb->d_ref(k) = linkNode(node_kj);
//      }
//      high_nb->d_ref(j) = createReducedNode(-1, low_nb);
//    }
//
//    unpacked_node::recycle(high_nr);
//
//    node_handle node = createReducedNode(-1, high_nb);
//    MEDDLY_DCASSERT(getNodeInCount(node) == 1);
//    MEDDLY_DCASSERT(getNodeLevel(node) == hlevel);
//
//    swapNodes(hnodes[i], node);
//    unlinkNode(node);
//  }
//
//  delete[] hnodes;

  //	printf("After: Level %d : %d, Level %d : %d\n",
  //			level+1, unique->getNumEntries(low_var),
  //			level, unique->getNumEntries(high_var));
  //	printf("#Node: %d\n", getCurrentNumNodes());
}

void MEDDLY::mtmxd_forest::dynamicReorderVariables(int top, int bottom)
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

  for (int i = 0; i < vars.size(); i++) {
    int max = i;
    unsigned max_num = unique->getNumEntries(vars[max]) + unique->getNumEntries(-vars[max]);
    for (int j = i + 1; j < vars.size(); j++){
      unsigned num = unique->getNumEntries(vars[j]) + unique->getNumEntries(-vars[j]);
      if (num > max_num) {
        max = j;
        max_num = num;
      }
    }

    int temp = vars[max];
    vars[max] = vars[i];
    vars[i] = temp;

    sifting(vars[i], top, bottom);
  }
}

void MEDDLY::mtmxd_forest::sifting(int var, int top, int bottom)
{
  int level = getLevelByVar(var);

  MEDDLY_DCASSERT(level <= top && level >= bottom);

  int num = getCurrentNumNodes();
  if(level <= (top + bottom) / 2) {
    // Move to the bottom
    while(level > bottom) {
      swapAdjacentVariables(level - 1);
      level--;
    }

    int change = 0;
    int min_level = bottom;

    MEDDLY_DCASSERT(level == bottom);
    // Move to the top
    while(level < top) {
      int high_var = getVarByLevel(level + 1);
      size_t old_sum = unique->getNumEntries(var) + unique->getNumEntries(-var)
          + unique->getNumEntries(high_var) + unique->getNumEntries(-high_var);
      swapAdjacentVariables(level);
      size_t new_sum = unique->getNumEntries(var) + unique->getNumEntries(-var)
          + unique->getNumEntries(high_var) + unique->getNumEntries(-high_var);
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
      swapAdjacentVariables(level);
      level++;
    }

    int change = 0;
    int min_level = top;
    int min = change;

    MEDDLY_DCASSERT(level == top);
    // Move to the bottom
    while(level > bottom) {
      int low_var = getVarByLevel(level - 1);
      size_t old_sum = unique->getNumEntries(var) + unique->getNumEntries(-var)
          + unique->getNumEntries(low_var) + unique->getNumEntries(-low_var);
      swapAdjacentVariables(level - 1);
      size_t new_sum = unique->getNumEntries(var) + unique->getNumEntries(-var)
          + unique->getNumEntries(low_var) + unique->getNumEntries(-low_var);
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
}

void MEDDLY::mtmxd_forest::moveDownVariable(int high, int low)
{
  throw error(error::NOT_IMPLEMENTED);
}

void MEDDLY::mtmxd_forest::moveUpVariable(int low, int high)
{
  throw error(error::NOT_IMPLEMENTED);
}

MEDDLY::mtmxd_forest::mtmxd_iterator::~mtmxd_iterator()
{
}

bool MEDDLY::mtmxd_forest::mtmxd_iterator::start(const dd_edge &e)
{
  if (F != e.getForest()) {
    throw error(error::FOREST_MISMATCH);
  }
  return first(maxLevel, e.getNode());
}

bool MEDDLY::mtmxd_forest::mtmxd_iterator::next()
{
  MEDDLY_DCASSERT(F);
  MEDDLY_DCASSERT(F->isForRelations());
  MEDDLY_DCASSERT(index);
  MEDDLY_DCASSERT(nzp);
  MEDDLY_DCASSERT(path);

  int k = -1;
  node_handle down = 0;
  for (;;) { 
    nzp[k]++;
    if (nzp[k] < path[k].getNNZs()) {
      index[k] = path[k].i(nzp[k]);
      down = path[k].d(nzp[k]);
      MEDDLY_DCASSERT(down);
      break;
    }
    if (k<0) {
      k = -k;
    } else {
      if (maxLevel == k) {
        level_change = k;
        return false;
      }
      k = -k-1;
    }
  }
  level_change = k;

  return first( (k>0) ? -k : -k-1, down);
}

bool MEDDLY::mtmxd_forest::mtmxd_iterator::first(int k, node_handle down)
{
  MEDDLY_DCASSERT(F);
  MEDDLY_DCASSERT(F->isForRelations());
  MEDDLY_DCASSERT(index);
  MEDDLY_DCASSERT(nzp);
  MEDDLY_DCASSERT(path);

  if (0==down) return false;

  bool isFully = F->isFullyReduced();

  for ( ; k; k = downLevel(k) ) {
    MEDDLY_DCASSERT(down);
    int kdn = F->getNodeLevel(down);
    MEDDLY_DCASSERT(!isLevelAbove(kdn, k));

    if (isLevelAbove(k, kdn)) {
      if (k>0 || isFully) {
        path[k].initRedundant(F, k, down, false);
      } else {
        path[k].initIdentity(F, k, index[-k], down, false);
      }
    } else {
      path[k].initFromNode(F, down, false);
    }
    nzp[k] = 0;
    index[k] = path[k].i(0);
    down = path[k].d(0);
  }
  // save the terminal value
  index[0] = down;
  return true;
}

// ******************************************************************
// *                                                                *
// *           mtmxd_forest::mtmxd_fixedrow_iter  methods           *
// *                                                                *
// ******************************************************************

MEDDLY::mtmxd_forest::
mtmxd_fixedrow_iter::mtmxd_fixedrow_iter(const expert_forest *F)
 : mt_iterator(F)
{
}

MEDDLY::mtmxd_forest::mtmxd_fixedrow_iter::~mtmxd_fixedrow_iter()
{
}

bool MEDDLY::mtmxd_forest::mtmxd_fixedrow_iter
::start(const dd_edge &e, const int* minterm)
{
  if (F != e.getForest()) {
    throw error(error::FOREST_MISMATCH);
  }
  for (int k=1; k<=maxLevel; k++) {
    index[k] = minterm[k];
  }
  return first(maxLevel, e.getNode());
}

bool MEDDLY::mtmxd_forest::mtmxd_fixedrow_iter::next()
{
  MEDDLY_DCASSERT(F);
  MEDDLY_DCASSERT(F->isForRelations());
  MEDDLY_DCASSERT(index);
  MEDDLY_DCASSERT(nzp);
  MEDDLY_DCASSERT(path);

  node_handle down = 0;
  // Only try to advance the column, because the row is fixed.
  for (int k=-1; k>=-maxLevel; k--) { 
    for (nzp[k]++; nzp[k] < path[k].getNNZs(); nzp[k]++) {
      index[k] = path[k].i(nzp[k]);
      down = path[k].d(nzp[k]);
      MEDDLY_DCASSERT(down);
      level_change = k;
      if (first(downLevel(k), down)) return true;
    }
  } // for

  return false;
}


bool MEDDLY::mtmxd_forest::mtmxd_fixedrow_iter::first(int k, node_handle down)
{
  MEDDLY_DCASSERT(F);
  MEDDLY_DCASSERT(F->isForRelations());
  MEDDLY_DCASSERT(index);
  MEDDLY_DCASSERT(nzp);
  MEDDLY_DCASSERT(path);

  if (0==k) {
    index[0] = down;
    return true;
  }

  // Check that this "row" node has a non-zero pointer
  // for the fixed index.
  MEDDLY_DCASSERT(k>0);
  int cdown;
  if (isLevelAbove(k, F->getNodeLevel(down))) {
    // skipped unprimed level, must be "fully" reduced
    cdown = down;
  } else {
    cdown = F->getDownPtr(down, index[k]);
  }
  if (0==cdown) return false;

  //
  // Ok, set up the "column" node below
  k = downLevel(k);
  MEDDLY_DCASSERT(k<0);

  if (isLevelAbove(k, F->getNodeLevel(cdown))) {
    // Skipped level, we can be fast about this.
    // first, recurse.
    if (!first(downLevel(k), cdown)) return false;
    // Ok, there is a valid path.
    // Set up this level.
    nzp[k] = 0;
    if (F->isFullyReduced()) {
      path[k].initRedundant(F, k, cdown, false);
      index[k] = 0;
    } else {
      index[k] = index[upLevel(k)];
      path[k].initIdentity(F, k, index[k], cdown, false);
    }
    return true;
  } 

  // Proper node here.
  // cycle through it and recurse... 

  path[k].initFromNode(F, cdown, false);

  for (int z=0; z<path[k].getNNZs(); z++) {
    if (first(downLevel(k), path[k].d(z))) {
      nzp[k] = z;
      index[k] = path[k].i(z);
      return true;
    }
  }

  return false;
}

// ******************************************************************
// *                                                                *
// *           mtmxd_forest::mtmxd_fixedcol_iter  methods           *
// *                                                                *
// ******************************************************************

MEDDLY::mtmxd_forest::
mtmxd_fixedcol_iter::mtmxd_fixedcol_iter(const expert_forest *F)
 : mt_iterator(F)
{
}

MEDDLY::mtmxd_forest::mtmxd_fixedcol_iter::~mtmxd_fixedcol_iter()
{
}

bool MEDDLY::mtmxd_forest::mtmxd_fixedcol_iter
::start(const dd_edge &e, const int* minterm)
{
  if (F != e.getForest()) {
    throw error(error::FOREST_MISMATCH);
  }
  
  for (int k=1; k<=maxLevel; k++) {
    index[-k] = minterm[k];
  }

  return first(maxLevel, e.getNode());
}

bool MEDDLY::mtmxd_forest::mtmxd_fixedcol_iter::next()
{
  MEDDLY_DCASSERT(F);
  MEDDLY_DCASSERT(F->isForRelations());
  MEDDLY_DCASSERT(index);
  MEDDLY_DCASSERT(nzp);
  MEDDLY_DCASSERT(path);

  node_handle down = 0;
  // Only try to advance the row, because the column is fixed.
  for (int k=1; k<=maxLevel; k++) { 
    for (nzp[k]++; nzp[k] < path[k].getNNZs(); nzp[k]++) {
      index[k] = path[k].i(nzp[k]);
      down = path[k].d(nzp[k]);
      MEDDLY_DCASSERT(down);
      level_change = k;
      if (first(downLevel(k), down)) return true;
    }
  } // for

  return false;
}


bool MEDDLY::mtmxd_forest::mtmxd_fixedcol_iter::first(int k, node_handle down)
{
  MEDDLY_DCASSERT(F);
  MEDDLY_DCASSERT(F->isForRelations());
  MEDDLY_DCASSERT(index);
  MEDDLY_DCASSERT(nzp);
  MEDDLY_DCASSERT(path);

  if (0==k) {
    index[0] = down;
    return true;
  }

  if (k<0) {
    // See if this "column node" has a path
    // at the specified index.
    if (isLevelAbove(k, F->getNodeLevel(down))) {
      if (!F->isFullyReduced()) {
        // Identity node here - check index
        if (index[k] != index[upLevel(k)]) return false;
      }
      return first(downLevel(k), down);
    }
    int cdown = F->getDownPtr(down, index[k]);
    if (0==cdown) return false;
    return first(downLevel(k), cdown);
  }

  // Row node.  Find an index, if any,
  // such that there is a valid path below.
  MEDDLY_DCASSERT(k>0);
  int kdn = F->getNodeLevel(down);
  if (isLevelAbove(k, kdn)) {
    // Skipped level, handle quickly
    int kpr = downLevel(k);
    if (isLevelAbove(kpr, F->getNodeLevel(kdn))) {
      // next level is also skipped.
      // See if there is a valid path below.
      if (!first(downLevel(kpr), down)) return false;
      // There's one below, set up the one at these levels.
      path[k].initRedundant(F, k, down, false);
      if (F->isFullyReduced()) {
        nzp[k] = 0;
        index[k] = 0;
      } else {
        nzp[k] = index[kpr];
        index[k] = index[kpr];
      }
      return true;
    }
    // next level is not skipped.
    // See if there is a valid path below.
    int cdown = F->getDownPtr(down, index[kpr]);
    if (0==cdown) return false;
    if (!first(kpr, cdown)) return false;
    path[k].initRedundant(F, k, down, false);
    nzp[k] = 0;
    index[k] = 0;
    return true;
  }

  // Level is not skipped.
  path[k].initFromNode(F, down, false);
  
  for (int z=0; z<path[k].getNNZs(); z++) {
    index[k] = path[k].i(z);
    if (first(downLevel(k), path[k].d(z))) {
      nzp[k] = z;
      return true;
    }
  }
  return false;
}

