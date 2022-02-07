#include "plan_database.h"

#include "../../option_parser.h"
#include "../../plugin.h"
#include "../../state_registry.h"

using namespace std;

namespace symbolic {
void PlanDataBase::add_options_to_parser(options::OptionParser &parser) {
    parser.add_option<int>("num_plans", "number of plans", "infinity",
                           Bounds("1", "infinity"));
}

PlanDataBase::PlanDataBase(const options::Options &opts)
    : sym_vars(nullptr), state_registry(nullptr), anytime_completness(false),
      num_desired_plans(opts.get<int>("num_plans")), num_accepted_plans(0),
      num_rejected_plans(0),
      first_accepted_plan_cost(numeric_limits<double>::infinity()) {}

void PlanDataBase::init(shared_ptr<SymVariables> sym_vars,
                        const shared_ptr<AbstractTask> &task,
                        PlanManager &plan_manager) {
    this->sym_vars = sym_vars;
    state_registry = make_shared<StateRegistry>(TaskProxy(*task));
    plan_mgr = plan_manager;
    states_accepted_goal_paths = sym_vars->zeroBDD();
}

bool PlanDataBase::has_accepted_plan(const Plan &plan) const {
    size_t plan_seed = get_hash_value(plan);
    if (hashes_accepted_plans.count(plan_seed) == 0) {
        return false;
    }
    if (different(hashes_accepted_plans.at(plan_seed), plan)) {
        return false;
    }
    return true;
}

bool PlanDataBase::has_rejected_plan(const Plan &plan) const {
    size_t plan_seed = get_hash_value(plan);
    if (hashes_rejected_plans.count(plan_seed) == 0) {
        return false;
    }
    if (different(hashes_rejected_plans.at(plan_seed), plan)) {
        return false;
    }
    return true;
}

void PlanDataBase::dump_first_accepted_plan() const {
    plan_mgr.dump_plan(first_accepted_plan,
                       state_registry->get_task_proxy());
}

const Plan &PlanDataBase::get_first_accepted_plan() const {
    return first_accepted_plan;
}

void PlanDataBase::print_options() const {
    utils::g_log << "Plan Selector: " << tag() << endl;
    utils::g_log << "Plan files: " << plan_mgr.get_plan_filename() << endl;
}

size_t PlanDataBase::different(const vector<Plan> &plans,
                               const Plan &plan) const {
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

BDD PlanDataBase::get_final_state(const Plan &plan) const {
    GlobalState cur = state_registry->get_initial_state();
    for (auto &op : plan) {
        cur = state_registry->get_successor_state(
            cur,
            state_registry->get_task_proxy().get_operators()[op]);
    }
    return sym_vars->getStateBDD(cur);
}

BDD PlanDataBase::states_on_path(const Plan &plan) {
    GlobalState cur = state_registry->get_initial_state();
    BDD path_states = sym_vars->getStateBDD(cur);
    for (auto &op : plan) {
        cur = state_registry->get_successor_state(
            cur,
            state_registry->get_task_proxy().get_operators()[op]);
        path_states += sym_vars->getStateBDD(cur);
    }
    return path_states;
}

// Hashes a vector of ints (= a plan)
// According to the following link this is the hash function used by boost
// for hashing vector<int>. Experience: really good function
// https://stackoverflow.com/questions/20511347/a-good-hash-function-for-a-vector

size_t PlanDataBase::get_hash_value(const Plan &plan) const {
    size_t seed = plan.size();
    for (auto &op : plan) {
        seed ^= op.get_index() + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
    return seed;
}

void PlanDataBase::save_accepted_plan(const Plan &plan) {
    if (num_accepted_plans == 0) {
        first_accepted_plan = plan;
        first_accepted_plan_cost = calculate_plan_cost(
            plan, state_registry->get_task_proxy());
    }

    size_t plan_seed = get_hash_value(plan);
    if (hashes_accepted_plans.count(plan_seed) == 0) {
        hashes_accepted_plans[plan_seed] = vector<Plan>();
    }
    hashes_accepted_plans[plan_seed].push_back(plan);
    states_accepted_goal_paths += states_on_path(plan);
    num_accepted_plans++;
    plan_mgr.save_plan(plan, state_registry->get_task_proxy(),
                       false, true);
}

void PlanDataBase::save_rejected_plan(const Plan &plan) {
    size_t plan_seed = get_hash_value(plan);
    if (hashes_rejected_plans.count(plan_seed) == 0) {
        hashes_rejected_plans[plan_seed] = vector<Plan>();
    }
    hashes_rejected_plans[plan_seed].push_back(plan);
    states_accepted_goal_paths += states_on_path(plan);
    num_rejected_plans++;
}

bool PlanDataBase::has_zero_cost_loop(const Plan &plan) const {
    GlobalState cur = state_registry->get_initial_state();
    BDD zero_reachable = sym_vars->getStateBDD(cur);
    for (auto &op : plan) {
        cur = state_registry->get_successor_state(
            cur,
            state_registry->get_task_proxy().get_operators()[op]);
        BDD new_state = sym_vars->getStateBDD(cur);

        if (state_registry
            ->get_task_proxy()
            .get_operators()[op]
            .get_cost() != 0) {
            zero_reachable = new_state;
        } else {
            BDD intersection = zero_reachable * new_state;
            if (!intersection.IsZero()) {
                return true;
            }
            zero_reachable += new_state;
        }
    }

    return false;
}

pair<int, int>
PlanDataBase::get_first_zero_cost_loop(const Plan &plan) const {
    pair<int, int> zero_cost_op_seq(-1, -1);
    int last_zero_op_state = 0;
    vector<GlobalState> states;
    states.push_back(state_registry->get_initial_state());
    for (size_t op_i = 0; op_i < plan.size(); ++op_i) {
        GlobalState succ = state_registry->get_successor_state(
            states.back(), state_registry
            ->get_task_proxy()
            .get_operators()[plan[op_i]]);

        for (size_t state_i = last_zero_op_state; state_i < states.size();
             ++state_i) {
            if (states[state_i].get_id() == succ.get_id()) {
                zero_cost_op_seq.first = state_i;
                zero_cost_op_seq.second = op_i;
                break;
            }
        }
        if (state_registry
            ->get_task_proxy()
            .get_operators()[plan[op_i]]
            .get_cost() != 0) {
            last_zero_op_state = states.size() - 1;
        }

        if (zero_cost_op_seq.first != -1) {
            break;
        }
        states.push_back(succ);
    }

    if (zero_cost_op_seq.first == -1) {
        cerr << "Zero loop goes wrong!" << endl;
        exit(0);
    }
    return zero_cost_op_seq;
}

vector<Plan> PlanDataBase::get_accepted_plans() const {
    vector<Plan> res;
    for (auto &it : hashes_accepted_plans) {
        res.insert(res.end(), it.second.begin(), it.second.end());
    }
    return res;
}

static PluginTypePlugin<PlanDataBase> _type_plugin("PlanDataBase", "");
}
