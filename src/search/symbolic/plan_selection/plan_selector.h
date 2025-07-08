#ifndef SYMBOLIC_PLAN_SELECTION_H
#define SYMBOLIC_PLAN_SELECTION_H

#include "../../plan_manager.h"
#include "../../plugins/plugin.h"
#include "../sym_variables.h"

#include "../plan_reconstruction/sym_solution_cut.h"

#include <memory>
#include <unordered_map>

class StateRegistry;

namespace symbolic {
class PlanSelector {
public:
    static void add_options_to_feature(plugins::Feature &feature);

    PlanSelector(const plugins::Options &opts);

    virtual ~PlanSelector() {}

    virtual void init(std::shared_ptr<SymVariables> sym_vars,
                      const std::shared_ptr<AbstractTask> &task,
                      PlanManager &plan_manager);

    virtual void add_plan(const Plan &plan) = 0;

    bool has_accepted_plan(const Plan &plan) const;

    bool has_rejected_plan(const Plan &plan) const;

    bool has_zero_cost_loop(const Plan &plan) const;

    std::pair<int, int> get_first_zero_cost_loop(const Plan &plan) const;

    int get_num_desired_plans() const {return num_desired_plans;}

    int get_num_accepted_plans() const {return num_accepted_plans;}

    int get_num_rejected_plans() const {return num_rejected_plans;}

    bool found_enough_plans() const {
        return num_accepted_plans >= num_desired_plans;
    }

    virtual bool reconstruct_solutions(int /*cost*/) const {
        return !found_enough_plans();
    }

    BDD get_states_accepted_goal_path() const {
        return anytime_completness ? states_accepted_goal_paths
               : sym_vars->oneBDD();
    }

    int get_num_reported_plan() const {
        return plan_mgr.get_num_previously_generated_plans();
    }

    BDD get_final_state(const Plan &plan) const;

    BDD states_on_path(const Plan &plan);

    const Plan &get_first_accepted_plan() const;

    double get_first_plan_cost() const {return first_accepted_plan_cost;}

    virtual void print_options() const;

    virtual std::string tag() const = 0;

protected:
    std::shared_ptr<SymVariables> sym_vars;
    std::shared_ptr<StateRegistry> state_registry;   // used for explicit stuff

    // Determines if it possible/desired to proof that no more (accepted)
    // plans exits
    // 1. If true: terminates if open contains only states which are in
    // the closed list and not on an accepted goal path 8e.g. top-k)
    // 2. If false: terminates never and keeps searching for new plans
    // Note: the algorithm still terminates if the open list is empty
    // which only occurs if no reachable loops part of in the state space
    bool anytime_completness;

    bool dump_plans;
    bool write_plans;

    int num_desired_plans;
    int num_accepted_plans;
    int num_rejected_plans;

    TaskProxy plan_mgr_task_proxy;
    PlanManager plan_mgr;

    std::unordered_map<size_t, std::vector<Plan>> hashes_accepted_plans;
    std::unordered_map<size_t, std::vector<Plan>> hashes_rejected_plans;

    Plan first_accepted_plan;
    double first_accepted_plan_cost;

    BDD states_accepted_goal_paths;


    void save_accepted_plan(const Plan &plan);
    void save_rejected_plan(const Plan &plan);

    std::vector<Plan> get_accepted_plans() const;

    size_t different(const std::vector<Plan> &plans, const Plan &plan) const;
    size_t get_hash_value(const Plan &plan) const;
};
}

#endif
