#ifndef SYMBOLIC_TOP_K_SELECTOR_H
#define SYMBOLIC_TOP_K_SELECTOR_H

#include "plan_database.h"

namespace symbolic {
class TopKSelector : public PlanDataBase {
public:
    TopKSelector(const options::Options &opts);

    ~TopKSelector() {}

    void add_plan(const Plan &plan) override;

    std::string tag() const override {return "Top-K";}
};
}

#endif
