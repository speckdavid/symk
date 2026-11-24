#ifndef SYMBOLIC_VARIABLE_ORDERINGS_GAMER_ORDERING_H
#define SYMBOLIC_VARIABLE_ORDERINGS_GAMER_ORDERING_H

#include "variable_ordering.h"

#include "../../utils/rng.h"

namespace symbolic {
class GamerOrdering : public VariableOrdering {
    std::shared_ptr<utils::RandomNumberGenerator> rng;
    std::vector<std::vector<double>> influence_graph;
    double timeout;

    double influence(int var1, int var2) const;
    void set_influence(int var1, int var2, double value = 1);
    double optimize_ordering(std::vector<int> &ordering, int iterations) const;
    double compute_function(const std::vector<int> &ordering) const;
    std::vector<int> randomize(const std::vector<int> &ordering) const;

public:
    explicit GamerOrdering(const plugins::Options &opts);

    std::string tag() const override {
        return reversed ? "Gamer Ordering [Reversed]" : "Gamer Ordering";
    }

    void compute_ordering(
        const TaskProxy &task_proxy, std::vector<int> &ordering) override;
};
}

#endif