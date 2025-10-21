#ifndef SEARCH_ALGORITHMS_ASTAR_COUNT_H
#define SEARCH_ALGORITHMS_ASTAR_COUNT_H

#include "../open_list.h"
#include "../search_algorithm.h"
#include "../utils/hash.h"

#include <optional>

namespace astar_count {
class AStarCount : public SearchAlgorithm {
    size_t num_paths_to_goals;

    void update_path_count_for_state(const State &state, const State &parent,
                                     int state_g, int parent_g);

    void start_f_value_statistics(EvaluationContext &eval_context);
    void update_f_value_statistics(EvaluationContext &eval_context);

    void expand(const SearchNode &node);
protected:
    std::unique_ptr<StateOpenList> open_list;
    std::shared_ptr<Evaluator> f_evaluator;

    utils::HashMap<std::pair<int, int>, size_t> state_cost_to_path_count;
    // If we also want to store the number of actions in use, we can think of an ADD
    // or EVMDD for each action which share information over states

    virtual void initialize() override;
    virtual SearchStatus step() override;

    std::optional<SearchNode> pop_min_node();

public:
    explicit AStarCount(const std::shared_ptr<Evaluator> &h_eval,
                        OperatorCost cost_type, int bound, double max_time,
                        const std::string &description, utils::Verbosity verbosity);

    virtual void print_statistics() const override;
};
}

#endif
