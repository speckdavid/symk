#ifndef SYMBOLIC_SEARCH_ENGINES_SYMBOLIC_SEARCH_H
#define SYMBOLIC_SEARCH_ENGINES_SYMBOLIC_SEARCH_H


#include "../plan_reconstruction/sym_solution_registry.h"
#include "../search_engine.h"
#include "../sym_enums.h"
#include "../sym_params_search.h"
#include "../sym_state_space_manager.h"
#include "../../option_parser.h"

#include <memory>
#include <vector>

namespace options {
class Options;
}

namespace symbolic {
class SymStateSpaceManager;
class SymSearch;
class PlanDataBase;
class SymVariables;

class SymbolicSearch : public SearchEngine {
private:
protected:
    // Hold a reference to the task implementation and pass it to objects that
    // need it.
    const std::shared_ptr<AbstractTask> task;
    std::shared_ptr<AbstractTask> search_task;
    // Use task_proxy to access task information.
    TaskProxy task_proxy;

    // Symbolic manager to perform bdd operations
    std::shared_ptr<SymStateSpaceManager> mgr;

    std::unique_ptr<SymSearch> search;

    std::shared_ptr<SymVariables> vars; // The symbolic variables are declared

    SymParamsMgr mgrParams; // Parameters for SymStateSpaceManager configuration.
    SymParamsSearch searchParams; // Parameters to search the original state space

    int step_num;
    bool lower_bound_increased;
    int lower_bound; // Lower bound of search (incl. min-action costs)
    int upper_bound; // Upper bound of search (not use by top_k)
    int min_g;     // min g costs of open lists

    std::shared_ptr<PlanDataBase> plan_data_base;
    std::shared_ptr<SymSolutionRegistry> solution_registry; // Solution registry
    bool simple;

    virtual void initialize() override;

    virtual SearchStatus step() override;

public:
    SymbolicSearch(const options::Options &opts);
    virtual ~SymbolicSearch() = default;

    virtual void setLowerBound(int lower);

    virtual void setMinG(int g) {min_g = std::max(g, min_g);}

    virtual bool solved() const {return lower_bound >= upper_bound;}

    virtual int getLowerBound() const {return lower_bound;}

    virtual int getMinG() const {return min_g;}

    virtual BDD get_states_on_goal_paths() const {
        return solution_registry->get_states_on_goal_paths();
    }

    virtual void new_solution(const SymSolutionCut &sol);

    virtual void print_statistics() const {}

    virtual void save_plan_if_necessary() override;

    static void add_options_to_parser(OptionParser &parser);
};
}

#endif
