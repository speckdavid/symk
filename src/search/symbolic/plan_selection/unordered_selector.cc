#include "unordered_selector.h"

#include "../../option_parser.h"
#include "../../state_registry.h"

using namespace std;

namespace symbolic {
UnorderedSelector::UnorderedSelector(const options::Options &opts) : PlanDataBase(opts) {
    PlanDataBase::anytime_completness = true;
}

void UnorderedSelector::add_plan(const Plan &plan) {
    Plan unordered = plan;
    sort(unordered.begin(), unordered.end());

    if (!has_accepted_plan(unordered)) {
        save_accepted_plan(plan, unordered);
    }
}

void UnorderedSelector::save_accepted_plan(const Plan &ordered_plan, const Plan &unordered_plan) {
    if (num_accepted_plans == 0) {
        first_accepted_plan = ordered_plan;
        first_accepted_plan_cost = calculate_plan_cost(
            ordered_plan, state_registry->get_task_proxy());
    }

    size_t plan_seed = get_hash_value(unordered_plan);
    if (hashes_accepted_plans.count(plan_seed) == 0) {
        hashes_accepted_plans[plan_seed] = vector<Plan>();
    }
    hashes_accepted_plans[plan_seed].push_back(unordered_plan);
    states_accepted_goal_paths += states_on_path(ordered_plan);
    num_accepted_plans++;
    plan_mgr.save_plan(ordered_plan, state_registry->get_task_proxy(),
                       false, true);
}

static shared_ptr<PlanDataBase> _parse(OptionParser &parser) {
    PlanDataBase::add_options_to_parser(parser);

    Options opts = parser.parse();
    if (parser.dry_run())
        return nullptr;
    return make_shared<UnorderedSelector>(opts);
}

static Plugin<PlanDataBase> _plugin("unordered", _parse);
} // namespace symbolic
