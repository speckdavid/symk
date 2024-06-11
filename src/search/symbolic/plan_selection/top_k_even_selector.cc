#include "top_k_even_selector.h"

using namespace std;

namespace symbolic {
TopKEvenSelector::TopKEvenSelector(const plugins::Options &opts)
    : PlanSelector(opts) {
    anytime_completness = true;
}

void TopKEvenSelector::add_plan(const Plan &plan) {
    if (!has_rejected_plan(plan) && !has_accepted_plan(plan)) {
        if (plan.size() % 2 == 0) {
            save_accepted_plan(plan);
        } else {
            save_rejected_plan(plan);
        }
    }
}

class TopKEvenSelectorFeature : public plugins::TypedFeature<PlanSelector, TopKEvenSelector> {
public:
    TopKEvenSelectorFeature() : TypedFeature("top_k_even") {
        document_title("Top-K with even plan length plan selector");

        PlanSelector::add_options_to_feature(*this);
    }
};

static plugins::FeaturePlugin<TopKEvenSelectorFeature> _plugin;
}
