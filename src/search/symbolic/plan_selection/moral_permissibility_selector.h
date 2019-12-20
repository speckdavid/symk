#ifndef SYMBOLIC_MORAL_PERMISSIBILITY_SELECTOR_H
#define SYMBOLIC_MOTAL_PERMISSIBILITY_SELECTOR_H

#include "plan_database.h"

namespace symbolic {

class MoralPermissibilitySelector : public PlanDataBase {
public:
  MoralPermissibilitySelector(const options::Options &opts);

  ~MoralPermissibilitySelector(){};

  void add_plan(const Plan &plan) override;

  std::string tag() const override { return "Moral Permissibility"; }
};

} // namespace symbolic

#endif /* SYMBOLIC_MOTAL_PERMISSIBILITY_SELECTOR_H */