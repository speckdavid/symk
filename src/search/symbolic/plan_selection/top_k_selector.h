#ifndef SYMBOLIC_TOP_K_SELECTOR_H
#define SYMBOLIC_TOP_K_SELECTOR_H

#include "plan_selector.h"

namespace symbolic {
class TopKSelector : public PlanSelector {
public:
    TopKSelector(const options::Options &opts);

    ~TopKSelector() {}

    void add_plan(const Plan &plan) override;

    std::string tag() const override {return "Top-K";}
};
}

#endif
