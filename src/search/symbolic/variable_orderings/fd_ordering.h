#ifndef SYMBOLIC_VARIABLE_ORDERINGS_FD_ORDERING_H
#define SYMBOLIC_VARIABLE_ORDERINGS_FD_ORDERING_H

#include "variable_ordering.h"

namespace symbolic {
class FDOrdering : public VariableOrdering {
public:
    explicit FDOrdering(const plugins::Options &opts);

    std::string tag() const override {
        return reversed ? "FD Ordering [Reversed]" : "FD Ordering";
    }

    void compute_ordering(
        const TaskProxy &task_proxy, std::vector<int> &ordering) override;
};
}

#endif