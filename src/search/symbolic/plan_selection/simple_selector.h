#ifndef SYMBOLIC_PLAN_SELECTION_SIMPLE_SELECTOR_H
#define SYMBOLIC_PLAN_SELECTION_SIMPLE_SELECTOR_H

#include "plan_selector.h"
#include "../../task_utils/task_properties.h"

using namespace std;

/**
 * restricts its solution set to simple plans (without a loop).
 */

namespace symbolic {
class SimpleSelector : public PlanSelector {
public:
    SimpleSelector(const plugins::Options &opts);
    ~SimpleSelector() {}

    void add_plan(const Plan &plan) override;
    std::string tag() const override {return "Simple";}

private:
    bool is_simple(const Plan &plan);
};
}

#endif
