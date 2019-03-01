// $Id$

#ifndef SINK_DOWN_REORDERING_H
#define SINK_DOWN_REORDERING_H

namespace MEDDLY{

class sink_down_reordering : public reordering_base
{
public:
  virtual void reorderVariables(expert_forest* forest, const int * level2var)
  {
    int size = forest->getDomain()->getNumVariables();

    for (int i = 1; i < size; i++) {
      int level = forest->getLevelByVar(level2var[i]);
      while (level > i) {
        forest->swapAdjacentVariables(level - 1);
        level--;
      }
    }
  }
};

}

#endif
