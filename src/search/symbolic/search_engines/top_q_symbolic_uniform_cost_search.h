#ifndef SYMBOLIC_SEARCH_ENGINES_TOP_Q_SYMBOLIC_UNIFORM_COST_SEARCH_H
#define SYMBOLIC_SEARCH_ENGINES_TOP_Q_SYMBOLIC_UNIFORM_COST_SEARCH_H

#include "top_k_symbolic_uniform_cost_search.h"

namespace symbolic {
class TopqSymbolicUniformCostSearch : public TopkSymbolicUniformCostSearch {
private:
    double get_quality_bound() {
        return solution_registry->cheapest_solution_cost_found() *
               quality_multiplier;
    }

protected:
    // Let c be the cost of the cheapest plan
    // We want all plans with quality_multiplier * c
    double quality_multiplier;

    virtual SearchStatus step() override;

public:
    TopqSymbolicUniformCostSearch(const options::Options &opts, bool fw, bool bw);
    virtual ~TopqSymbolicUniformCostSearch() = default;

    virtual void new_solution(const SymSolutionCut &sol) override;

    static void add_options_to_parser(OptionParser &parser);
};
}

#endif
