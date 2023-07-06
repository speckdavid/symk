#ifndef SYMBOLIC_SEARCH_ENGINES_SYMBOLIC_OSP_SEARCH_H
#define SYMBOLIC_SEARCH_ENGINES_SYMBOLIC_OSP_SEARCH_H

#include "symbolic_search.h"

#include <map>

namespace symbolic {
class SymbolicOspSearch : public SymbolicSearch {
protected:
    std::map<int, BDD> utility_function;
    int max_utility;
    int highest_seen_utility;

    virtual void initialize() override;
    virtual void initialize_utlility();

    ADD create_utility_function() const;

    SymSolutionCut get_highest_util_solution(const SymSolutionCut &sol) const;

    virtual SearchStatus step() override;

public:
    SymbolicOspSearch(const options::Options &opts);
    virtual ~SymbolicOspSearch() = default;

    virtual void new_solution(const SymSolutionCut &sol) override;
};
}

#endif
