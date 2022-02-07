#include "iterative_cost_selector.h"

#include "../../task_utils/task_properties.h"
#include "../../state_registry.h"

#include <iostream>
#include <stdio.h>

using namespace std;

namespace symbolic {
IterativeCostSelector::IterativeCostSelector(const options::Options &opts)
    : PlanDataBase(opts),
      most_expensive_plan_cost(-1) {
    PlanDataBase::anytime_completness = true;
}

bool IterativeCostSelector::reconstruct_solutions(
    const SymSolutionCut &cut) const {
    if (most_expensive_plan_cost >= cut.get_sol_cost()) {
        return false;
    }
    return !found_enough_plans();
}


void IterativeCostSelector::add_plan(const Plan &plan) {
    int cur_plan_cost = calculate_plan_cost(plan, state_registry->get_task_proxy());

    if (cur_plan_cost > most_expensive_plan_cost) {
        if (!has_accepted_plan(plan)) {
            save_accepted_plan(plan);
        }
        most_expensive_plan_cost = cur_plan_cost;
    }
}

static shared_ptr<PlanDataBase> _parse(OptionParser &parser) {
    PlanDataBase::add_options_to_parser(parser);

    Options opts = parser.parse();
    if (parser.dry_run())
        return nullptr;
    return make_shared<IterativeCostSelector>(opts);
}

static Plugin<PlanDataBase> _plugin("iterative_cost", _parse);
}
