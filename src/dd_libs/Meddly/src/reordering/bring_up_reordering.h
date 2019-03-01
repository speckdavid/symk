// $Id$

#ifndef BUBBLE_UP_REORDERING_H
#define BUBBLE_UP_REORDERING_H

namespace MEDDLY{

class bring_up_reordering : public reordering_base
{
public:
  virtual void reorderVariables(expert_forest* forest, const int * level2var)
  {
    int size = forest->getDomain()->getNumVariables();

    for (int i = size; i > 1; i--) {
      int level = forest->getLevelByVar(level2var[i]);
      while (level < i) {
        forest->swapAdjacentVariables(level);
        level++;
      }
    }
  }
};

}

#endif
