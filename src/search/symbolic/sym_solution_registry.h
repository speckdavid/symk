#ifndef SYMBOLIC_SYM_SOLUTION_REGISTRY_H
#define SYMBOLIC_SYM_SOLUTION_REGISTRY_H

#include "sym_variables.h"
#include "sym_solution.h"
#include "../plan_manager.h"
#include "../task_proxy.h"
#include "../state_registry.h"


namespace symbolic {
    class PlanReconstructor;

    class SymCut {
        protected:
        int g;
        int h;
        Bdd cut;

        public:
        SymCut(int g, int h, Bdd cut);

        int get_g() const;
        int get_h() const;
        int get_f() const;
        Bdd get_cut() const;
        void merge(const SymCut &other);

        void set_g(int g);
        void set_h(int h);
        void set_cut(Bdd cut);
        
        // Here we only compare g and h values!!!
        bool operator<(const SymCut &other) const;
        bool operator>(const SymCut &other) const;
        bool operator==(const SymCut &other) const;
        bool operator!=(const SymCut &other) const;

        friend std::ostream & operator<<(std::ostream &os, const SymCut& sym_cut) 
        {
            return os << "symcut{g=" << sym_cut.get_g() << ", h=" << sym_cut.get_h() << ", f=" << sym_cut.get_f() << ", nodes=" << sym_cut.get_cut().nodeCount() << "}";
        }
    };

    class SymSolutionRegistry {
        protected:
        std::shared_ptr<PlanReconstructor> plan_reconstructor;
        int target_num_plans;
        std::vector<Plan> found_plans;
        std::vector<SymCut> sym_cuts; // always sorted in ascending order!!!

        PlanManager plan_mgr;
        TaskProxy relevant_task;
        std::shared_ptr<StateRegistry> state_registry;
        std::shared_ptr<SymVariables> sym_vars;
        Bdd states_on_goal_paths;

        int missing_plans() const {
            return target_num_plans - found_plans.size();
        }

        Bdd states_on_path(const Plan& plan);

        public:
        SymSolutionRegistry(int target_num_plans);

        void init(std::shared_ptr<SymVariables> sym_vars) {
            this->sym_vars = sym_vars;
            state_registry = std::make_shared<StateRegistry>(relevant_task),
            states_on_goal_paths = sym_vars->zeroBDD();
        }

        void register_solution(const SymSolution& solution);
        void construct_cheaper_solutions(int bound);

        bool found_all_plans() const {
            return missing_plans() <= 0;
        }

        size_t num_found_plans() const {
            return found_plans.size();
        }

        Bdd get_states_on_goal_paths() const {
            return states_on_goal_paths;
        }
    };
}

#endif