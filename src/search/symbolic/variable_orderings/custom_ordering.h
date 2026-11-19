#ifndef SYMBOLIC_VARIABLE_ORDERINGS_CUSTOM_ORDERING_H
#define SYMBOLIC_VARIABLE_ORDERINGS_CUSTOM_ORDERING_H

#include "variable_ordering.h"

namespace symbolic {
class CustomOrdering : public VariableOrdering {
    std::vector<int> ordering;

public:
    explicit CustomOrdering(const plugins::Options &opts);

    std::string tag() const override {
        return reversed ? "Custom Ordering [Reversed]" : "Custom Ordering";
    }

    void compute_ordering(
        const TaskProxy &task_proxy, std::vector<int> &ordering) override;
};
}

#endif