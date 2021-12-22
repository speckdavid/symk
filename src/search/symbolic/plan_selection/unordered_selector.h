#ifndef SYMBOLIC_UNORDERED_SELECTOR_H
#define SYMBOLIC_UNORDERED_SELECTOR_H

#include "plan_database.h"

namespace symbolic {
class UnorderedSelector : public PlanDataBase {
public:
    UnorderedSelector(const options::Options &opts);

    ~UnorderedSelector() {}

    void add_plan(const Plan &plan) override;

    std::string tag() const override {return "Unordered";}

protected:
    void save_accepted_plan(const Plan &ordered_plan, const Plan &unordered_plan);
};
}

#endif
