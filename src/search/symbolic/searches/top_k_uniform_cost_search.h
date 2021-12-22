#ifndef SYMBOLIC_SEARCHES_TOP_K_UNIFORM_COST_SEARCH_H
#define SYMBOLIC_SEARCHES_TOP_K_UNIFORM_COST_SEARCH_H

#include "uniform_cost_search.h"

namespace symbolic {
class TopkUniformCostSearch : public UniformCostSearch {
protected:
    virtual bool provable_no_more_plans() override;

    virtual void checkFrontierCut(Bucket &bucket, int g) override;

    virtual void filterFrontier() override;

public:
    TopkUniformCostSearch(SymbolicSearch *eng, const SymParamsSearch &params)
        : UniformCostSearch(eng, params) {}
};
} // namespace symbolic

#endif
