#ifndef SYMBOLIC_SEARCH_ENGINES_SYMBOLIC_OSP_TOP_K_SEARCH_H
#define SYMBOLIC_SEARCH_ENGINES_SYMBOLIC_OSP_TOP_K_SEARCH_H

#include "symbolic_osp_search.h"

#include <map>

namespace symbolic {
class SymbolicOspTopkSearch : public SymbolicOspSearch {
private:
    int get_quality_bound() {
        if (quality_multiplier == std::numeric_limits<double>::infinity())
            return -std::numeric_limits<int>::max();
        // negative utility
        if (highest_seen_utility < 0)
            return highest_seen_utility * quality_multiplier;
        return ceil(highest_seen_utility / quality_multiplier);
    }

protected:
    virtual void initialize() override;
    virtual SearchStatus step() override;
    virtual std::vector<SymSolutionCut> get_all_util_solutions(const SymSolutionCut &sol);

    // Let u be the cost of the utility of the best found plan
    // We want all plans with utility scaled by quality_multiplier
    // see get_quality_bound()
    double quality_multiplier;

public:
    SymbolicOspTopkSearch(const options::Options &opts);
    virtual ~SymbolicOspTopkSearch() = default;

    virtual void new_solution(const SymSolutionCut &sol) override;
};
}

#endif
