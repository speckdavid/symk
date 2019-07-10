#include "plan_database.h"

#include "../../option_parser.h"
#include "../../plugin.h"
#include "../../tasks/root_task.h"
#include "../../state_registry.h"


namespace symbolic {

    void PlanDataBase::add_options_to_parser(options::OptionParser &parser) {
        parser.add_option<int>("num_plans", "number of plans", "1");
    }

    PlanDataBase::PlanDataBase(const options::Options &opts) :
    num_desired_plans(opts.get<int>("num_plans")),
    num_accepted_plans(0),
    num_rejected_plans(0),
    sym_vars(nullptr) {
        plan_mgr.set_plan_filename("found_plans/sas_plan");
    }

    void PlanDataBase::init(std::shared_ptr<SymVariables> sym_vars) {
        this->sym_vars = sym_vars;
        states_accepted_goal_paths = sym_vars->zeroBDD();
    }

    bool PlanDataBase::has_accepted_plan(const Plan& plan) const {
        size_t plan_seed = get_hash_value(plan);
        if (hashes_accepted_plans.count(plan_seed) == 0) {
            return false;
        }
        if (different(hashes_accepted_plans.at(plan_seed), plan)) {
            return false;
        }
        return true;
    }

    bool PlanDataBase::has_rejected_plan(const Plan& plan) const {
        size_t plan_seed = get_hash_value(plan);
        if (hashes_rejected_plans.count(plan_seed) == 0) {
            return false;
        }
        if (different(hashes_rejected_plans.at(plan_seed), plan)) {
            return false;
        }
        return true;
    }

    void PlanDataBase::print_options() const {
        std::cout << "Plan Selector: " << tag() << std::endl;
    }

    size_t PlanDataBase::different(const std::vector<Plan> &plans, const Plan &plan) const {
        for (auto &cur : plans) {
            if (cur.size() == plan.size()) {
                bool same = true;
                for (size_t i = 0; i < cur.size(); ++i) {
                    if (cur.at(i) != plan.at(i)) {
                        same = false;
                        break;
                    }
                }
                if (same) {
                    return false;
                }
            }
        }
        return true;
    }

    BDD PlanDataBase::states_on_path(const Plan &plan) {
        GlobalState cur = sym_vars->get_state_registry()->get_initial_state();
        BDD path_states = sym_vars->getStateBDD(cur);
        for (auto &op : plan) {
            cur = sym_vars->get_state_registry()->get_successor_state(
                    cur, sym_vars->get_state_registry()->get_task_proxy().get_operators()[op]);
            path_states += sym_vars->getStateBDD(cur);
        }
        return path_states;
    }

    // Hashes a vector of ints (= a plan)
    // According to the following link this is the has function used by boost
    // for hashing vector<int>. Experience: really good function
    // https://stackoverflow.com/questions/20511347/a-good-hash-function-for-a-vector

    size_t PlanDataBase::get_hash_value(const Plan &plan) const {
        std::size_t seed = plan.size();
        for (auto &op : plan) {
            seed ^= op.get_index() + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        return seed;
    }

    void PlanDataBase::save_accepted_plan(const Plan &plan) {
        size_t plan_seed = get_hash_value(plan);
        if (hashes_accepted_plans.count(plan_seed) == 0) {
            hashes_accepted_plans[plan_seed] = std::vector<Plan>();
        }
        hashes_accepted_plans[plan_seed].push_back(plan);
        states_accepted_goal_paths += states_on_path(plan);
        num_accepted_plans++;
        plan_mgr.save_plan(plan, sym_vars->get_state_registry()->get_task_proxy(), false, true);
    }

    void PlanDataBase::save_rejected_plan(const Plan &plan) {
        size_t plan_seed = get_hash_value(plan);
        if (hashes_rejected_plans.count(plan_seed) == 0) {
            hashes_rejected_plans[plan_seed] = std::vector<Plan>();
        }
        hashes_rejected_plans[plan_seed].push_back(plan);
        states_accepted_goal_paths += states_on_path(plan);
        num_rejected_plans++;
    }

    /*static std::shared_ptr<PlanDataBase> _parse(OptionParser &parser) {
        PlanDataBase::add_options_to_parser(parser);

        Options opts = parser.parse();
        if (parser.dry_run())
            return nullptr;
        return std::make_shared<PlanDataBase>(opts);
    }*/

    // static Plugin<PlanDataBase> _plugin("top_k_original", _parse);

    static PluginTypePlugin<PlanDataBase> _type_plugin(
            "PlanDataBase", "");

}

