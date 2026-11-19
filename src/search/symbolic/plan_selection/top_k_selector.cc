#include "top_k_selector.h"

#include "../../plugins/plugin.h"

using namespace std;

namespace symbolic {
TopKSelector::TopKSelector(const plugins::Options &opts) : PlanSelector(opts) {
    PlanSelector::anytime_completness = true;
}

void TopKSelector::add_plan(const Plan &plan) {
    if (!has_accepted_plan(plan)) {
        save_accepted_plan(plan);
    }
}

class TopKSelectorFeature
    : public plugins::TypedFeature<PlanSelector> {
public:
    TopKSelectorFeature() : TypedFeature("top_k") {
        document_title("Top-k plan selector");

        PlanSelector::add_options_to_feature(*this);
    }

    virtual shared_ptr<PlanSelector> create_component(
        const plugins::Options &opts) const override {
        return make_shared<TopKSelector>(opts);
    }
};

static plugins::FeaturePlugin<TopKSelectorFeature> _plugin;
}
