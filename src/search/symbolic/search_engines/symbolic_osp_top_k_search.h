#ifndef SYMBOLIC_SEARCH_ENGINES_SYMBOLIC_OSP_TOP_K_SEARCH_H
#define SYMBOLIC_SEARCH_ENGINES_SYMBOLIC_OSP_TOP_K_SEARCH_H

#include "symbolic_osp_search.h"

#include <map>

namespace symbolic {
class SymbolicOspTopkSearch : public SymbolicOspSearch {
protected:
    virtual void initialize() override;
    virtual SearchStatus step() override;
    virtual std::vector<SymSolutionCut> get_all_util_solutions(const SymSolutionCut &sol);

public:
    SymbolicOspTopkSearch(const options::Options &opts);
    virtual ~SymbolicOspTopkSearch() = default;

    virtual void new_solution(const SymSolutionCut &sol) override;
};
}

#endif
