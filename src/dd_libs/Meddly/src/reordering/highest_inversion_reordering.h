// $Id$

#ifndef HIGH_INVERSION_REORDERING_H
#define HIGH_INVERSION_REORDERING_H

namespace MEDDLY{

class highest_inversion_reordering : public reordering_base
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

    // The variables above ordered_level are ordered
    int ordered_level = size - 1;
    int level = size - 1;
    while (ordered_level > 0) {
      level = ordered_level;
      while (level < size && (var2level[forest->getVarByLevel(level)] > var2level[forest->getVarByLevel(level + 1)])) {
        forest->swapAdjacentVariables(level);
        level++;
      }
      ordered_level--;
    }

    delete[] var2level;
  }
};

}

#endif
