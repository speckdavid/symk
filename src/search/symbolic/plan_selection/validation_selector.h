#ifndef SYMBOLIC_VALIDATION_SELECTOR_H
#define SYMBOLIC_VALIDATION_SELECTOR_H

#include "plan_database.h"

namespace symbolic {
class ValidationSelector : public PlanDataBase {
    // Original State Space
    const TaskProxy original_task_proxy;
    std::shared_ptr<StateRegistry> original_state_registry;

    bool is_valid_plan(const Plan &plan);

public:
    ValidationSelector(const options::Options &opts);

    ~ValidationSelector() {}

    void add_plan(const Plan &plan) override;

    std::string tag() const override {return "Validation";}
};
} // namespace symbolic

#endif
