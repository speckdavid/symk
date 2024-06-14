#include "iterative_cost_selector.h"

#include "../../task_utils/task_properties.h"
#include "../../state_registry.h"

#include <iostream>
#include <stdio.h>

using namespace std;

namespace symbolic {
IterativeCostSelector::IterativeCostSelector(const plugins::Options &opts)
    : PlanSelector(opts),
      most_expensive_plan_cost(opts.get<int>("plan_cost_bound")) {
    PlanSelector::anytime_completness = false;
}

void IterativeCostSelector::init(shared_ptr<SymVariables> sym_vars,
                                 const shared_ptr<AbstractTask> &task,
                                 PlanManager &plan_manager) {
    PlanSelector::init(sym_vars, task, plan_manager);
    utils::g_log << "Plan cost bound: " << most_expensive_plan_cost << endl;
    cout << endl;
}

bool IterativeCostSelector::reconstruct_solutions(int cost) const {
    if (most_expensive_plan_cost >= cost) {
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

class IterativeCostSelectorFeature : public plugins::TypedFeature<PlanSelector, IterativeCostSelector> {
public:
    IterativeCostSelectorFeature() : TypedFeature("iterative_cost") {
        document_title("Iterative cost plan selector");

        PlanSelector::add_options_to_feature(*this);
        this->add_option<int>(
            "plan_cost_bound",
            "plan cost bound such that only more expansive plans are reconstructed",
            "-1",
            plugins::Bounds("-1", "infinity"));
    }
};

static plugins::FeaturePlugin<IterativeCostSelectorFeature> _plugin;
}
