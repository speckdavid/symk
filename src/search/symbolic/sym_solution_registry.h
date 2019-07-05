#ifndef SYMBOLIC_SYM_SOLUTION_REGISTRY_H
#define SYMBOLIC_SYM_SOLUTION_REGISTRY_H

#include "../plan_manager.h"
#include "../state_registry.h"
#include "../task_proxy.h"
#include "sym_solution_cut.h"
#include "sym_variables.h"

namespace symbolic {
    class PlanReconstructor;
    class UnidirectionalSearch;

    class SymSolutionRegistry {
    protected:
        std::shared_ptr<PlanReconstructor> plan_reconstructor;
        int num_target_plans;
        int num_found_plans;
        std::vector<SymSolutionCut> sym_cuts; // always sorted in ascending order!!!

        TaskProxy relevant_task;
        std::shared_ptr<StateRegistry> state_registry;
        std::shared_ptr<SymVariables> sym_vars;
        BDD states_on_goal_paths;
        int plan_cost_bound;

        int missing_plans() const {
            return num_target_plans - num_found_plans;
        }

    public:
        SymSolutionRegistry(int target_num_plans);

        void init(std::shared_ptr<SymVariables> sym_vars, UnidirectionalSearch* fwd_search, UnidirectionalSearch* bwd_search);

        void register_solution(const SymSolutionCut &solution);
        void construct_cheaper_solutions(int bound);

        bool found_all_plans() const {
            return missing_plans() <= 0;
        }

        int get_num_found_plans() const {
            return num_found_plans;
        }

        BDD get_states_on_goal_paths() const {
            return states_on_goal_paths;
        }
    };
} // namespace symbolic

#endif
