#ifndef SYMBOLIC_ITERATIVE_COST_SELECTOR_H
#define SYMBOLIC_ITERATIVE_COST_SELECTOR_H

#include "plan_database.h"
#include "../../option_parser.h"
#include "../../task_utils/task_properties.h"

using namespace std;

namespace symbolic {
class IterativeCostSelector : public PlanDataBase {
public:
    IterativeCostSelector(const options::Options &opts);
    ~IterativeCostSelector() {}

    virtual void init(std::shared_ptr<SymVariables> sym_vars,
                      const std::shared_ptr<AbstractTask> &task,
                      PlanManager &plan_manager);

    virtual bool reconstruct_solutions(const SymSolutionCut &cut) const override;
    virtual void add_plan(const Plan &plan) override;

    std::string tag() const override {return "Iterative cost selector";}

protected:
    int most_expensive_plan_cost;
};
}

#endif
