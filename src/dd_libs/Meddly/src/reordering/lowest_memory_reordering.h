// $Id$

#ifndef LOWEST_MEMORY_REORDERING_H
#define LOWEST_MEMORY_REORDERING_H

#include "../heap.h"
#include "../unique_table.h"

namespace MEDDLY{

class lowest_memory_reordering : public reordering_base
{
protected:
  long calculate_swap_memory_cost(expert_forest* forest, int level)
  {
    int lvar = forest->getVarByLevel(level);
    int hvar = forest->getVarByLevel(level+1);
    long before = get_unique_table(forest)->getNumEntries(hvar)
        + get_unique_table(forest)->getNumEntries(lvar);

    forest->swapAdjacentVariables(level);
    long after = get_unique_table(forest)->getNumEntries(hvar)
        + get_unique_table(forest)->getNumEntries(lvar);
    forest->swapAdjacentVariables(level);

    return (after-before);
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
        long cost = calculate_swap_memory_cost(forest, i);
        heap.push(i, cost);
      }
    }

    bool swapped[] = {false, false};
    int saved = 0;
    while (!heap.empty()) {
      int level = heap.top_key();
      if (swapped[0] || swapped[1]) {
        saved++;
        swapped[0] = false;
        swapped[1] = false;
      }
      else {
        forest->swapAdjacentVariables(level);
      }
      heap.pop();

      if (level < size-1
          && var2level[forest->getVarByLevel(level + 1)] > var2level[forest->getVarByLevel(level + 2)]) {
        int lvar = forest->getVarByLevel(level+1);
        int hvar = forest->getVarByLevel(level+2);
        long before = get_unique_table(forest)->getNumEntries(hvar)
            + get_unique_table(forest)->getNumEntries(lvar);
        forest->swapAdjacentVariables(level+1);
        long after = get_unique_table(forest)->getNumEntries(hvar)
            + get_unique_table(forest)->getNumEntries(lvar);

        heap.push(level+1, after-before);

        swapped[0] = true;
      }
      if (level > 1
          && var2level[forest->getVarByLevel(level - 1)] > var2level[forest->getVarByLevel(level)]) {
        int lvar = forest->getVarByLevel(level-1);
        int hvar = forest->getVarByLevel(level);
        long before = get_unique_table(forest)->getNumEntries(hvar)
            + get_unique_table(forest)->getNumEntries(lvar);
        forest->swapAdjacentVariables(level-1);
        long after = get_unique_table(forest)->getNumEntries(hvar)
            + get_unique_table(forest)->getNumEntries(lvar);

        heap.push(level-1, after-before);

        swapped[1] = true;
      }

      if (swapped[0] && heap.top_key() != level + 1) {
        // Undo
        forest->swapAdjacentVariables(level + 1);
        swapped[0] = false;
      }
      if (swapped[1] && heap.top_key()!=level - 1) {
        // Undo
        forest->swapAdjacentVariables(level-1);
        swapped[1] = false;
      }
    }

    delete[] var2level;
  }
};

}

#endif
