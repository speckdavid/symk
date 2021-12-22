#ifndef SYMBOLIC_NAIVE_SIMPLE_SELECTOR_H
#define SYMBOLIC_NAIVE_SIMPLE_SELECTOR_H

#include "plan_database.h"
#include "../../option_parser.h"
#include "../../task_utils/task_properties.h"

using namespace std;

/**
 * restricts its solution set to simple plans (without a loop).
 */

namespace symbolic {
class SimpleSelector : public PlanDataBase {
public:
    SimpleSelector(const options::Options &opts);
    ~SimpleSelector() {}

    void add_plan(const Plan &plan) override;
    std::string tag() const override {return "Simple";}

private:
    bool is_simple(const Plan &plan);
};
}

#endif
