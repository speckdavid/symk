// $Id$

#ifndef RANDOM_REORDERING_H
#define RANDOM_REORDERING_H

#include <time.h>
#include <vector>
#include <time.h>

namespace MEDDLY{

class random_reordering : public reordering_base
{
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

    std::vector<bool> inversions(size+1, false);
    std::vector<int> levels;
    for (int i = 1; i < size; i++) {
      if (var2level[forest->getVarByLevel(i)] > var2level[forest->getVarByLevel(i + 1)]) {
        inversions[i] = true;
        levels.push_back(i);
      }
    }

    srand(time(nullptr));
    int seed = rand();
    srand(seed);

    while (!levels.empty()) {
      int index = rand() % levels.size();
      int level = levels[index];
      MEDDLY_DCASSERT(inversions[level]);
      forest->swapAdjacentVariables(level);

      inversions[level] = false;
      if (level > 1) {
        if (!inversions[level-1]
            && var2level[forest->getVarByLevel(level - 1)] > var2level[forest->getVarByLevel(level)]){
          // New inversion at lower level
          inversions[level-1] = true;
          levels.push_back(level - 1);
        }
      }
      if (level < size - 1) {
        if (!inversions[level + 1]
            && var2level[forest->getVarByLevel(level + 1)] > var2level[forest->getVarByLevel(level + 2)]) {
          // New inversion at upper level
          inversions[level+1] = true;
          levels.push_back(level + 1);
        }
      }
      levels[index] = levels.back();
      levels.pop_back();
    }

    delete[] var2level;
  }
};

}

#endif
