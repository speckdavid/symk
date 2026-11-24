#ifndef SYMBOLIC_VARIABLE_ORDERINGS_VARIABLE_ORDERING_H
#define SYMBOLIC_VARIABLE_ORDERINGS_VARIABLE_ORDERING_H

#include "../../task_proxy.h"

#include <string>
#include <vector>

namespace plugins {
class Feature;
class Options;
}

namespace symbolic {
class VariableOrdering {
protected:
    bool reversed;

public:
    explicit VariableOrdering(const plugins::Options &opts);
    virtual ~VariableOrdering() = default;

    virtual void compute_ordering(
        const TaskProxy &task_proxy, std::vector<int> &ordering) = 0;
    virtual std::string tag() const = 0;

    static void add_options_to_feature(plugins::Feature &feature);
};
}

#endif