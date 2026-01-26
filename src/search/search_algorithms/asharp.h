#ifndef SEARCH_ALGORITHMS_ASHARP_H
#define SEARCH_ALGORITHMS_ASHARP_H

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

namespace asharp {

class ASharp : public SearchAlgorithm {
    void update_path_count_for_state(
        const State &state, const State &parent, bool lower_cost);

    void start_f_value_statistics(EvaluationContext &eval_context);
    void update_f_value_statistics(EvaluationContext &eval_context);
    void update_bound(int optimal_plan_cost);
    void check_cost_variable_bound() const;

    void expand(const SearchNode &node);
protected:
    std::unique_ptr<StateOpenList> open_list;
    std::shared_ptr<Evaluator> f_evaluator;

    double quality_multiplier;
    Count num_goal_paths;
    int max_plan_cost;

    utils::HashMap<int, Count> state_to_path_count;

    virtual void initialize() override;
    virtual SearchStatus step() override;

    std::optional<SearchNode> pop_min_node();


public:
    explicit ASharp(
        const std::shared_ptr<Evaluator> &h_eval, OperatorCost cost_type,
        int bound, double max_time, const std::string &description,
        utils::Verbosity verbosity, double quality_multiplier);

    virtual void print_statistics() const override;
};
}

#endif
