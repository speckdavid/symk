#ifndef SYMBOLIC_PLAN_SELECTION_ITERATIVE_COST_SELECTOR_H
#define SYMBOLIC_PLAN_SELECTION_ITERATIVE_COST_SELECTOR_H

#include "plan_selector.h"
#include "../../task_utils/task_properties.h"

using namespace std;

namespace symbolic {
class IterativeCostSelector : public PlanSelector {
public:
    IterativeCostSelector(const plugins::Options &opts);
    ~IterativeCostSelector() {}

    virtual void init(std::shared_ptr<SymVariables> sym_vars,
                      const std::shared_ptr<AbstractTask> &task,
                      PlanManager &plan_manager);

    virtual bool reconstruct_solutions(int cost) const override;
    virtual void add_plan(const Plan &plan) override;

    std::string tag() const override {return "Iterative cost selector";}

protected:
    int most_expensive_plan_cost;
};
}

#endif
