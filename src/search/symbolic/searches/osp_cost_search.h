#ifndef SYMBOLIC_SEARCHES_OSP_COST_SEARCH_H
#define SYMBOLIC_SEARCHES_OSP_COST_SEARCH_H

#include "uniform_cost_search.h"

namespace symbolic {
class OspCostSearch : public UniformCostSearch {
protected:

    virtual void filterFrontier() override;

public:
    OspCostSearch(SymbolicSearch *eng, const SymParamsSearch &params)
        : UniformCostSearch(eng, params) {}
};
}

#endif
