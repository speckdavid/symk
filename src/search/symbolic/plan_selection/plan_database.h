#ifndef SYMBOLIC_PLAN_DATABASE_H
#define SYMBOLIC_PLAN_DATABASE_H

#include "../../plan_manager.h"
#include "../sym_variables.h"
#include "../../plugin.h"

#include <unordered_map>
#include <memory>

class StateRegistry;

namespace options {
    class OptionParser;
    class Options;
} // namespace options

namespace symbolic {

    class PlanDataBase {
    public:
        
        static void add_options_to_parser(options::OptionParser &parser);

        PlanDataBase(const options::Options &opts);

        virtual ~PlanDataBase() {
        };
        
        virtual void init(std::shared_ptr<SymVariables> sym_vars);

        virtual void add_plan(const Plan& plan) = 0;
        
        bool has_accepted_plan(const Plan& plan) const;
        
        bool has_rejected_plan(const Plan& plan) const;

        int get_num_desired_plans() const {
            return num_desired_plans;
        }

        int get_num_accepted_plans() const {
            return num_accepted_plans;
        }

        int get_num_rejected_plans() const {
            return num_rejected_plans;
        }

        bool found_enough_plans() const {
            return num_accepted_plans >= num_desired_plans;
        }

        BDD get_states_accepted_goal_path() const {
            return states_accepted_goal_paths;
        }
        
        virtual void print_options() const;
        
        virtual std::string tag() const = 0;
        
    protected:
        void save_accepted_plan(const Plan &plan);
        void save_rejected_plan(const Plan &plan);
        
    private:
        int num_desired_plans;
        int num_accepted_plans;
        int num_rejected_plans;

        std::unordered_map<size_t, std::vector<Plan>> hashes_accepted_plans;
        std::unordered_map<size_t, std::vector<Plan>> hashes_rejected_plans;

        BDD states_accepted_goal_paths;
        std::shared_ptr<SymVariables> sym_vars;

        PlanManager plan_mgr;

        size_t different(const std::vector<Plan> &plans, const Plan &plan) const;
        BDD states_on_path(const Plan &plan);
        size_t get_hash_value(const Plan &plan) const;

    };
    
}


#endif /* SYMBOLIC_PLAN_DATABASE_H */

