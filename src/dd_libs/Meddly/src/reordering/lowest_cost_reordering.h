// $Id$

#ifndef LOWEST_COST_REORDERING_H
#define LOWEST_COST_REORDERING_H

#include "../heap.h"
#include "../unique_table.h"

namespace MEDDLY{

class lowest_cost_reordering : public reordering_base
{
protected:
  long calculate_swap_cost(expert_forest* forest, int level)
  {
    //int lvar = forest->getVarByLevel(level);
    int hvar = forest->getVarByLevel(level+1);
    return (long) get_unique_table(forest)->getNumEntries(hvar) * forest->getVariableSize(hvar);
  }

public:
  virtual void reorderVariables(expert_forest* forest, const int* level2var)
  {
    int size = forest->getDomain()->getNumVariables();

    // Transpose order
    int* var2level = new int[size];
    var2level[0] = 0;
    for (int i = 1; i <= size; i++) {
      var2level[level2var[i]] = i;
    }

    IndexedHeap<long, less<long>> heap(size);
    for (int i = 1; i < size; i++) {
      if (var2level[forest->getVarByLevel(i)] > var2level[forest->getVarByLevel(i + 1)]) {
        long weight = calculate_swap_cost(forest, i);
        heap.push(i, weight);
      }
    }

    while (!heap.empty()) {
      int level = heap.top_key();
      forest->swapAdjacentVariables(level);
      heap.pop();

      if (level < size - 1
          && var2level[forest->getVarByLevel(level + 1)] > var2level[forest->getVarByLevel(level + 2)]) {
        long weight = calculate_swap_cost(forest, level + 1);
        heap.push(level+1, weight);
      }
      if (level > 1
          && var2level[forest->getVarByLevel(level - 1)] > var2level[forest->getVarByLevel(level)]) {
        long weight = calculate_swap_cost(forest, level - 1);
        heap.push(level-1, weight);
      }
    }

    delete[] var2level;
  }
};

}

#endif
