#ifndef SEARCH_ALGORITHMS_ASTAR_COUNT_H
#define SEARCH_ALGORITHMS_ASTAR_COUNT_H

#include "../open_list.h"
#include "../search_algorithm.h"

#include "../utils/hash.h"
#include "../utils/int_inf.h"

#include <optional>
#include <unordered_set>

#ifdef HAS_GMP
#include <gmpxx.h>
using Count = mpz_class;
#else
using Count = utils::IntInf;
#endif

namespace astar_count {

class AStarCount : public SearchAlgorithm {
    void update_path_count_for_state(
        const State &state, const State &parent, int state_g, int parent_g,
        const OperatorProxy &op);

    void start_f_value_statistics(EvaluationContext &eval_context);
    void update_f_value_statistics(EvaluationContext &eval_context);
    void update_bound(int optimal_plan_cost);

    void expand(const SearchNode &node);
protected:
    std::unique_ptr<StateOpenList> open_list;
    std::shared_ptr<Evaluator> f_evaluator;

    double quality_multiplier;
    bool prefix_optimal;
    std::unordered_set<int> projected_operators;
    Count num_goal_paths;
    int max_plan_cost;

    utils::HashMap<int, utils::HashMap<int, Count>> state_cost_to_path_count;

    virtual void initialize() override;
    virtual SearchStatus step() override;

    std::optional<SearchNode> pop_min_node();


public:
    explicit AStarCount(
        const std::shared_ptr<Evaluator> &h_eval, OperatorCost cost_type,
        int bound, double max_time, const std::string &description,
        utils::Verbosity verbosity, double quality_multiplier,
        bool prefix_optimal, std::unordered_set<int> projected_operators);

    virtual void print_statistics() const override;
};
}

#endif
