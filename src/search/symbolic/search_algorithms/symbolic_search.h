#ifndef SYMBOLIC_SEARCH_ALGORITHMS_SYMBOLIC_SEARCH_H
#define SYMBOLIC_SEARCH_ALGORITHMS_SYMBOLIC_SEARCH_H

#include "../sym_enums.h"
#include "../sym_parameters.h"
#include "../sym_state_space_manager.h"

#include "../../plugins/plugin.h"
#include "../../search_algorithm.h"
#include "../plan_reconstruction/sym_solution_registry.h"

#include <memory>
#include <vector>

namespace options {
class Options;
}

namespace symbolic {
class SymStateSpaceManager;
class SymSearch;
class PlanSelector;
class SymVariables;

class SymbolicSearch : public SearchAlgorithm {
private:
    SymbolicSearch(
        const std::shared_ptr<AbstractTask> &task, const plugins::Options &opts,
        const std::tuple<
            OperatorCost, int, double, std::string, utils::Verbosity> &args);

protected:
    std::shared_ptr<AbstractTask> search_task;

    // Symbolic manager to perform bdd operations
    std::shared_ptr<SymStateSpaceManager> mgr;

    std::unique_ptr<SymSearch> search;

    std::shared_ptr<SymVariables> vars; // The symbolic variables are declared

    SymParameters sym_params; // Parameters for symbolic search

    int step_num;
    bool lower_bound_increased;
    int lower_bound; // Lower bound of search (incl. min-action costs)
    int upper_bound; // Upper bound of search (not use by top_k)
    int min_g; // min g costs of open lists

    std::shared_ptr<PlanSelector> plan_data_base;
    std::shared_ptr<SymSolutionRegistry> solution_registry; // Solution registry
    bool simple;

    bool silent;

    virtual void initialize() override;

    virtual SearchStatus step() override;

public:
    SymbolicSearch(
        const std::shared_ptr<AbstractTask> &task,
        const plugins::Options &opts);
    virtual ~SymbolicSearch() = default;

    virtual void setLowerBound(int lower);

    virtual void setMinG(int g) {
        min_g = std::max(g, min_g);
    }

    virtual bool solved() const {
        return lower_bound >= upper_bound;
    }

    virtual int getLowerBound() const {
        return lower_bound;
    }

    virtual int getMinG() const {
        return min_g;
    }

    virtual BDD get_states_on_goal_paths() const {
        return solution_registry->get_states_on_goal_paths();
    }

    virtual void new_solution(const SymSolutionCut &sol);

    virtual void print_statistics() const override;

    virtual bool is_complete_within_bound() const override {
        return true;
    }

    virtual void save_plan_if_necessary() override;

    static void add_options_to_feature(plugins::Feature &feature);
};

/*
  Task-independent wrapper that keeps the parsed options and constructs
  the symbolic search once a task is bound.
*/
template<typename T>
class TaskIndependentSymbolicSearch : public TaskIndependentSearchAlgorithm {
    plugins::Options opts;
    bool fw;
    bool bw;
    bool alternating;

    virtual std::shared_ptr<SearchAlgorithm> create_task_specific_component(
        const std::shared_ptr<AbstractTask> &task) const override {
        return std::make_shared<T>(task, opts, fw, bw, alternating);
    }

public:
    TaskIndependentSymbolicSearch(
        const plugins::Options &opts, bool fw, bool bw, bool alternating)
        : opts(opts), fw(fw), bw(bw), alternating(alternating) {
    }
};
}

#endif
