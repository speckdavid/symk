#ifndef SYMBOLIC_SEARCH_ALGORITHMS_TOP_K_SYMBOLIC_UNIFORM_COST_SEARCH_H
#define SYMBOLIC_SEARCH_ALGORITHMS_TOP_K_SYMBOLIC_UNIFORM_COST_SEARCH_H

#include "symbolic_uniform_cost_search.h"

namespace symbolic {
class TopkSymbolicUniformCostSearch : public SymbolicUniformCostSearch {
protected:
    virtual void initialize() override;

    virtual SearchStatus step() override {
        return SymbolicUniformCostSearch::step();
    }

public:
    TopkSymbolicUniformCostSearch(const plugins::Options &opts, bool fw, bool bw, bool alternating = false);
    virtual ~TopkSymbolicUniformCostSearch() = default;

    virtual void new_solution(const SymSolutionCut &sol) override;
};
}

#endif
