// $Id$

#ifndef REORDERING_BASE_H
#define REORDERING_BASE_H

#include "../meddly_expert.h"

namespace MEDDLY{

class reordering_base
{
protected:
  const unique_table* get_unique_table(expert_forest* forest) const;
  int getInCount(expert_forest* forest, node_handle p) const;

public:
  virtual void reorderVariables(expert_forest* forest, const int* level2var) = 0;
};

inline const unique_table* reordering_base::get_unique_table(expert_forest* forest) const
{
  return forest->unique;
}

inline int reordering_base::getInCount(expert_forest* forest, node_handle p) const
{
  return forest->getNodeInCount(p);
}

}

#endif
