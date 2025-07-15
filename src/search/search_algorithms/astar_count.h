#ifndef SEARCH_ALGORITHMS_ASTAR_COUNT_H
#define SEARCH_ALGORITHMS_ASTAR_COUNT_H

#include "eager_search.h"

namespace astar_count {
class AStarCount : public eager_search::EagerSearch {
public:
    explicit AStarCount(const std::shared_ptr<OpenListFactory> &open,
                        bool reopen_closed, const std::shared_ptr<Evaluator> &f_eval,
                        const std::vector<std::shared_ptr<Evaluator>> &preferred,
                        const std::shared_ptr<PruningMethod> &pruning,
                        const std::shared_ptr<Evaluator> &lazy_evaluator,
                        OperatorCost cost_type, int bound, double max_time,
                        const std::string &description, utils::Verbosity verbosity);
};
}

#endif
