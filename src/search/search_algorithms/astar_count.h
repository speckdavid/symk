#ifndef SEARCH_ALGORITHMS_ASTAR_COUNT_H
#define SEARCH_ALGORITHMS_ASTAR_COUNT_H

#include "../open_list.h"
#include "../search_algorithm.h"
#include "../utils/hash.h"

namespace astar_count {
class AStarCount : public SearchAlgorithm {
    void update_path_count_for_state(const State &state, const State &parent,
                                     int state_g, int parent_g);

    void start_f_value_statistics(EvaluationContext &eval_context);
    void update_f_value_statistics(EvaluationContext &eval_context);

    size_t num_paths_to_goals;

protected:
    std::unique_ptr<StateOpenList> open_list;
    std::shared_ptr<Evaluator> f_evaluator;

    utils::HashMap<std::pair<int, int>, int> state_cost_to_path_count;

    virtual void initialize() override;
    virtual SearchStatus step() override;

public:
    explicit AStarCount(const std::shared_ptr<Evaluator> &h_eval,
                        OperatorCost cost_type, int bound, double max_time,
                        const std::string &description, utils::Verbosity verbosity);

    virtual void print_statistics() const override;
};
}

#endif
